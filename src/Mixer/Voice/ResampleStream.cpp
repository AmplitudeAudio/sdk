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

        _resampler->Initialize(1, sourceRate, outputRate);
        _baseRatio = static_cast<AmReal64>(sourceRate) / static_cast<AmReal64>(outputRate);
        _speed = 1.0;
        _sourceChannels = sourceChannels;

        // Size the FIFO for kFifoRatio blocks plus the widest read-ahead, measured at that ratio.
        _resampler->SetRatio(kFifoRatio);
        const AmUInt64 reach = _resampler->GetInputFramesNeeded(1);
        _resampler->SetRatio(_baseRatio);

        const auto blockInput = static_cast<AmUInt64>(std::ceil(kFifoRatio * static_cast<AmReal64>(maxBlockFrames)));
        const AmUInt64 capacity = blockInput + 2 * reach + kFifoMargin;
        _fifo = AudioBuffer(capacity, 1);
        _source = AudioBuffer(capacity, sourceChannels);
        _scratch = AudioBuffer(maxBlockFrames, 1);

        Reset();
        return true;
    }

    void ResampleStream::SetSpeed(AmReal64 speed)
    {
        if (!std::isfinite(speed) || speed <= 0.0)
            speed = 1.0;

        if (speed == _speed)
            return;

        _speed = speed;
        _resampler->SetRatio(GetRatio());
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

    void ResampleStream::UpdateTail()
    {
        // Every output whose kernel reaches the source end must be produced: past the end the stream feeds zeros until
        // the next output's centre is a read-ahead beyond it. A resampler with a delay also needs that delay flushed.
        _tail = 2 * _resampler->GetLatency() + _resampler->GetInputFramesNeeded(1);
    }

    ResampleStream::PullReport ResampleStream::Pull(SourceReader& reader, AudioBufferChannel& out, AmUInt64 offset, AmUInt64 frames)
    {
        PullReport report;
        AmUInt32 stalls = 0;
        const auto capacity = static_cast<AmUInt64>(_fifo.GetFrameCount());

        while (report.produced < frames)
        {
            const AmUInt64 want = AM_MIN(frames - report.produced, static_cast<AmUInt64>(_scratch.GetFrameCount()));
            const AmUInt64 needed = _resampler->GetInputFramesNeeded(want);

            if (_fifoCount < needed && _fifoCount < capacity)
                Refill(reader, AM_MIN(needed - _fifoCount, capacity - _fifoCount), report.produced, report);

            AmUInt64 inFrames = _fifoCount;
            AmUInt64 outFrames = want;
            const AmUInt64 consumedBefore = _inputConsumed;
            const AmUInt64 producedBefore = report.produced;
            if (!_resampler->Process(_fifo, inFrames, _scratch, outFrames))
                inFrames = outFrames = 0;

            std::copy_n(_scratch[0].begin(), outFrames, out.begin() + offset + report.produced);
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

            if (outFrames > 0)
            {
                stalls = 0;
                continue;
            }

            if (++stalls < 2)
                continue;

            std::fill(out.begin() + offset + report.produced, out.begin() + offset + frames, 0.0f);
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

    void ResampleStream::Refill(SourceReader& reader, AmUInt64 frames, AmUInt64 produced, PullReport& report)
    {
        if (frames == 0)
            return;

        SourceReadReport read;
        reader.Read(_source, 0, frames, read);

        auto& fifo = _fifo[0];
        if (_sourceChannels == 1)
        {
            std::copy_n(_source[0].begin(), frames, fifo.begin() + _fifoCount);
        }
        else if (_sourceChannels == 2)
        {
            const AmReal32 scale = InverseSquareRoot(2);
            for (AmUInt64 i = 0; i < frames; ++i)
                fifo[_fifoCount + i] = (_source[0][i] + _source[1][i]) * scale;
        }
        else
        {
            const AmReal32 scale = 1.0f / static_cast<AmReal32>(_sourceChannels);
            for (AmUInt64 i = 0; i < frames; ++i)
            {
                AmReal32 sum = 0.0f;
                for (AmUInt16 c = 0; c < _sourceChannels; ++c)
                    sum += _source[c][i];
                fifo[_fifoCount + i] = sum * scale;
            }
        }

        // Input frames map to output frames through the ratio; exact to within a frame, which events need.
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

        report.starved |= read.starved;
        _fifoCount += frames;
    }

    void ResampleStream::Consume(AmUInt64 frames)
    {
        AMPLITUDE_ASSERT(frames <= _fifoCount);
        if (frames == 0)
            return;

        auto& fifo = _fifo[0];
        std::copy(fifo.begin() + frames, fifo.begin() + _fifoCount, fifo.begin());
        _fifoCount -= frames;
    }

    AmUInt64 ResampleStream::GetSourcePosition(const SourceReader& reader) const
    {
        if (reader.IsEnded())
            return reader.GetCursor();

        return reader.Rewind(_fifoCount + _primedInput);
    }
} // namespace SparkyStudios::Audio::Amplitude
