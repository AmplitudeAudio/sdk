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
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Ambisonics/AmbisonicSource.h>
#include <Ambisonics/SpeakerDecodingMatrix.h>
#include <Utils/lebedev-quadrature/lebedev_quadrature.hpp>
#include <Utils/Utils.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, ambisonics_decoder, speaker_decoding_matrix_meets_targets)
    {
    public:
        void Run() override
        {
            AM_EXPECT(SpeakerDecodingMatrix::Build(0, ChannelLayout::Get(eChannelLayout_Stereo)) == nullptr);
            AM_EXPECT(SpeakerDecodingMatrix::Build(4, ChannelLayout::Get(eChannelLayout_Stereo)) == nullptr);

            for (AmUInt32 order = 1; order <= 3; ++order)
            {
                CheckMono(order);
                for (const Target& target : Targets())
                    if (target.order == order)
                        CheckTarget(target);
            }
        }

    private:
        struct Target
        {
            eChannelLayout layout;
            AmUInt32 order;
            AmReal64 lfSpread;
            AmReal64 hfSpread;
            AmReal64 meanError; // negative: not checked (stereo)
            AmReal64 maxError;
            AmReal64 leak;      // 5.1 only; 0: not checked
        };

        static const std::vector<Target>& Targets()
        {
            static const std::vector<Target> targets{
                { eChannelLayout_Quad, 1, 0.2, 0.2, 1.0, 2.0, 0.0 },
                { eChannelLayout_Quad, 2, 0.94, 0.34, 4.4, 7.3, 0.0 },
                { eChannelLayout_Quad, 3, 0.98, 0.52, 6.2, 10.1, 0.0 },
                { eChannelLayout_Surround_5_1, 1, 1.87, 1.45, 12.2, 24.9, -4.5 },
                { eChannelLayout_Surround_5_1, 2, 2.33, 1.77, 9.3, 25.7, -7.1 },
                { eChannelLayout_Surround_5_1, 3, 2.33, 2.03, 8.4, 23.6, -5.8 },
                { eChannelLayout_Surround_7_1, 1, 0.79, 0.57, 3.2, 6.2, 0.0 },
                { eChannelLayout_Surround_7_1, 2, 1.63, 1.01, 1.9, 4.2, 0.0 },
                { eChannelLayout_Surround_7_1, 3, 1.91, 1.30, 2.2, 5.2, 0.0 },
                { eChannelLayout_Stereo, 1, 2.42, 1.07, -1.0, -1.0, 0.0 },
                { eChannelLayout_Stereo, 2, 1.44, 1.21, -1.0, -1.0, 0.0 },
                { eChannelLayout_Stereo, 3, 1.71, 1.09, -1.0, -1.0, 0.0 },
            };
            return targets;
        }

        /// SN3D harmonics of a direction given in the layout's convention (azimuth positive left), as the encoder makes them.
        static std::vector<AmReal64> Harmonics(AmUInt32 order, AmReal64 azimuth, AmReal64 elevation)
        {
            AmbisonicSource source;
            source.Configure(order, true);
            source.SetPosition(SphericalPosition(static_cast<AmReal32>(-azimuth * AM_DegToRad), static_cast<AmReal32>(elevation * AM_DegToRad), 1.0f));

            std::vector<AmReal64> y(OrderToComponents(order, true));
            for (AmUInt32 c = 0; c < y.size(); ++c)
                y[c] = source.GetCoefficient(c);
            return y;
        }

        /// Speaker gains for a direction: the matrix, its columns weighted per degree by @p weights.
        static std::vector<AmReal64> Decode(
            const SpeakerDecodingMatrix& m, const ChannelLayout& layout, const std::vector<AmReal64>& y, const std::vector<AmReal64>& weights)
        {
            std::vector<AmReal64> g(layout.GetChannelCount(), 0.0);
            for (AmUInt16 s = 0; s < layout.GetChannelCount(); ++s)
                for (AmUInt32 c = 0; c < y.size(); ++c)
                    g[s] += m.GetCoefficient(s, c) * weights[static_cast<AmUInt32>(std::floor(std::sqrt(static_cast<AmReal64>(c))))] * y[c];
            return g;
        }

        static AmReal64 Energy(const std::vector<AmReal64>& g)
        {
            AmReal64 e = 0.0;
            for (const AmReal64 v : g)
                e += v * v;
            return e;
        }

        /// Mean energy over the 1202-point Lebedev grid.
        static AmReal64 GridEnergy(const SpeakerDecodingMatrix& m, const ChannelLayout& layout, const std::vector<AmReal64>& weights)
        {
            const lebedev::QuadraturePoints grid(lebedev::QuadratureOrder::order_1202);
            AmReal64 total = 0.0;
            for (std::size_t k = 0; k < grid.get_x().size(); ++k)
            {
                const AmReal64 az = std::atan2(grid.get_y()[k], grid.get_x()[k]) * AM_RadToDeg;
                const AmReal64 el = std::asin(std::clamp(grid.get_z()[k], -1.0, 1.0)) * AM_RadToDeg;
                total += grid.get_weights()[k] * Energy(Decode(m, layout, Harmonics(m.GetOrder(), az, el), weights));
            }
            return total;
        }

        void CheckMono(AmUInt32 order)
        {
            const ChannelLayout& mono = ChannelLayout::Get(eChannelLayout_Mono);
            const auto m = SpeakerDecodingMatrix::Build(order, mono);
            AM_EXPECT(m != nullptr);
            for (AmUInt32 c = 1; c < m->GetChannelCount(); ++c)
                AM_EXPECT(std::abs(m->GetCoefficient(0, c)) < 1e-6f);
            AM_EXPECT(m->GetCoefficient(0, 0) > 0.0f);
        }

        void CheckTarget(const Target& target)
        {
            const ChannelLayout& layout = ChannelLayout::Get(target.layout);
            const auto m = SpeakerDecodingMatrix::Build(target.order, layout);
            AM_EXPECT(m != nullptr);
            AM_EXPECT(m->GetOrder() == target.order);
            AM_EXPECT(m->GetLayout() == target.layout);
            AM_EXPECT(m->GetChannelCount() == OrderToComponents(target.order, true));
            AM_EXPECT(m->GetHighFrequencyGains().size() == target.order + 1);

            const std::vector<AmReal64> lf(target.order + 1, 1.0);
            std::vector<AmReal64> hf;
            for (const AmReal32 g : m->GetHighFrequencyGains())
                hf.push_back(g);

            // LFE rows are zero.
            if (const AmInt32 lfe = layout.GetLfeChannel(); lfe >= 0)
                for (AmUInt32 c = 0; c < m->GetChannelCount(); ++c)
                    AM_EXPECT_EQ(0.0f, m->GetCoefficient(static_cast<AmUInt16>(lfe), c));

            // Loudness: the direct path's unit power on average, and the two bands alike.
            const AmReal64 lfEnergy = GridEnergy(*m, layout, lf);
            const AmReal64 hfEnergy = GridEnergy(*m, layout, hf);
            AM_EXPECT(std::abs(10.0 * std::log10(lfEnergy)) <= 0.05);
            AM_EXPECT(std::abs(10.0 * std::log10(hfEnergy / lfEnergy)) <= 0.1);

            AmReal64 lfMin = 1e30, lfMax = 0.0, hfMin = 1e30, hfMax = 0.0, errorSum = 0.0, errorMax = 0.0;
            for (AmInt32 a = 0; a < 360; ++a)
            {
                const std::vector<AmReal64> y = Harmonics(target.order, a, 0.0);
                const AmReal64 el = Energy(Decode(*m, layout, y, lf));
                const std::vector<AmReal64> g = Decode(*m, layout, y, hf);
                const AmReal64 eh = Energy(g);
                lfMin = std::min(lfMin, el);
                lfMax = std::max(lfMax, el);
                hfMin = std::min(hfMin, eh);
                hfMax = std::max(hfMax, eh);

                // Energy vector direction of the high band.
                AmReal64 rx = 0.0, ry = 0.0;
                for (AmUInt16 s = 0; s < layout.GetChannelCount(); ++s)
                {
                    if (layout.IsLfe(s))
                        continue;
                    rx += g[s] * g[s] * std::cos(layout.GetAzimuth(s) * AM_DegToRad);
                    ry += g[s] * g[s] * std::sin(layout.GetAzimuth(s) * AM_DegToRad);
                }
                AmReal64 error = std::abs(std::fmod(std::atan2(ry, rx) * AM_RadToDeg - a + 540.0, 360.0) - 180.0);
                errorSum += error;
                errorMax = std::max(errorMax, error);
            }

            AM_EXPECT(10.0 * std::log10(lfMax / lfMin) <= target.lfSpread);
            AM_EXPECT(10.0 * std::log10(hfMax / hfMin) <= target.hfSpread);
            if (target.meanError >= 0.0)
            {
                AM_EXPECT(errorSum / 360.0 <= target.meanError);
                AM_EXPECT(errorMax <= target.maxError);
            }

            // 5.1: a front-left source must reach Ls more than Rs.
            if (target.leak != 0.0)
            {
                const std::vector<AmReal64> g = Decode(*m, layout, Harmonics(target.order, 30.0, 0.0), hf);
                AM_EXPECT(10.0 * std::log10(g[5] * g[5] / (g[4] * g[4])) <= target.leak);
            }
        }
    };

    AM_REGISTER_TEST(ambisonics_decoder, speaker_decoding_matrix_meets_targets);
} // namespace SparkyStudios::Audio::Amplitude::Tests
