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

#include <Mixer/SoundData.h>
#include <Mixer/Streaming/StreamSource.h>
#include <Mixer/Voice/SourceReader.h>

namespace SparkyStudios::Audio::Amplitude
{
    namespace
    {
        void ZeroFrames(AudioBuffer& out, AmUInt64 offset, AmUInt64 frames)
        {
            for (AmSize c = 0; c < out.GetChannelCount(); ++c)
                std::fill_n(out[c].begin() + offset, frames, 0.0f);
        }
    } // namespace

    VoiceSource MakeVoiceSource(SoundData* data)
    {
        VoiceSource source;
        if (data == nullptr || data->chunk == nullptr)
            return source;

        // A streamed sound has no whole-sound buffer: the mixer binds its ring through VoiceSource::stream.
        source.buffer = data->stream ? nullptr : data->chunk->buffer;
        source.length = data->length;
        source.channels = data->format.GetNumChannels();
        source.sampleRate = data->format.GetSampleRate();

        return source;
    }

    void SourceReader::Initialize(const VoiceSource& source, AmUInt64 regionStart, AmUInt64 regionEnd, bool loop, AmUInt32 loopCount)
    {
        _source = source;
        _end = regionEnd == 0 ? source.length : AM_MIN(regionEnd, source.length);
        _start = AM_MIN(regionStart, _end);
        _loop = loop;
        _remainingWraps = !loop ? 0 : loopCount == 0 ? kInfiniteWraps : loopCount - 1;
        _cursor = _start;
        _ended = _end <= _start;
        _ringIndex = 0;
        MapCursorToRing();
    }

    void SourceReader::BindStream(StreamSource* stream)
    {
        _source.stream = stream;
        _ringIndex = 0;
    }

    void SourceReader::MapCursorToRing()
    {
        if (!_source.IsStreamed())
            return;

        if (!_source.stream->IndexOf(_cursor, _ringIndex) || _ringIndex < _source.stream->GetRing().GetReleaseIndex())
        {
            // The frame is outside the plan, or already released: slip to the oldest frame the ring still holds.
            _ringIndex = _source.stream->GetRing().GetReleaseIndex();
            _cursor = _source.stream->SourceFrameOf(_ringIndex);
        }
    }

    AmUInt64 SourceReader::Read(AudioBuffer& out, AmUInt64 offset, AmUInt64 frames, SourceReadReport& report)
    {
        AMPLITUDE_ASSERT(out.GetChannelCount() == _source.channels);
        AMPLITUDE_ASSERT(offset + frames <= out.GetFrameCount());

        AmUInt64 written = 0;
        AmUInt64 delivered = 0;

        while (written < frames)
        {
            if (_ended)
            {
                ZeroFrames(out, offset + written, frames - written);
                break;
            }

            if (_cursor >= _end)
            {
                if (_remainingWraps > 0)
                {
                    if (_remainingWraps != kInfiniteWraps)
                        --_remainingWraps;

                    _cursor = _start;
                    if (report.wraps++ == 0)
                        report.firstWrapOffset = written;

                    continue;
                }

                _ended = true;
                report.ended = true;
                report.endOffset = written;
                continue;
            }

            const AmUInt64 wanted = AM_MIN(frames - written, _end - _cursor);
            const AmUInt64 got = CopyFrames(out, offset + written, wanted);

            _cursor += got;
            written += got;
            delivered += got;

            if (got < wanted)
            {
                const StreamRing& ring = _source.stream->GetRing();
                if (ring.IsFailed() || ring.GetEndIndex() <= _ringIndex)
                {
                    // A failed stream, or one that ended before the region end, ends the source here.
                    _ended = true;
                    report.ended = true;
                    report.endOffset = written;
                    continue;
                }

                // Underrun: the rest of this read is silence and the cursor holds, so nothing is skipped.
                report.underrun = true;
                ZeroFrames(out, offset + written, frames - written);
                break;
            }
        }

        return delivered;
    }

    void SourceReader::Seek(AmUInt64 frame)
    {
        _ended = _end <= _start;
        _cursor = _ended ? _start : AM_CLAMP(frame, _start, _end - 1);
        MapCursorToRing();
    }

    AmUInt64 SourceReader::Rewind(AmUInt64 frames) const
    {
        const AmUInt64 played = _cursor - _start;
        if (frames <= played)
            return _cursor - frames;

        if (!_loop || _end <= _start)
            return _start;

        const AmUInt64 length = _end - _start;
        const AmUInt64 back = (frames - played) % length;
        return back == 0 ? _start : _end - back;
    }

    AmUInt64 SourceReader::CopyFrames(AudioBuffer& out, AmUInt64 offset, AmUInt64 frames)
    {
        if (!_source.IsStreamed())
        {
            AudioBuffer::Copy(*_source.buffer, _cursor, out, offset, frames);
            return frames;
        }

        StreamSource* stream = _source.stream;
        StreamRing& ring = stream->GetRing();

        if (stream->IsSynchronous() && ring.GetWriteIndex() < _ringIndex + frames)
            stream->Fill(_ringIndex + frames - ring.GetWriteIndex());

        const AmUInt64 got = ring.Read(_ringIndex, out, offset, frames);
        _ringIndex += got;
        if (_releasesRing)
            ring.Release(_ringIndex);
        return got;
    }
} // namespace SparkyStudios::Audio::Amplitude
