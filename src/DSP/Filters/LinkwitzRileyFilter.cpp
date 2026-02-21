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

#include <DSP/Filters/LinkwitzRileyFilter.h>
#include <Utils/Utils.h>

#include <cmath>

namespace SparkyStudios::Audio::Amplitude
{
    // Butterworth Q value: 1/sqrt(2) for maximally flat passband
    static constexpr AmReal32 kButterworthQ = 0.7071067811865476f;

    LinkwitzRileyFilter::LinkwitzRileyFilter()
        : m_channelCount(0)
        , m_sampleRate(0)
        , m_crossoverFrequency(0.0f)
        , m_configured(false)
    {
        // Initialize coefficients to passthrough (identity)
        for (int i = 0; i < 2; ++i)
        {
            m_lpCoeffs[i] = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f };
            m_hpCoeffs[i] = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f };
        }
    }

    bool LinkwitzRileyFilter::Configure(AmUInt32 channelCount, AmUInt32 sampleRate, AmReal32 crossoverFrequency)
    {
        // Validate parameters
        if (channelCount == 0 || sampleRate == 0 || crossoverFrequency <= 0.0f)
            return false;

        // Crossover frequency must be below Nyquist
        if (crossoverFrequency >= static_cast<AmReal32>(sampleRate) / 2.0f)
            return false;

        m_channelCount = channelCount;
        m_sampleRate = sampleRate;
        m_crossoverFrequency = crossoverFrequency;

        // Compute coefficients for both low-pass biquads
        for (int i = 0; i < 2; ++i)
        {
            ComputeLowPassCoefficients(
                crossoverFrequency,
                static_cast<AmReal32>(sampleRate),
                m_lpCoeffs[i].b0,
                m_lpCoeffs[i].b1,
                m_lpCoeffs[i].b2,
                m_lpCoeffs[i].a1,
                m_lpCoeffs[i].a2);

            ComputeHighPassCoefficients(
                crossoverFrequency,
                static_cast<AmReal32>(sampleRate),
                m_hpCoeffs[i].b0,
                m_hpCoeffs[i].b1,
                m_hpCoeffs[i].b2,
                m_hpCoeffs[i].a1,
                m_hpCoeffs[i].a2);
        }

        // Allocate state for each channel
        for (int i = 0; i < 2; ++i)
        {
            m_lpState[i].resize(channelCount);
            m_hpState[i].resize(channelCount);
        }

        Reset();
        m_configured = true;
        return true;
    }

    void LinkwitzRileyFilter::Reset()
    {
        for (int stage = 0; stage < 2; ++stage)
        {
            for (AmUInt32 ch = 0; ch < m_channelCount; ++ch)
            {
                m_lpState[stage][ch] = { 0.0f, 0.0f, 0.0f, 0.0f };
                m_hpState[stage][ch] = { 0.0f, 0.0f, 0.0f, 0.0f };
            }
        }
    }

    void LinkwitzRileyFilter::Process(
        const AmReal32* const* input,
        AmReal32* const* lowPassOutput,
        AmReal32* const* highPassOutput,
        AmUInt32 sampleCount)
    {
        if (!m_configured || sampleCount == 0)
            return;

        // Process each channel independently
        for (AmUInt32 ch = 0; ch < m_channelCount; ++ch)
        {
            const AmReal32* inChannel = input[ch];
            AmReal32* lpChannel = lowPassOutput[ch];
            AmReal32* hpChannel = highPassOutput[ch];

            // Temporary buffer for intermediate stage
            AmReal32 tempLP, tempHP;

            for (AmUInt32 i = 0; i < sampleCount; ++i)
            {
                AmReal32 sample = inChannel[i];

                // --- Low-pass path: cascade two Butterworth LP biquads ---
                tempLP = sample;

                // First LP biquad
                {
                    BiquadState& state = m_lpState[0][ch];
                    const BiquadCoefficients& c = m_lpCoeffs[0];

                    // Direct Form I: y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
                    AmReal32 y = c.b0 * tempLP + c.b1 * state.x1 + c.b2 * state.x2 - c.a1 * state.y1 - c.a2 * state.y2;

                    state.x2 = state.x1;
                    state.x1 = tempLP;
                    state.y2 = state.y1;
                    state.y1 = y;

                    // Denormal prevention
                    constexpr AmReal32 denormalThreshold = 1e-10f;
                    if (std::abs(y) < denormalThreshold)
                        y = 0.0f;

                    tempLP = y;
                }

                // Second LP biquad
                {
                    BiquadState& state = m_lpState[1][ch];
                    const BiquadCoefficients& c = m_lpCoeffs[1];

                    AmReal32 y = c.b0 * tempLP + c.b1 * state.x1 + c.b2 * state.x2 - c.a1 * state.y1 - c.a2 * state.y2;

                    state.x2 = state.x1;
                    state.x1 = tempLP;
                    state.y2 = state.y1;
                    state.y1 = y;

                    constexpr AmReal32 denormalThreshold = 1e-10f;
                    if (std::abs(y) < denormalThreshold)
                        y = 0.0f;

                    tempLP = y;
                }

                lpChannel[i] = tempLP;

                // --- High-pass path: cascade two Butterworth HP biquads ---
                tempHP = sample;

                // First HP biquad
                {
                    BiquadState& state = m_hpState[0][ch];
                    const BiquadCoefficients& c = m_hpCoeffs[0];

                    AmReal32 y = c.b0 * tempHP + c.b1 * state.x1 + c.b2 * state.x2 - c.a1 * state.y1 - c.a2 * state.y2;

                    state.x2 = state.x1;
                    state.x1 = tempHP;
                    state.y2 = state.y1;
                    state.y1 = y;

                    constexpr AmReal32 denormalThreshold = 1e-10f;
                    if (std::abs(y) < denormalThreshold)
                        y = 0.0f;

                    tempHP = y;
                }

                // Second HP biquad
                {
                    BiquadState& state = m_hpState[1][ch];
                    const BiquadCoefficients& c = m_hpCoeffs[1];

                    AmReal32 y = c.b0 * tempHP + c.b1 * state.x1 + c.b2 * state.x2 - c.a1 * state.y1 - c.a2 * state.y2;

                    state.x2 = state.x1;
                    state.x1 = tempHP;
                    state.y2 = state.y1;
                    state.y1 = y;

                    constexpr AmReal32 denormalThreshold = 1e-10f;
                    if (std::abs(y) < denormalThreshold)
                        y = 0.0f;

                    tempHP = y;
                }

                hpChannel[i] = tempHP;
            }
        }
    }

    void LinkwitzRileyFilter::ComputeLowPassCoefficients(
        AmReal32 frequency,
        AmReal32 sampleRate,
        AmReal32& b0,
        AmReal32& b1,
        AmReal32& b2,
        AmReal32& a1,
        AmReal32& a2)
    {
        // RBJ Audio EQ Cookbook formulas for 2nd-order Butterworth low-pass
        // Public domain formulas by Robert Bristow-Johnson
        //
        // H(s) = 1 / (s^2 + s/Q + 1)
        //
        // For Butterworth: Q = 1/sqrt(2)

        const AmReal32 omega = 2.0f * AM_PI32 * frequency / sampleRate;
        const AmReal32 cosOmega = std::cos(omega);
        const AmReal32 sinOmega = std::sin(omega);
        const AmReal32 alpha = sinOmega / (2.0f * kButterworthQ);

        // Normalization factor
        const AmReal32 a0 = 1.0f + alpha;

        // Numerator coefficients (normalized)
        b0 = (1.0f - cosOmega) / 2.0f / a0;
        b1 = (1.0f - cosOmega) / a0;
        b2 = (1.0f - cosOmega) / 2.0f / a0;

        // Denominator coefficients (a0 normalized to 1)
        a1 = -2.0f * cosOmega / a0;
        a2 = (1.0f - alpha) / a0;
    }

    void LinkwitzRileyFilter::ComputeHighPassCoefficients(
        AmReal32 frequency,
        AmReal32 sampleRate,
        AmReal32& b0,
        AmReal32& b1,
        AmReal32& b2,
        AmReal32& a1,
        AmReal32& a2)
    {
        // RBJ Audio EQ Cookbook formulas for 2nd-order Butterworth high-pass
        // Public domain formulas by Robert Bristow-Johnson
        //
        // H(s) = s^2 / (s^2 + s/Q + 1)
        //
        // For Butterworth: Q = 1/sqrt(2)

        const AmReal32 omega = 2.0f * AM_PI32 * frequency / sampleRate;
        const AmReal32 cosOmega = std::cos(omega);
        const AmReal32 sinOmega = std::sin(omega);
        const AmReal32 alpha = sinOmega / (2.0f * kButterworthQ);

        // Normalization factor
        const AmReal32 a0 = 1.0f + alpha;

        // Numerator coefficients (normalized)
        b0 = (1.0f + cosOmega) / 2.0f / a0;
        b1 = -(1.0f + cosOmega) / a0;
        b2 = (1.0f + cosOmega) / 2.0f / a0;

        // Denominator coefficients (a0 normalized to 1)
        a1 = -2.0f * cosOmega / a0;
        a2 = (1.0f - alpha) / a0;
    }
} // namespace SparkyStudios::Audio::Amplitude
