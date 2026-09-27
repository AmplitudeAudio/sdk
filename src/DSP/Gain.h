// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

namespace SparkyStudios::Audio::Amplitude
{
    namespace Gain
    {
        void ApplyReplaceConstantGain(
            AmReal32 gain, const AudioBufferChannel& in, AmSize inOffset, AudioBufferChannel& out, AmSize outOffset, AmSize frames);
        void ApplyAccumulateConstantGain(
            AmReal32 gain, const AudioBufferChannel& in, AmSize inOffset, AudioBufferChannel& out, AmSize outOffset, AmSize frames);

        void ApplyReplaceLinearGain(
            AmReal32 startGain,
            AmReal32 endGain,
            const AudioBufferChannel& in,
            AmSize inOffset,
            AudioBufferChannel& out,
            AmSize outOffset,
            AmSize frames);
        void ApplyAccumulateLinearGain(
            AmReal32 startGain,
            AmReal32 endGain,
            const AudioBufferChannel& in,
            AmSize inOffset,
            AudioBufferChannel& out,
            AmSize outOffset,
            AmSize frames);

        void ApplyReplaceGain(
            Curve gainCurve, const AudioBufferChannel& in, AmSize inOffset, AudioBufferChannel& out, AmSize outOffset, AmSize frames);
        void ApplyAccumulateGain(
            Curve gainCurve, const AudioBufferChannel& in, AmSize inOffset, AudioBufferChannel& out, AmSize outOffset, AmSize frames);

        bool IsZero(AmReal32 gain);
        bool IsOne(AmReal32 gain);

        AmVector2 CalculateStereoPannedGain(AmReal32 gain, AmVector3 sourcePosition, AmMatrix4 listenerViewMatrix);
        AmVector2 CalculateStereoPannedGain(AmReal32 gain, SphericalPosition sourcePosition);
        AmVector2 CalculateStereoPannedGain(AmReal32 gain, AmReal32 pan);
    } // namespace Gain

    /**
     * @brief Minimum duration of a gain ramp, in milliseconds.
     */
    constexpr AmReal32 kGainRampMinDurationMs = 5.0f;

    /**
     * @brief Minimum ramp length used until @c GainProcessor::SetMinRampFrames() is called (~5.3 ms at 48 kHz).
     */
    constexpr AmSize kDefaultGainRampMinFrames = 256;

    /**
     * @brief Converts the minimum gain ramp duration to frames at the given sample rate.
     *
     * @param[in] sampleRate The sample rate in Hz.
     *
     * @return The minimum ramp length in frames, never less than 1.
     */
    AM_INLINE AmSize GainRampMinFrames(AmUInt32 sampleRate)
    {
        const auto frames = static_cast<AmSize>(static_cast<AmReal32>(sampleRate) * kGainRampMinDurationMs / 1000.0f);
        return AM_MAX(frames, AmSize(1));
    }

    /**
     * @brief Applies a gain to audio with click-free linear ramps that persist across blocks.
     *
     * A new target starts a linear ramp from the current gain lasting @c max(frames, minRampFrames) frames.
     * The ramp continues unchanged across subsequent calls with the same target. The first call on an
     * uninitialized processor snaps to the target gain without ramping.
     *
     * The class holds plain values only and is safe to copy.
     */
    class GainProcessor
    {
    public:
        GainProcessor();
        GainProcessor(AmReal32 initialGain);

        /**
         * @brief Applies the gain ramp towards @p gain to @p frames samples of @p in, writing into @p out.
         *
         * @param[in] gain The target gain. Non-finite values are treated as 0.
         * @param[in] in The input channel.
         * @param[in] inOffset The offset in the input channel.
         * @param[out] out The output channel. May be the same channel as @p in only when @p outOffset equals
         * @p inOffset; overlapping ranges at different offsets are not supported.
         * @param[in] outOffset The offset in the output channel.
         * @param[in] frames The number of frames to process. 0 is a no-op.
         * @param[in] accumulate Whether to add to @p out instead of replacing it.
         */
        void ApplyGain(
            AmReal32 gain,
            const AudioBufferChannel& in,
            AmSize inOffset,
            AudioBufferChannel& out,
            AmSize outOffset,
            AmSize frames,
            bool accumulate);

        /**
         * @brief Advances the ramp state exactly as @c ApplyGain() would, without processing audio.
         */
        void Advance(AmReal32 gain, AmSize frames);

        /**
         * @brief Sets the current and target gain, cancels any ramp, and marks the processor initialized.
         */
        void Reset(AmReal32 gain);

        /**
         * @brief Alias of @c Reset(), kept for existing callers.
         */
        void SetGain(AmReal32 gain);

        /**
         * @brief Marks the processor uninitialized: the next @c ApplyGain() snaps to its target.
         */
        void Invalidate();

        /**
         * @brief Sets the minimum ramp length in frames. Values below 1 are clamped to 1.
         */
        void SetMinRampFrames(AmSize frames);

        [[nodiscard]] AM_INLINE AmReal32 GetGain() const
        {
            return _currentGain;
        }

        [[nodiscard]] AM_INLINE bool IsRamping() const
        {
            return _remainingFrames > 0;
        }

        [[nodiscard]] AM_INLINE bool IsInitialized() const
        {
            return _isInitialized;
        }

        [[nodiscard]] AM_INLINE AmSize GetMinRampFrames() const
        {
            return _minRampFrames;
        }

    private:
        AmSize BeginBlock(AmReal32 gain, AmSize frames);
        void EndRamp(AmSize rampFrames);

        static void LinearGainRamp(AmReal32 startGain, AmReal32 step, const AmReal32* in, AmReal32* out, AmSize frames, bool accumulate);
        static void ConstantGain(AmReal32 gain, const AmReal32* in, AmReal32* out, AmSize frames, bool accumulate);

        AmReal32 _currentGain;
        AmReal32 _targetGain;
        AmReal32 _step;
        AmSize _remainingFrames;
        AmSize _minRampFrames;
        bool _isInitialized;
    };
} // namespace SparkyStudios::Audio::Amplitude