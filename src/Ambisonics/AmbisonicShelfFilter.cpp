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

#include <Ambisonics/AmbisonicShelfFilter.h>
#include <Utils/Utils.h>

#include <algorithm>
#include <cmath>

namespace SparkyStudios::Audio::Amplitude
{
    // Average human head radius in meters (from Bertet et al. 2013)
    static constexpr AmReal32 kHeadRadius = 0.09f;

    // Max-rE gain calculation angle (from Zotter & Frank)
    static constexpr AmReal32 kMaxReAngleDegrees = 137.9f;

    AmbisonicShelfFilter::AmbisonicShelfFilter()
        : AmbisonicComponent()
        , m_maxBlockSize(0)
        , m_sampleRate(0)
        , m_speedOfSound(343.0f)
        , m_configured(false)
    {}

    AmbisonicShelfFilter::~AmbisonicShelfFilter() = default;

    bool AmbisonicShelfFilter::Configure(AmUInt32 order, bool is3D, AmUInt32 maxBlockSize, AmUInt32 sampleRate, AmReal32 speedOfSound)
    {
        if (order < 1 || order > 3)
            return false;

        if (maxBlockSize == 0 || sampleRate == 0)
            return false;

        if (!AmbisonicComponent::Configure(order, is3D))
            return false;

        m_maxBlockSize = maxBlockSize;
        m_sampleRate = sampleRate;
        m_speedOfSound = speedOfSound;

        const AmReal32 crossoverFreq = ComputeCrossoverFrequency(order, speedOfSound);

        if (!m_crossoverFilter.Configure(m_channelCount, sampleRate, crossoverFreq))
            return false;

        if (!m_lowPassBuffer.Configure(order, is3D, maxBlockSize))
            return false;

        m_tempInputBuffer = AudioBuffer(maxBlockSize, m_channelCount);
        m_inputPtrs.resize(m_channelCount);
        m_lpPtrs.resize(m_channelCount);
        m_hpPtrs.resize(m_channelCount);
        m_channelOrders.resize(m_channelCount);
        for (AmUInt32 ch = 0; ch < m_channelCount; ++ch)
            m_channelOrders[ch] = ChannelToOrder(ch);

        m_highFreqGains = ComputeMaxReGains(order, is3D);

        m_configured = true;
        return true;
    }

    void AmbisonicShelfFilter::Reset()
    {
        m_crossoverFilter.Reset();

        if (m_lowPassBuffer.GetBuffer() != nullptr)
            m_lowPassBuffer.GetBuffer()->Clear();
    }

    void AmbisonicShelfFilter::Refresh()
    {}

    std::vector<AmReal32> AmbisonicShelfFilter::GetMaxReGains() const
    {
        return m_highFreqGains;
    }

    void AmbisonicShelfFilter::SetHighFrequencyGains(const std::vector<AmReal32>& gains)
    {
        if (gains.size() == m_order + 1)
            m_highFreqGains = gains;
    }

    AmReal32 AmbisonicShelfFilter::GetCrossoverFrequency() const
    {
        return ComputeCrossoverFrequency(m_order, m_speedOfSound);
    }

    void AmbisonicShelfFilter::Process(BFormat* buffer, AmUInt32 sampleCount)
    {
        if (!m_configured || buffer == nullptr || sampleCount == 0)
            return;

        if (sampleCount > m_maxBlockSize)
            return;

        for (AmUInt32 ch = 0; ch < m_channelCount; ++ch)
        {
            m_lpPtrs[ch] = m_lowPassBuffer.GetBufferChannel(ch).begin();
            m_hpPtrs[ch] = buffer->GetBufferChannel(ch).begin();
        }

        for (AmUInt32 ch = 0; ch < m_channelCount; ++ch)
        {
            const AmReal32* src = buffer->GetBufferChannel(ch).begin();
            std::copy(src, src + sampleCount, m_tempInputBuffer[ch].begin());
            m_inputPtrs[ch] = m_tempInputBuffer[ch].begin();
        }

        m_crossoverFilter.Process(m_inputPtrs.data(), m_lpPtrs.data(), m_hpPtrs.data(), sampleCount);

        for (AmUInt32 ch = 0; ch < m_channelCount; ++ch)
        {
            const AmReal32 gain = m_highFreqGains[m_channelOrders[ch]];

            AmReal32* hpData = m_hpPtrs[ch];
            const AmReal32* lpData = m_lpPtrs[ch];

            for (AmUInt32 i = 0; i < sampleCount; ++i)
                hpData[i] = gain * hpData[i] + lpData[i];
        }
    }

    AmReal32 AmbisonicShelfFilter::Legendre(AmUInt32 n, AmReal32 x)
    {
        if (n == 0)
            return 1.0f;

        if (n == 1)
            return x;

        AmReal32 pPrev2 = 1.0f;
        AmReal32 pPrev1 = x;
        AmReal32 pCurrent = 0.0f;

        for (AmUInt32 i = 1; i < n; ++i)
        {
            pCurrent =
                ((2.0f * static_cast<AmReal32>(i) + 1.0f) * x * pPrev1 - static_cast<AmReal32>(i) * pPrev2) / static_cast<AmReal32>(i + 1);

            pPrev2 = pPrev1;
            pPrev1 = pCurrent;
        }

        return pCurrent;
    }

    AmUInt32 AmbisonicShelfFilter::ChannelToOrder(AmUInt32 channel)
    {
        return static_cast<AmUInt32>(std::floor(std::sqrt(static_cast<AmReal32>(channel))));
    }

    AmReal32 AmbisonicShelfFilter::ComputeCrossoverFrequency(AmUInt32 order, AmReal32 speedOfSound)
    {
        if (order == 0 || speedOfSound <= 0.0f)
            return 0.0f;

        const AmReal32 M = static_cast<AmReal32>(order);
        const AmReal32 numerator = speedOfSound * M;
        const AmReal32 denominator = 4.0f * kHeadRadius * (M + 1.0f) * std::sin(AM_PI32 / (2.0f * M + 2.0f));

        return numerator / denominator;
    }

    std::vector<AmReal32> AmbisonicShelfFilter::ComputeMaxReGains(AmUInt32 order, bool is3D)
    {
        std::vector<AmReal32> gains(order + 1);

        const AmReal32 N = static_cast<AmReal32>(order);

        if (is3D)
        {
            // 3D max-rE gains using Legendre polynomials
            // Formula: a_n = P_n(cos(137.9° / (N + 1.51))) * energyComp
            // Reference: Zotter & Frank, "Ambisonics: A Practical 3D Audio Theory"
            const AmReal32 thetaDegrees = kMaxReAngleDegrees / (N + 1.51f);
            const AmReal32 thetaRadians = thetaDegrees * AM_PI32 / 180.0f;
            const AmReal32 cosTheta = std::cos(thetaRadians);

            // Compute raw Legendre polynomial values
            for (AmUInt32 n = 0; n <= order; ++n)
            {
                gains[n] = Legendre(n, cosTheta);
            }

            // Apply energy compensation: (N+1) / sqrt(sum(P_n^2 * (2n+1)))
            AmReal32 sumSquared = 0.0f;
            for (AmUInt32 n = 0; n <= order; ++n)
            {
                const AmReal32 nFloat = static_cast<AmReal32>(n);
                sumSquared += gains[n] * gains[n] * (2.0f * nFloat + 1.0f);
            }
            constexpr AmReal32 sumSquaredThreshold = 1e-10f;
            const AmReal32 energyComp = (sumSquared > sumSquaredThreshold) ? (N + 1.0f) / std::sqrt(sumSquared) : 1.0f;

            for (AmUInt32 n = 0; n <= order; ++n)
            {
                gains[n] *= energyComp;
            }
        }
        else
        {
            // 2D max-rE gains using cosine formula
            // Formula: a_n = cos(n * π / (2N + 2)) * sqrt((2N + 1) / (N + 1))
            // Reference: Daniel 2000, Zotter & Frank, libspatialaudio
            const AmReal32 energyComp = std::sqrt((2.0f * N + 1.0f) / (N + 1.0f));

            for (AmUInt32 n = 0; n <= order; ++n)
            {
                const AmReal32 nFloat = static_cast<AmReal32>(n);
                gains[n] = std::cos(nFloat * AM_PI32 / (2.0f * N + 2.0f)) * energyComp;
            }
        }

        return gains;
    }
} // namespace SparkyStudios::Audio::Amplitude
