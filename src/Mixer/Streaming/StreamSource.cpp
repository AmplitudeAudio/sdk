// Copyright (c) 2026-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <algorithm>

#include <SparkyStudios/Audio/Amplitude/Core/Thread.h>

#include <Mixer/Streaming/StreamSource.h>
#include <Mixer/Voice/SourceReader.h>

namespace SparkyStudios::Audio::Amplitude
{
    StreamPlan PlanStream(
        AmUInt64 regionStart,
        AmUInt64 regionEnd,
        bool loop,
        AmUInt32 remainingWraps,
        AmUInt64 target,
        AmUInt64 prerollFrames,
        AmUInt64& prerollOut)
    {
        StreamPlan plan;
        plan.regionStart = regionStart;
        plan.regionEnd = regionEnd;
        plan.loop = loop;
        plan.remainingWraps = remainingWraps;
        plan.start = target;
        plan.target = target;
        plan.targetIndex = 0;

        const AmUInt64 played = target - regionStart;
        prerollOut = 0;
        if (played == 0 || regionEnd <= regionStart)
            return plan;

        const AmUInt64 length = regionEnd - regionStart;
        AmUInt64 frames = std::min(prerollFrames, played + (loop ? length : 0));
        // A preroll longer than one region is pointless: the same frames would repeat. Cap it to one pass before the target.
        frames = std::min(frames, loop ? length : played);
        const AmUInt64 tail = frames - std::min(frames, played);

        prerollOut = frames;
        plan.start = tail > 0 ? regionEnd - tail : target - frames;
        plan.targetIndex = frames;
        if (tail > 0 && plan.remainingWraps != SourceReader::kInfiniteWraps)
            ++plan.remainingWraps; // the tail is played before the wrap to the target
        return plan;
    }

    StreamSource::StreamSource(
        std::shared_ptr<Codec::Decoder> decoder,
        const StreamPlan& plan,
        AmUInt64 capacityFrames,
        AmUInt16 channels,
        AmUInt64 chunkFrames,
        AmUInt32 sampleRate,
        bool synchronous)
        : _decoder(std::move(decoder))
        , _plan(plan)
        , _ring(capacityFrames, channels)
        , _chunk(chunkFrames, channels)
        , _chunkFrames(chunkFrames)
        , _next(plan.start)
        , _wrapsLeft(plan.remainingWraps)
        , _sampleRate(sampleRate)
        , _synchronous(synchronous)
    {}

    StreamSource::~StreamSource()
    {
        if (_decoder != nullptr)
            _decoder->Close();
    }

    void StreamSource::AssertProducerThread()
    {
#if !defined(NDEBUG)
        const AmUInt64 self = static_cast<AmUInt64>(Thread::GetCurrentThreadId());
        AmUInt64 expected = 0;
        if (!_producerThread.compare_exchange_strong(expected, self))
            AMPLITUDE_ASSERT(expected == self);
#endif
    }

    AmUInt64 StreamSource::Fill(AmUInt64 maxFrames)
    {
        AssertProducerThread();

        AmUInt64 written = 0;
        while (written < maxFrames && !IsFilled())
        {
            if (_next >= _plan.regionEnd)
            {
                if (!_plan.loop || _wrapsLeft == 0 || _plan.regionEnd <= _plan.regionStart)
                {
                    _ring.MarkEnd();
                    break;
                }

                if (_wrapsLeft != SourceReader::kInfiniteWraps)
                    --_wrapsLeft;

                _next = _plan.regionStart;
            }

            const AmUInt64 room = _ring.GetWritableFrames();
            if (room == 0)
                break;

            const AmUInt64 wanted = std::min({ maxFrames - written, room, _chunkFrames, _plan.regionEnd - _next });
            const AmUInt64 got = _decoder != nullptr ? _decoder->Stream(&_chunk, 0, _next, wanted) : 0;
            if (got == 0)
            {
                _ring.MarkFailed();
                break;
            }

            written += _ring.Write(_chunk, 0, got);
            _next += got;

            if (got < wanted)
            {
                // A decoder that returns short before the region end has no more data: treat it as a failure.
                _ring.MarkFailed();
                break;
            }
        }

        return written;
    }

    bool StreamSource::IsFilled() const
    {
        return _ring.GetEndIndex() != StreamRing::kNoEnd || _ring.IsFailed();
    }

    AmUInt64 StreamSource::SourceFrameOf(AmUInt64 index) const
    {
        const AmUInt64 first = _plan.regionEnd - _plan.start;
        if (index < first || !_plan.loop)
            return _plan.start + index;

        const AmUInt64 length = _plan.regionEnd - _plan.regionStart;
        return _plan.regionStart + (index - first) % length;
    }

    bool StreamSource::IndexOf(AmUInt64 sourceFrame, AmUInt64& index) const
    {
        if (sourceFrame == _plan.target)
        {
            index = _plan.targetIndex;
            return true;
        }

        if (sourceFrame >= _plan.start && sourceFrame < _plan.regionEnd)
        {
            index = sourceFrame - _plan.start;
            return true;
        }

        if (_plan.loop && sourceFrame >= _plan.regionStart && sourceFrame < _plan.start)
        {
            index = (_plan.regionEnd - _plan.start) + (sourceFrame - _plan.regionStart);
            return true;
        }

        return false;
    }

    StreamRing& StreamSource::GetRing()
    {
        return _ring;
    }

    const StreamRing& StreamSource::GetRing() const
    {
        return _ring;
    }

    const StreamPlan& StreamSource::GetPlan() const
    {
        return _plan;
    }

    AmUInt64 StreamSource::GetBufferedFrames() const
    {
        return _ring.GetWriteIndex() - _ring.GetReleaseIndex();
    }

    AmUInt32 StreamSource::GetSampleRate() const
    {
        return _sampleRate;
    }

    bool StreamSource::IsSynchronous() const
    {
        return _synchronous;
    }

    void StreamSource::SetSuspended(bool suspended)
    {
        _suspended.store(suspended, std::memory_order_release);
    }

    bool StreamSource::IsSuspended() const
    {
        return _suspended.load(std::memory_order_acquire);
    }
} // namespace SparkyStudios::Audio::Amplitude
