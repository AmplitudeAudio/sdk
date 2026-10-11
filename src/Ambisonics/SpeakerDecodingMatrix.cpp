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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Math/Utils.h>

#include <Ambisonics/AmbisonicSource.h>
#include <Ambisonics/SpeakerDecodingMatrix.h>
#include <DSP/SpeakerPanner.h>
#include <Utils/lebedev-quadrature/lebedev_quadrature.hpp>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    namespace
    {
        // Max-rE angle for a 3D decoder (Zotter & Frank): a_l = P_l(cos(137.9° / (N + 1.51))).
        constexpr AmReal64 kMaxReAngleDegrees = 137.9;

        using Harmonics = std::array<AmReal64, SpeakerDecodingMatrix::kMaxAmbisonicChannels>;
        using Rows = std::array<Harmonics, kAmMaxLayoutChannelCount>;

        AmUInt32 DegreeOf(AmUInt32 channel)
        {
            return static_cast<AmUInt32>(std::floor(std::sqrt(static_cast<AmReal64>(channel))));
        }

        AmReal64 Legendre(AmUInt32 n, AmReal64 x)
        {
            AmReal64 previous = 1.0;
            AmReal64 current = x;
            if (n == 0)
                return previous;

            for (AmUInt32 i = 1; i < n; ++i)
            {
                const AmReal64 next = ((2.0 * i + 1.0) * x * current - i * previous) / (i + 1.0);
                previous = current;
                current = next;
            }

            return current;
        }

        /// Weighted mean, over the grid, of the decoded energy with the columns of @p rows scaled per degree by @p weights.
        AmReal64 MeanEnergy(
            const Rows& rows,
            const std::vector<Harmonics>& grid,
            const std::vector<AmReal64>& gridWeights,
            const std::vector<AmReal64>& degreeWeights,
            AmUInt16 outputs,
            AmUInt32 channels)
        {
            AmReal64 total = 0.0;
            for (std::size_t k = 0; k < grid.size(); ++k)
            {
                AmReal64 energy = 0.0;
                for (AmUInt16 s = 0; s < outputs; ++s)
                {
                    AmReal64 g = 0.0;
                    for (AmUInt32 c = 0; c < channels; ++c)
                        g += rows[s][c] * degreeWeights[DegreeOf(c)] * grid[k][c];
                    energy += g * g;
                }
                total += gridWeights[k] * energy;
            }
            return total;
        }
    } // namespace

    std::shared_ptr<const SpeakerDecodingMatrix> SpeakerDecodingMatrix::Build(AmUInt32 order, const ChannelLayout& layout)
    {
        if (order < 1 || order > 3)
            return nullptr;

        const AmUInt32 channels = OrderToComponents(order, true);
        const AmUInt16 outputs = layout.GetChannelCount();

        const lebedev::QuadraturePoints points(lebedev::QuadratureOrder::order_1202);
        const auto& x = points.get_x();
        const auto& y = points.get_y();
        const auto& z = points.get_z();
        const auto& w = points.get_weights();

        AmReal64 weightSum = 0.0;
        for (const AmReal64 weight : w)
            weightSum += weight;

        std::vector<Harmonics> grid(x.size());
        std::vector<AmReal64> gridWeights(x.size());

        AmbisonicSource source;
        source.Configure(order, true);

        Rows rows{};
        for (std::size_t k = 0; k < x.size(); ++k)
        {
            // Any orientation of the grid is a valid quadrature: x is front, y left, z up here.
            const AmReal64 azimuth = std::atan2(y[k], x[k]) * 180.0 / AM_PI;
            const AmReal64 elevation = std::asin(std::clamp(z[k], -1.0, 1.0)) * 180.0 / AM_PI;

            // The encoder's azimuth is negative to the left.
            source.SetPosition(SphericalPosition(
                static_cast<AmReal32>(-azimuth * AM_DegToRad), static_cast<AmReal32>(elevation * AM_DegToRad), 1.0f));
            for (AmUInt32 c = 0; c < channels; ++c)
                grid[k][c] = source.GetCoefficient(c);

            gridWeights[k] = w[k] / weightSum;

            const SpeakerGains gains = SpeakerPanner::PanSphere(static_cast<AmReal32>(azimuth), static_cast<AmReal32>(elevation), layout);
            for (AmUInt16 s = 0; s < outputs; ++s)
                if (!layout.IsLfe(s))
                    for (AmUInt32 c = 0; c < channels; ++c)
                        rows[s][c] += gridWeights[k] * gains[s] * grid[k][c];
        }

        // SN3D input: the (2l + 1) factor turns the grid projection into a decoder.
        for (AmUInt16 s = 0; s < outputs; ++s)
            for (AmUInt32 c = 0; c < channels; ++c)
                rows[s][c] *= 2.0 * DegreeOf(c) + 1.0;

        // Unit power on average, like the direct path.
        const std::vector<AmReal64> flat(order + 1, 1.0);
        const AmReal64 scale = 1.0 / std::sqrt(MeanEnergy(rows, grid, gridWeights, flat, outputs, channels));
        for (AmUInt16 s = 0; s < outputs; ++s)
            for (AmUInt32 c = 0; c < channels; ++c)
                rows[s][c] *= scale;

        // Max-rE above the shelf crossover, scaled to the low band's energy.
        std::vector<AmReal64> maxRe(order + 1);
        const AmReal64 cosine = std::cos(kMaxReAngleDegrees / (order + 1.51) * AM_PI / 180.0);
        for (AmUInt32 l = 0; l <= order; ++l)
            maxRe[l] = Legendre(l, cosine);
        const AmReal64 match = 1.0 / std::sqrt(MeanEnergy(rows, grid, gridWeights, maxRe, outputs, channels));

        auto matrix = ampoolshared(eMemoryPoolKind_Amplimix, SpeakerDecodingMatrix);
        matrix->_order = order;
        matrix->_layout = layout.GetType();
        matrix->_channelCount = channels;
        for (AmUInt16 s = 0; s < outputs; ++s)
            for (AmUInt32 c = 0; c < channels; ++c)
                matrix->_coefficients[s][c] = static_cast<AmReal32>(rows[s][c]);

        matrix->_highFrequencyGains.resize(order + 1);
        for (AmUInt32 l = 0; l <= order; ++l)
            matrix->_highFrequencyGains[l] = static_cast<AmReal32>(maxRe[l] * match);

        return matrix;
    }

    AmUInt32 SpeakerDecodingMatrix::GetOrder() const
    {
        return _order;
    }

    eChannelLayout SpeakerDecodingMatrix::GetLayout() const
    {
        return _layout;
    }

    AmUInt32 SpeakerDecodingMatrix::GetChannelCount() const
    {
        return _channelCount;
    }

    AmReal32 SpeakerDecodingMatrix::GetCoefficient(AmUInt16 outputChannel, AmUInt32 ambisonicChannel) const
    {
        if (outputChannel >= kAmMaxLayoutChannelCount || ambisonicChannel >= _channelCount)
            return 0.0f;

        return _coefficients[outputChannel][ambisonicChannel];
    }

    const std::vector<AmReal32>& SpeakerDecodingMatrix::GetHighFrequencyGains() const
    {
        return _highFrequencyGains;
    }
} // namespace SparkyStudios::Audio::Amplitude
