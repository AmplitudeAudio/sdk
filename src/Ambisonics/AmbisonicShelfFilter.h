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

#ifndef _AM_AMBISONIC_SHELF_FILTER_H
#define _AM_AMBISONIC_SHELF_FILTER_H

#include <Ambisonics/AmbisonicComponent.h>
#include <Ambisonics/BFormat.h>
#include <DSP/Filters/LinkwitzRileyFilter.h>

#include <vector>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Psychoacoustic optimization filter for ambisonic audio.
     *
     * This filter applies shelf filtering to B-format audio to psychoacoustically
     * optimize the high-frequency band. It addresses a fundamental problem in
     * ambisonic sound field reproduction: at higher frequencies, spatial sampling
     * limitations cause localization accuracy to degrade.
     *
     * The filter:
     * 1. Splits the signal into low and high frequency bands using a Linkwitz-Riley crossover
     * 2. Applies max-rE (maximum energy vector) gains to the high-frequency band
     * 3. Recombines the bands
     *
     * The crossover frequency is calculated based on ambisonic order using the formula
     * from Bertet et al. (2013):
     *   f_c = c * M / (4 * r * (M+1) * sin(π / (2M + 2)))
     * where c = speed of sound, r = head radius, M = ambisonic order
     *
     * The max-rE gains are computed using Legendre polynomials:
     *   g_n = P_n(cos(137.9° / (N + 1.51)))
     * where P_n is the Legendre polynomial of order n, N is the ambisonic order
     *
     * @ingroup ambisonics
     */
    class AmbisonicShelfFilter : public AmbisonicComponent
    {
    public:
        AmbisonicShelfFilter();
        ~AmbisonicShelfFilter() override;

        /**
         * @brief Configures the shelf filter for the specified ambisonic order.
         *
         * @param order The ambisonic order (1-3 supported).
         * @param is3D Whether the signal is 3D (periphonic) or 2D (horizontal only).
         * @param maxBlockSize The maximum number of samples to process in one call.
         * @param sampleRate The sample rate of the audio signal.
         * @param speedOfSound The speed of sound in m/s (default: 343.0). Affects crossover frequency.
         * @return True if configuration was successful, false otherwise.
         */
        bool Configure(AmUInt32 order, bool is3D, AmUInt32 maxBlockSize, AmUInt32 sampleRate, AmReal32 speedOfSound = 343.0f);

        /**
         * @brief Resets the filter state.
         */
        void Reset() override;

        /**
         * @brief Refreshes the filter (no-op for this implementation).
         */
        void Refresh() override;

        /**
         * @brief Gets the max-rE gains for the configured order and dimensionality.
         *
         * These gains are computed using the Legendre polynomial formula:
         *   g_n = P_n(cos(137.9° / (N + 1.51)))
         *
         * @return Vector of (order + 1) gains, one for each ambisonic order.
         */
        [[nodiscard]] std::vector<AmReal32> GetMaxReGains() const;

        /**
         * @brief Sets custom high-frequency gains to override the max-rE values.
         *
         * @param gains Vector of (order + 1) gains, one for each ambisonic order.
         */
        void SetHighFrequencyGains(const std::vector<AmReal32>& gains);

        /**
         * @brief Gets the crossover frequency for the current configuration.
         *
         * The crossover frequency is calculated using the formula from
         * Bertet et al. (2013):
         *   f_c = c * M / (4 * r * (M+1) * sin(π / (2M + 2)))
         *
         * @return The crossover frequency in Hz.
         */
        [[nodiscard]] AmReal32 GetCrossoverFrequency() const;

        /**
         * @brief Gets the speed of sound used for crossover calculation.
         *
         * @return The speed of sound in m/s.
         */
        [[nodiscard]] AmReal32 GetSpeedOfSound() const
        {
            return m_speedOfSound;
        }

        /**
         * @brief Processes a B-format audio buffer in-place.
         *
         * Applies psychoacoustic optimization by:
         * 1. Splitting into low/high frequency bands
         * 2. Applying max-rE gains to the high-frequency band
         * 3. Recombining the bands
         *
         * @param buffer The B-format buffer to process (modified in-place).
         * @param sampleCount The number of samples to process.
         */
        void Process(BFormat* buffer, AmUInt32 sampleCount);

    private:
        /**
         * @brief Computes Legendre polynomial P_n(x) using recurrence relation.
         *
         * The Legendre polynomials satisfy:
         *   P_0(x) = 1
         *   P_1(x) = x
         *   (n+1) * P_{n+1}(x) = (2n+1) * x * P_n(x) - n * P_{n-1}(x)
         *
         * @param n The order of the polynomial.
         * @param x The argument (typically cos(theta) for spherical harmonics).
         * @return The value of P_n(x).
         */
        static AmReal32 Legendre(AmUInt32 n, AmReal32 x);

        /**
         * @brief Gets the ambisonic order for a given ACN channel index.
         *
         * For ACN (Ambisonic Channel Numbering):
         *   order = floor(sqrt(channel))
         *
         * @param channel The ACN channel index.
         * @return The ambisonic order (degree) of that channel.
         */
        static AmUInt32 ChannelToOrder(AmUInt32 channel);

        /**
         * @brief Computes the crossover frequency for a given ambisonic order.
         *
         * Uses the formula from Bertet et al. (2013):
         *   f_c = c * M / (4 * r * (M+1) * sin(π / (2M + 2)))
         *
         * @param order The ambisonic order.
         * @param speedOfSound The speed of sound in m/s.
         * @return The crossover frequency in Hz.
         */
        static AmReal32 ComputeCrossoverFrequency(AmUInt32 order, AmReal32 speedOfSound);

        /**
         * @brief Computes max-rE gains for psychoacoustic optimization.
         *
         * For 3D (periphonic) ambisonics:
         *   g_n = P_n(cos(137.9° / (N + 1.51))) * energyComp
         *   where energyComp = (N+1) / sqrt(sum(P_n^2 * (2n+1)))
         *
         * For 2D (horizontal-only) ambisonics:
         *   g_n = cos(n * π / (2N + 2)) * sqrt((2N + 1) / (N + 1))
         *
         * References:
         *   - Zotter & Frank, "Ambisonics: A Practical 3D Audio Theory"
         *   - Daniel 2000, "Représentation de champs acoustiques"
         *   - Bertet et al. 2013, "Investigation on localisation accuracy"
         *
         * @param order The ambisonic order.
         * @param is3D Whether the gains are for 3D or 2D.
         * @return Vector of (order + 1) gains.
         */
        static std::vector<AmReal32> ComputeMaxReGains(AmUInt32 order, bool is3D);

        LinkwitzRileyFilter m_crossoverFilter;
        BFormat m_lowPassBuffer;
        AudioBuffer m_tempInputBuffer;
        std::vector<const AmReal32*> m_inputPtrs;
        std::vector<AmReal32*> m_lpPtrs;
        std::vector<AmReal32*> m_hpPtrs;
        std::vector<AmUInt32> m_channelOrders;
        std::vector<AmReal32> m_highFreqGains;
        AmUInt32 m_maxBlockSize;
        AmUInt32 m_sampleRate;
        AmReal32 m_speedOfSound;
        bool m_configured;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_AMBISONIC_SHELF_FILTER_H
