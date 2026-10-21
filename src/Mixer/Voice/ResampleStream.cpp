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
#include <cmath>

#include <SparkyStudios/Audio/Amplitude/Math/Utils.h>

#include <Mixer/Voice/ResampleStream.h>

namespace SparkyStudios::Audio::Amplitude
{
    bool ResampleStream::Initialize(
        const AmString& resamplerName, AmUInt32 sourceRate, AmUInt32 outputRate, AmUInt16 sourceChannels, AmUInt64 maxBlockFrames)
    {
        if (sourceRate == 0 || outputRate == 0 || sourceChannels == 0 || maxBlockFrames == 0)
            return false;

        _resampler = Resampler::Construct(resamplerName);
        if (_resampler == nullptr)
            return false;

        _resampler->Initialize(sourceChannels, sourceRate, outputRate);
        _baseRatio = static_cast<AmReal64>(sourceRate) / static_cast<AmReal64>(outputRate);
        _speed = 1.0;
        _speedRampStart = _speedRampEnd = 1.0;
        _speedRampFrames = 0;
        _sourceChannels = sourceChannels;

        // Size the FIFO for kFifoRatio blocks plus the widest read-ahead, measured at that ratio.
        _resampler->SetRatio(kFifoRatio);
        const AmUInt64 reach = _resampler->GetInputFramesNeeded(1);
        _resampler->SetRatio(_baseRatio);

        const auto blockInput = static_cast<AmUInt64>(std::ceil(kFifoRatio * static_cast<AmReal64>(maxBlockFrames)));
        const AmUInt64 capacity = blockInput + 2 * reach + kFifoMargin;
        _prerollFrames = reach;
        _fifo = AudioBuffer(capacity, sourceChannels);
        _scratch = AudioBuffer(maxBlockFrames, sourceChannels);

        Reset();
        return true;
    }

    void ResampleStream::SetSpeed(AmReal64 speed)
    {
        SetSpeedRamp(speed, speed, 0);
    }

    void ResampleStream::SetSpeedRamp(AmReal64 startSpeed, AmReal64 endSpeed, AmUInt64 outputFrames)
    {
        if (!std::isfinite(startSpeed) || startSpeed <= 0.0)
            startSpeed = 1.0;
        if (!std::isfinite(endSpeed) || endSpeed <= 0.0)
            endSpeed = 1.0;

        // Compare the whole ramp, not its mean: a (0.5, 1.5) ramp and a constant 1.0 share a mean, and dropping the
        // call would leave the stream ramping on a value the caller has retracted.
        if (startSpeed == _speedRampStart && endSpeed == _speedRampEnd && outputFrames == _speedRampFrames)
            return;

        _speedRampStart = startSpeed;
        _speedRampEnd = endSpeed;
        _speedRampFrames = outputFrames;

        // The mean is what the ramp advances the stream by, so it is what GetRatio() and the tail report.
        _speed = 0.5 * (startSpeed + endSpeed);
        _resampler->SetRatioRamp(_baseRatio * startSpeed, _baseRatio * endSpeed, outputFrames);
        UpdateTail();
    }

    void ResampleStream::Reset()
    {
        _fifoCount = 0;
        _inputConsumed = 0;
        _primedInput = 0;
        _endInput = 0;
        _endSeen = false;
        _finished = false;
        _resampler->Reset();
        UpdateTail();
    }

    void ResampleStream::Seek(SourceReader& reader, AmUInt64 position)
    {
        reader.Seek(position);
        Reset();

        // A restart at the region start plays from the top, after silence, even in a loop. Elsewhere a loop has its tail
        // before its start, and any other source nothing.
        const AmUInt64 start = reader.GetRegionStart();
        const AmUInt64 played = reader.GetCursor() - start;
        if (played == 0)
            return;

        const AmUInt64 length = reader.GetRegionEnd() - start;
        const AmUInt64 frames = AM_MIN(_prerollFrames, played + (reader.IsLooping() ? length : 0));
        const AmUInt64 tail = frames - AM_MIN(frames, played);

        // A copy reads the frames, each part exactly up to its end, so no seam, end or loop count is involved. The FIFO
        // is empty after Reset(): it holds the frames only until PrimeHistory() has taken them.
        SourceReader preroll = reader;
        preroll.SetReleasesRing(false);
        SourceReadReport read;
        if (tail > 0)
        {
            preroll.Seek(reader.GetRegionEnd() - tail);
            preroll.Read(_fifo, 0, tail, read);
        }

        preroll.Seek(reader.GetCursor() - (frames - tail));
        preroll.Read(_fifo, tail, frames - tail, read);
        _resampler->PrimeHistory(_fifo, frames);
    }

    void ResampleStream::UpdateTail()
    {
        // Every output whose kernel reaches the source end must be produced: past the end the stream feeds zeros until
        // the next output's centre is a read-ahead beyond it. A resampler with a delay also needs that delay flushed.
        // Under a ramp this is the read-ahead at the ramp's widest point, which is conservative: it holds the
        // stream until the end of the ramp even where the live ratio would already have reached it.
        _tail = 2 * _resampler->GetLatency() + _resampler->GetInputFramesNeeded(1);
    }

    ResampleStream::PullReport ResampleStream::Pull(SourceReader& reader, AudioBuffer& out, AmUInt64 offset, AmUInt64 frames)
    {
        PullReport report;
        AmUInt32 stalls = 0;
        const auto capacity = static_cast<AmUInt64>(_fifo.GetFrameCount());

        while (report.produced < frames)
        {
            const AmUInt64 want = AM_MIN(frames - report.produced, static_cast<AmUInt64>(_scratch.GetFrameCount()));
            const AmUInt64 needed = _resampler->GetInputFramesNeeded(want);

            bool underran = false;
            if (_fifoCount < needed && _fifoCount < capacity)
                underran = Refill(reader, AM_MIN(needed - _fifoCount, capacity - _fifoCount), report.produced, report);

            AmUInt64 inFrames = _fifoCount;
            AmUInt64 outFrames = want;
            const AmUInt64 consumedBefore = _inputConsumed;
            const AmUInt64 producedBefore = report.produced;
            if (!_resampler->Process(_fifo, inFrames, _scratch, outFrames))
                inFrames = outFrames = 0;

            for (AmUInt16 c = 0; c < _sourceChannels; ++c)
                std::copy_n(_scratch[c].begin(), outFrames, out[c].begin() + offset + report.produced);
            Consume(inFrames);
            _inputConsumed += inFrames;
            report.produced += outFrames;

            if (_endSeen && !_finished && _inputConsumed >= _endInput + _tail)
            {
                _finished = true;
                report.finished = true;

                // One Process() call covers a whole round, so map the input frame the flush completed at back onto the
                // output frame it lands on: reporting the end of the round would place it a whole block too late.
                const AmUInt64 threshold = _endInput + _tail;
                const AmUInt64 from = threshold - consumedBefore;
                const auto offsetFrames = static_cast<AmUInt64>(static_cast<AmReal64>(from) / GetRatio());
                report.finishedFrame = AM_MIN(producedBefore + offsetFrames, report.produced);
            }

            if (underran && _fifoCount < needed && outFrames < want)
            {
                // The stream could not keep up: the rest of the block is silence. The resampler is not advanced for it,
                // so the next pull continues from the held position and no source frame is skipped.
                for (AmUInt16 c = 0; c < _sourceChannels; ++c)
                    std::fill(out[c].begin() + offset + report.produced, out[c].begin() + offset + frames, 0.0f);
                report.produced = frames;
                break;
            }

            if (outFrames > 0)
            {
                stalls = 0;
                continue;
            }

            if (++stalls < 2)
                continue;

            for (AmUInt16 c = 0; c < _sourceChannels; ++c)
                std::fill(out[c].begin() + offset + report.produced, out[c].begin() + offset + frames, 0.0f);
            report.produced = frames;
            report.error = true;
            break;
        }

        const AmUInt64 last = frames > 0 ? frames - 1 : 0;
        report.firstWrapFrame = AM_MIN(report.firstWrapFrame, last);
        report.endFrame = AM_MIN(report.endFrame, last);
        report.finishedFrame = AM_MIN(report.finishedFrame, last);

        return report;
    }

    bool ResampleStream::Refill(SourceReader& reader, AmUInt64 frames, AmUInt64 produced, PullReport& report)
    {
        if (frames == 0)
            return false;

        SourceReadReport read;
        const AmUInt64 delivered = reader.Read(_fifo, _fifoCount, frames, read);

        // Input frames map to output frames through the mean ratio. At a constant ratio that is exact to within a frame;
        // during a ramp the frames between here and the event follow the ramp, not its mean, so an event lands up to
        // |end - start| * frames / 8 output frames off -- a few frames for an engine glide, well inside a block.
        const AmReal64 ratio = GetRatio();
        const auto toOutput = [&](AmUInt64 readOffset)
        {
            return produced + static_cast<AmUInt64>(static_cast<AmReal64>(_fifoCount + readOffset) / ratio);
        };

        if (read.wraps > 0)
        {
            if (report.wraps == 0)
                report.firstWrapFrame = toOutput(read.firstWrapOffset);

            report.wraps += read.wraps;
        }

        if (read.ended && !_endSeen)
        {
            _endSeen = true;
            _endInput = _inputConsumed + _fifoCount + read.endOffset;
            report.ended = true;
            report.endFrame = toOutput(read.endOffset);
        }

        // An underrun holds the source position: only the frames the stream delivered enter the FIFO. Otherwise a
        // non-looping end zero-fills past the end on purpose, and UpdateTail() relies on it.
        _fifoCount += read.underrun ? delivered : frames;
        report.underrun |= read.underrun;
        return read.underrun;
    }

    void ResampleStream::Consume(AmUInt64 frames)
    {
        AMPLITUDE_ASSERT(frames <= _fifoCount);
        if (frames == 0)
            return;

        for (AmUInt16 c = 0; c < _sourceChannels; ++c)
        {
            auto& fifo = _fifo[c];
            std::copy(fifo.begin() + frames, fifo.begin() + _fifoCount, fifo.begin());
        }

        _fifoCount -= frames;
    }

    AmUInt64 ResampleStream::GetSourcePosition(const SourceReader& reader) const
    {
        if (reader.IsEnded())
            return reader.GetCursor();

        return reader.Rewind(_fifoCount + _primedInput);
    }
} // namespace SparkyStudios::Audio::Amplitude
