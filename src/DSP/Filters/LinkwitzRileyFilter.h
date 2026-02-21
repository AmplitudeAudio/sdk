// Copyright (c) 2025-present Sparky Studios. All rights reserved.
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

#ifndef _AM_IMPLEMENTATION_DSP_FILTERS_LINKWITZ_RILEY_FILTER_H
#define _AM_IMPLEMENTATION_DSP_FILTERS_LINKWITZ_RILEY_FILTER_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

#include <vector>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief A 4th-order Linkwitz-Riley crossover filter.
     *
     * This filter splits an audio signal into low-pass and high-pass bands.
     * The outputs sum to a flat magnitude response (0 dB at crossover frequency).
     *
     * A 4th-order Linkwitz-Riley filter is constructed by cascading two
     * 2nd-order Butterworth filters for each band. The Butterworth filters
     * use Q = 1/sqrt(2) ≈ 0.707 for maximally flat passband response.
     *
     * Coefficients are computed using the RBJ Audio EQ Cookbook formulas
     * (public domain by Robert Bristow-Johnson).
     *
     * @ingroup dsp
     */
    class LinkwitzRileyFilter
    {
    public:
        LinkwitzRileyFilter();
        ~LinkwitzRileyFilter() = default;

        /**
         * @brief Configures the filter with the specified parameters.
         *
         * @param channelCount The number of audio channels to process.
         * @param sampleRate The sample rate of the audio signal.
         * @param crossoverFrequency The crossover frequency in Hz.
         * @return True if configuration was successful, false otherwise.
         */
        bool Configure(AmUInt32 channelCount, AmUInt32 sampleRate, AmReal32 crossoverFrequency);

        /**
         * @brief Resets the filter state for all channels.
         */
        void Reset();

        /**
         * @brief Processes audio samples, splitting into low-pass and high-pass outputs.
         *
         * @param input The input audio buffer (channelCount x sampleCount).
         * @param lowPassOutput The low-pass filtered output (channelCount x sampleCount).
         * @param highPassOutput The high-pass filtered output (channelCount x sampleCount).
         * @param sampleCount The number of samples to process.
         */
        void Process(const AmReal32* const* input, AmReal32* const* lowPassOutput, AmReal32* const* highPassOutput, AmUInt32 sampleCount);

        /**
         * @brief Gets the number of channels configured for this filter.
         */
        [[nodiscard]] AmUInt32 GetChannelCount() const
        {
            return m_channelCount;
        }

    private:
        /**
         * @brief Computes Butterworth biquad coefficients for low-pass filter.
         *
         * Uses the RBJ Cookbook formula:
         *   omega = 2 * PI * freq / sampleRate
         *   alpha = sin(omega) / (2 * Q)
         *   b0 = (1 - cos(omega)) / (2 * (1 + alpha))
         *   b1 = (1 - cos(omega)) / (1 + alpha)
         *   b2 = (1 - cos(omega)) / (2 * (1 + alpha))
         *   a1 = -2 * cos(omega) / (1 + alpha)
         *   a2 = (1 - alpha) / (1 + alpha)
         */
        void ComputeLowPassCoefficients(AmReal32 frequency, AmReal32 sampleRate, AmReal32& b0, AmReal32& b1, AmReal32& b2, AmReal32& a1, AmReal32& a2);

        /**
         * @brief Computes Butterworth biquad coefficients for high-pass filter.
         *
         * Uses the RBJ Cookbook formula:
         *   omega = 2 * PI * freq / sampleRate
         *   alpha = sin(omega) / (2 * Q)
         *   b0 = (1 + cos(omega)) / (2 * (1 + alpha))
         *   b1 = -(1 + cos(omega)) / (1 + alpha)
         *   b2 = (1 + cos(omega)) / (2 * (1 + alpha))
         *   a1 = -2 * cos(omega) / (1 + alpha)
         *   a2 = (1 - alpha) / (1 + alpha)
         */
        void ComputeHighPassCoefficients(AmReal32 frequency, AmReal32 sampleRate, AmReal32& b0, AmReal32& b1, AmReal32& b2, AmReal32& a1, AmReal32& a2);

        // Filter coefficients for two cascaded low-pass biquads
        // Each biquad has: b0, b1, b2 (numerator), a1, a2 (denominator, a0 = 1)
        struct BiquadCoefficients
        {
            AmReal32 b0, b1, b2;
            AmReal32 a1, a2;
        };

        BiquadCoefficients m_lpCoeffs[2]; // Two low-pass biquads
        BiquadCoefficients m_hpCoeffs[2]; // Two high-pass biquads

        // Filter state for each channel and each biquad stage
        // State contains: x1 (previous input), x2 (input before that), y1 (previous output), y2 (output before that)
        struct BiquadState
        {
            AmReal32 x1, x2;
            AmReal32 y1, y2;
        };

        std::vector<BiquadState> m_lpState[2]; // State for two LP biquads, per channel
        std::vector<BiquadState> m_hpState[2]; // State for two HP biquads, per channel

        AmUInt32 m_channelCount;
        AmUInt32 m_sampleRate;
        AmReal32 m_crossoverFrequency;
        bool m_configured;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_DSP_FILTERS_LINKWITZ_RILEY_FILTER_H
