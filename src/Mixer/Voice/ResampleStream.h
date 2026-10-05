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

#pragma once

#ifndef _AM_IMPLEMENTATION_MIXER_VOICE_RESAMPLE_STREAM_H
#define _AM_IMPLEMENTATION_MIXER_VOICE_RESAMPLE_STREAM_H

#include <memory>

#include <SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h>
#include <SparkyStudios/Audio/Amplitude/DSP/Resampler.h>

#include <Mixer/Voice/SourceReader.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Produces exactly the number of mono output frames asked for, pulling source frames through a FIFO.
     *
     * Unused input stays in the FIFO and the resampler keeps its phase, so the output does not depend on how it is
     * split into pulls. @c Initialize() allocates (game thread); everything else is audio-thread safe.
     */
    class ResampleStream
    {
    public:
        struct PullReport
        {
            AmUInt64 produced = 0;
            AmUInt32 wraps = 0;
            AmUInt64 firstWrapFrame = 0; ///< Output frame, relative to the pull start, of the first loop seam.
            bool ended = false;
            AmUInt64 endFrame = 0; ///< Output frame, relative to the pull start, of the source end.
            bool finished = false; ///< The filter tail after the end has been flushed.
            AmUInt64 finishedFrame = 0;
            bool starved = false;
            bool error = false; ///< The resampler made no progress twice in a row; the rest is silence.
        };

        /// Extra FIFO room on top of the largest read-ahead, so a refill never has to wait for a consume.
        static constexpr AmUInt64 kFifoMargin = 64;

        /// Ratio the FIFO is sized for; above it Pull() refills in steps.
        static constexpr AmReal64 kFifoRatio = 4.0;

        /**
         * @brief Allocates the FIFO and source-read buffers and constructs the resampler instance.
         *
         * @param[in] resamplerName The name of the registered resampler to construct, e.g. "default".
         * @param[in] sourceRate The source sample rate.
         * @param[in] outputRate The output sample rate.
         * @param[in] sourceChannels The number of channels the source provides; downmixed to mono.
         * @param[in] maxBlockFrames The largest number of output frames a single @c Pull() call will ever request.
         *
         * @return @c true on success, @c false if any parameter is invalid or the resampler is not registered.
         */
        bool Initialize(
            const AmString& resamplerName, AmUInt32 sourceRate, AmUInt32 outputRate, AmUInt16 sourceChannels, AmUInt64 maxBlockFrames);

        /**
         * @brief Sets pitch times playback speed; 1 plays at the native rate. Non-finite or non-positive values mean 1.
         *
         * @param[in] speed The playback speed multiplier.
         */
        void SetSpeed(AmReal64 speed);

        /**
         * @brief Ramps the speed across the next @p outputFrames output frames instead of stepping it at the boundary.
         *
         * @copydetails ResamplerInstance::SetRatioRamp
         *
         * @param[in] startSpeed The playback speed at the first of those output frames.
         * @param[in] endSpeed The playback speed at the last of those output frames.
         * @param[in] outputFrames The number of output frames the ramp spans.
         */
        void SetSpeedRamp(AmReal64 startSpeed, AmReal64 endSpeed, AmUInt64 outputFrames);

        /**
         * @brief Writes exactly @p frames frames into @p out from @p offset.
         *
         * @param[in] reader The source reader to pull frames from.
         * @param[out] out The output channel to write into.
         * @param[in] offset The offset, in @p out, of the first frame to write.
         * @param[in] frames The number of frames to write.
         *
         * @return A report describing loop seams, source end, filter flush completion and errors encountered
         * during the pull, with every frame index relative to the start of this pull.
         */
        PullReport Pull(SourceReader& reader, AudioBufferChannel& out, AmUInt64 offset, AmUInt64 frames);

        /**
         * @brief Drops the FIFO and the resampler history (after a reader seek).
         */
        void Reset();

        /**
         * @brief Returns the next source frame not yet consumed by the resampler, wrapped into the loop region.
         *
         * @param[in] reader The source reader this stream has been pulling from.
         *
         * @return The next unconsumed source frame.
         */
        [[nodiscard]] AmUInt64 GetSourcePosition(const SourceReader& reader) const;

        /**
         * @brief Checks whether the filter tail after the source end has been fully flushed.
         *
         * @return @c true once the stream has finished producing meaningful output.
         */
        [[nodiscard]] bool IsFinished() const
        {
            return _finished;
        }

        /**
         * @brief Gets the current input-per-output ratio, pitch and speed included.
         *
         * @return The current conversion ratio.
         */
        [[nodiscard]] AmReal64 GetRatio() const
        {
            return _baseRatio * _speed;
        }

        /**
         * @brief Gets the resampler's group delay, in source frames.
         *
         * @return The start latency of the resampler.
         */
        [[nodiscard]] AmUInt64 GetLatency() const
        {
            return _resampler->GetLatency();
        }

        /**
         * @brief Gets the number of source frames the resampler has consumed since the last reset.
         *
         * @return The consumed source frame count.
         */
        [[nodiscard]] AmUInt64 GetInputConsumed() const
        {
            return _inputConsumed;
        }

        /**
         * @brief Records source frames consumed by discarded priming output, so @c GetSourcePosition() keeps reporting the
         * source frame the next audible output maps to. Cleared by @c Reset().
         *
         * @param[in] frames The number of source frames the priming consumed.
         */
        void AddPrimedInput(AmUInt64 frames)
        {
            _primedInput += frames;
        }

    private:
        void Refill(SourceReader& reader, AmUInt64 frames, AmUInt64 produced, PullReport& report);
        void Consume(AmUInt64 frames);
        void UpdateTail();

        std::shared_ptr<ResamplerInstance> _resampler;
        AudioBuffer _fifo;
        AudioBuffer _source;
        AudioBuffer _scratch;
        AmUInt64 _fifoCount = 0;
        AmUInt64 _inputConsumed = 0;
        AmUInt64 _primedInput = 0;
        AmUInt64 _endInput = 0;
        AmUInt64 _tail = 1;
        AmReal64 _baseRatio = 1.0;
        AmReal64 _speed = 1.0;
        AmReal64 _speedRampStart = 1.0;
        AmReal64 _speedRampEnd = 1.0;
        AmUInt64 _speedRampFrames = 0;
        AmUInt16 _sourceChannels = 1;
        bool _endSeen = false;
        bool _finished = false;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_VOICE_RESAMPLE_STREAM_H
