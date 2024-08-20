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

#ifndef _AM_IMPLEMENTATION_DSP_RESAMPLERS_DEFAULT_RESAMPLER_H
#define _AM_IMPLEMENTATION_DSP_RESAMPLERS_DEFAULT_RESAMPLER_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

namespace SparkyStudios::Audio::Amplitude
{
    class DefaultResamplerInstance final : public ResamplerInstance
    {
        friend class AudioConverter;

    public:
        /**
         * @brief Checks if the given conversion is supported.
         *
         * @param source The source frequency in Hz.
         * @param destination The destination frequency in Hz.
         *
         * @return @c true if the conversion is supported, @c false otherwise.
         */
        static bool IsConversionSupported(AmUInt64 source, AmUInt64 destination);

        /**
         * @brief Constructs a new @c Resampler.
         */
        DefaultResamplerInstance();

        /**
         * @brief Computes the maximum length of the output buffer from the given
         * input length, knowing the source and destination frequencies. The actual
         * output length will be either the returned value or one less.
         *
         * @param inputLength The length of the input buffer.
         *
         * @return The maximum length of the output buffer.
         */
        [[nodiscard]] AmUInt64 GetMaxOutputLength(AmUInt64 inputLength) const;

        /**
         * @copydoc ResamplerInstance::Process
         */
        bool Process(const AudioBuffer& input, AmUInt64& inputFrames, AudioBuffer& output, AmUInt64& outputFrames) override;

        /**
         * @copydoc ResamplerInstance::GetExpectedOutputFrames
         */
        [[nodiscard]] AmUInt64 GetExpectedOutputFrames(AmUInt64 inputLength) const override;

        /**
         * @copydoc ResamplerInstance::GetRequiredInputFrames
         */
        [[nodiscard]] AmUInt64 GetRequiredInputFrames(AmUInt64 outputLength) const override;

        /**
         * @copydoc ResamplerInstance::Initialize
         */
        void Initialize(AmUInt16 channelCount, AmUInt32 sampleRateIn, AmUInt32 sampleRateOut) override;

        /**
         * @copydoc ResamplerInstance::Reset
         */
        void Reset() override;

        /**
         * @copydoc ResamplerInstance::SetSampleRate
         */
        void SetSampleRate(AmUInt32 sampleRateIn, AmUInt32 sampleRateOut) override;

        /**
         * @copydoc ResamplerInstance::GetSampleRateIn
         */
        [[nodiscard]] AM_INLINE AmUInt32 GetSampleRateIn() const override
        {
            return _sampleRateIn;
        }

        /**
         * @copydoc ResamplerInstance::GetSampleRateOut
         */
        [[nodiscard]] AM_INLINE AmUInt32 GetSampleRateOut() const override
        {
            return _sampleRateOut;
        }

        /**
         * @copydoc ResamplerInstance::GetChannelCount
         */
        [[nodiscard]] AM_INLINE AmUInt16 GetChannelCount() const override
        {
            return _channelCount;
        }

        /**
         * @copydoc ResamplerInstance::GetInputLatency
         */
        [[nodiscard]] AM_INLINE AmUInt64 GetInputLatency() const override
        {
            return 0;
        }

        /**
         * @copydoc ResamplerInstance::GetOutputLatency
         */
        [[nodiscard]] AM_INLINE AmUInt64 GetOutputLatency() const override
        {
            return 0;
        }

        /**
         * @copydoc ResamplerInstance::Clear
         */
        void Clear() override;

    private:
        /**
         * @brief Initializes the @c _state buffer. Called when the resampler is configured
         * or when the resampler is reset.
         *
         * @param oldFrameCount Number of frames in the @c _state buffer prior to the
         * most recent call to @c GenerateInterpolatingFilter.
         */
        void InitializeStateBuffer(AmUInt64 oldFrameCount);

        /**
         * @brief Generates a windowed sinc to act as the interpolating/anti-aliasing filter.
         *
         * @param sampleRate The target sample rate.
         */
        void GenerateInterpolatingFilter(AmUInt64 sampleRate);

        /**
         * @brief Arranges the antialiasing filter coefficients in polyphase filter format.
         *
         * @param filterLength The number of samples in the filter buffer.
         * @param filter The buffer containing the filter coefficients.
         */
        void ArrangeFilterAsPolyphase(AmUInt64 filterLength, const AudioBufferChannel& filter);

        /**
         * @brief Generates Hann windowed sinc function anti aliasing filters.
         *
         * @param cutoffFrequency Transition band (-3dB) frequency of the filter.
         * @param sampleRate The target sample rate.
         * @param filterLength The number of samples in the filter buffer.
         * @param filter The buffer containing the filter coefficients.
         */
        void GenerateSincFilter(AmReal32 cutoffFrequency, AmUInt64 sampleRate, AmUInt64 filterLength, AudioBufferChannel* filter);

        /**
         * @brief Checks if the resampler is configured with the same source and destination frequencies.
         *
         * @return @c true if the resampler is configured with the same source and destination frequencies, @c false otherwise.
         */
        [[nodiscard]] AM_INLINE bool IsIdentity() const
        {
            return _upRate == _downRate;
        }

        // Rate of the interpolator section of the rational sampling rate converter.
        AmUInt64 _upRate;

        // Rate of the decimator section of the rational sampling rate converter.
        AmUInt64 _downRate;

        // Time variable for the polyphase filter.
        AmUInt64 _timeModuloUpRate;

        // Marks the last processed sample of the input.
        AmUInt64 _lastProcessedSample;

        // Number of channels in the AudioBuffer processed.
        AmUInt64 _channelCount;

        // Number of filter coefficients in each phase of the polyphase filter.
        AmUInt64 _coefficientsPerPhase;

        // Filter coefficients stored in polyphase form.
        AudioBuffer _transposedFilterCoefficients;

        // Filter coefficients in planar form, used for calculating the transposed filter.
        AudioBuffer _temporaryFilterCoefficients;

        // Buffer holding the samples of input required between input buffers.
        AudioBuffer _state;

        // Source and destination sample rates.
        AmUInt32 _sampleRateIn = 0;
        AmUInt32 _sampleRateOut = 0;
    };

    class DefaultResampler final : public Resampler
    {
    public:
        DefaultResampler()
            : Resampler("default")
        {}

        ResamplerInstance* CreateInstance() override
        {
            return ampoolnew(MemoryPoolKind::Filtering, DefaultResamplerInstance);
        }

        void DestroyInstance(ResamplerInstance* instance) override
        {
            ampooldelete(MemoryPoolKind::Filtering, DefaultResamplerInstance, (DefaultResamplerInstance*)instance);
        }
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_DSP_RESAMPLERS_DEFAULT_RESAMPLER_H
