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

#ifndef _AM_IMPLEMENTATION_DSP_RESAMPLERS_BANDLIMITED_RESAMPLER_H
#define _AM_IMPLEMENTATION_DSP_RESAMPLERS_BANDLIMITED_RESAMPLER_H

#include <memory>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <DSP/Resamplers/BandlimitedKernel.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Quality tiers of the built-in resampler, from cheapest to best.
     */
    enum class eResamplerPreset : AmUInt8
    {
        Linear, ///< 2 taps. Cheapest; dull and aliased.
        Cubic, ///< 4-point Hermite.
        Sinc, ///< 32-tap windowed sinc, about -80 dB.
        SincBest, ///< 80-tap windowed sinc, at least -110 dB, flat to 20 kHz. The default.
    };

    /// Above this ratio the sinc kernels stop stretching: the pitch stays exact, the top of the output band aliases.
    constexpr AmReal64 kMaxKernelStretch = 4.0;

    /**
     * @brief Gets the shared kernel table of a preset, building it on first use (game thread).
     *
     * @return The kernel, or @c nullptr for @c Linear and @c Cubic.
     */
    [[nodiscard]] std::shared_ptr<const BandlimitedKernel> GetPresetKernel(eResamplerPreset preset);

    /**
     * @brief Band-limited interpolation at any real ratio, with zero delay.
     *
     * A 32.32 fixed-point accumulator holds the position of the next output's centre, relative to the first frame of
     * the next input. An output reads every input frame j with |j - t| < W, where W is 1 (linear), 2 (cubic) or
     * zeroCrossings times the stretch (sinc presets; the stretch is the ratio clamped to [1, kMaxKernelStretch]). Frames
     * before the next output's centre are consumed; the last ones stay in the history. At a ratio of exactly 1 on a
     * whole-frame phase, an output reads only its centre frame: the input passes through unchanged.
     *
     * @c Initialize() and @c Clear() run on the game thread. Everything else is audio-thread safe and never allocates.
     */
    class BandlimitedResamplerInstance final : public ResamplerInstance
    {
    public:
        /**
         * @brief Constructs a new @c BandlimitedResamplerInstance.
         *
         * @param[in] preset The quality tier to run.
         * @param[in] kernel The kernel of @p preset, or @c nullptr for the polynomial presets.
         */
        BandlimitedResamplerInstance(eResamplerPreset preset, std::shared_ptr<const BandlimitedKernel> kernel);

        /**
         * @copydoc ResamplerInstance::Initialize
         */
        void Initialize(AmUInt16 channelCount, AmUInt32 sampleRateIn, AmUInt32 sampleRateOut) override;

        /**
         * @copydoc ResamplerInstance::Process
         */
        bool Process(const AudioBuffer& input, AmUInt64& inputFrames, AudioBuffer& output, AmUInt64& outputFrames) override;

        /**
         * @copydoc ResamplerInstance::SetRatio
         */
        void SetRatio(AmReal64 inputPerOutput) override;

        /**
         * @copydoc ResamplerInstance::SetRatioRamp
         */
        void SetRatioRamp(AmReal64 inputPerOutputStart, AmReal64 inputPerOutputEnd, AmUInt64 outputFrames) override;

        /**
         * @copydoc ResamplerInstance::IsConversionExact
         */
        [[nodiscard]] bool IsConversionExact(AmUInt32 sampleRateIn, AmUInt32 sampleRateOut) const override
        {
            return true;
        }

        /**
         * @copydoc ResamplerInstance::GetSampleRateIn
         */
        [[nodiscard]] AmUInt32 GetSampleRateIn() const override
        {
            return _rateIn;
        }

        /**
         * @copydoc ResamplerInstance::GetSampleRateOut
         */
        [[nodiscard]] AmUInt32 GetSampleRateOut() const override
        {
            return _rateOut;
        }

        /**
         * @copydoc ResamplerInstance::GetChannelCount
         */
        [[nodiscard]] AmUInt16 GetChannelCount() const override
        {
            return _channels;
        }

        /**
         * @copydoc ResamplerInstance::GetInputFramesNeeded
         */
        [[nodiscard]] AmUInt64 GetInputFramesNeeded(AmUInt64 outputFrameCount) const override;

        /**
         * @copydoc ResamplerInstance::GetLatency
         */
        [[nodiscard]] AmUInt64 GetLatency() const override
        {
            return 0;
        }

        /**
         * @copydoc ResamplerInstance::Reset
         */
        void Reset() override;

        /**
         * @copydoc ResamplerInstance::PrimeHistory
         */
        void PrimeHistory(const AudioBuffer& input, AmUInt64 frames) override;

        /**
         * @copydoc ResamplerInstance::Clear
         */
        void Clear() override;

        /**
         * @brief Gets the current input-per-output ratio.
         *
         * A ramp in progress reports its mean, the ratio the stream advances by over the ramp: this is the value a
         * caller sizes buffers and maps positions from. Once the ramp is spent the instance holds its end value, and
         * reports that.
         */
        [[nodiscard]] AmReal64 GetRatio() const
        {
            return _ratio;
        }

    private:
        static constexpr AmUInt64 kOne = 1ULL << 32;

        /// The widest read-ahead any ratio can ask for, in whole frames: the taps plus the history they reach into.
        [[nodiscard]] AmUInt64 MaxReachFrames() const;

        /// The read-ahead in use, in 32.32 frames: zero at identity, where an output needs its centre frame only.
        [[nodiscard]] AmUInt64 CurrentReach() const;

        /// The accumulator step, in 32.32 frames, of the output frame at @p position within the current ramp.
        [[nodiscard]] AmUInt64 StepAt(AmUInt64 position) const;

        /// Sizes the kernel for @p ratio and returns the read-ahead it needs, in 32.32 frames.
        AmUInt64 ApplyRatio(AmReal64 ratio);

        /// Convolves one output frame with the weights of its time, for every channel.
        void RenderFrame(const AudioBuffer& input, AudioBuffer& output, AmUInt64 time, AmUInt64 reach, AmUInt64 index);

        /// Moves the consumed trailing frames of the input into the history read-ahead.
        void PushHistory(const AudioBuffer& input, AmUInt64 consumed);

        eResamplerPreset _preset;
        std::shared_ptr<const BandlimitedKernel> _kernel;

        AudioBuffer _history;
        AmUInt64 _historyFrames = 0;
        std::vector<AmReal32> _weights;
        std::vector<AmReal32> _gather;

        /// The widest accumulator step of the current ratio or ramp, in 32.32 frames: the sizing bound.
        AmUInt64 _maxStep = kOne;
        AmUInt64 _rampStepStart = kOne;
        AmUInt64 _rampStepEnd = kOne;
        AmUInt64 _rampFrames = 0;
        AmUInt64 _rampPos = 0;
        AmUInt64 _frac = 0;
        AmUInt64 _reach = kOne;
        AmReal64 _ratio = 1.0;
        AmReal64 _stretch = 1.0;
        AmReal64 _tableScale = 0.0;
        AmReal32 _gain = 1.0f;

        AmUInt32 _rateIn = 0;
        AmUInt32 _rateOut = 0;
        AmUInt16 _channels = 0;
    };

    /**
     * @brief A built-in resampler preset, registered under its own name.
     */
    class BandlimitedResampler final : public Resampler
    {
    public:
        /**
         * @brief Constructs a new @c BandlimitedResampler.
         *
         * @param[in] name The name the preset is registered under.
         * @param[in] preset The quality tier to run.
         */
        BandlimitedResampler(AmString name, eResamplerPreset preset);

        /**
         * @copydoc Resampler::CreateInstance
         */
        std::shared_ptr<ResamplerInstance> CreateInstance() override;

        /**
         * @brief Gets the quality tier of this resampler.
         */
        [[nodiscard]] eResamplerPreset GetPreset() const
        {
            return _preset;
        }

    private:
        eResamplerPreset _preset;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_DSP_RESAMPLERS_BANDLIMITED_RESAMPLER_H
