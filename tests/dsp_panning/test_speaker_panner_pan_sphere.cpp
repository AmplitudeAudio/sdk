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

#include <cmath>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <DSP/SpeakerPanner.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(PureUnitTestCase, dsp_panning, speaker_panner_pan_sphere)
    {
    public:
        void Run() override
        {
            for (AmUInt8 type = 0; type < eChannelLayout_COUNT; ++type)
            {
                const ChannelLayout& layout = ChannelLayout::Get(static_cast<eChannelLayout>(type));
                const AmInt32 lfe = layout.GetLfeChannel();

                AmUInt16 speakers = 0;
                for (AmUInt16 c = 0; c < layout.GetChannelCount(); ++c)
                    speakers += layout.IsLfe(c) ? 0 : 1;

                // On the horizon it is the direct path's pan law.
                for (AmInt32 azimuth = -180; azimuth < 180; ++azimuth)
                {
                    const SpeakerGains flat = SpeakerPanner::Pan(static_cast<AmReal32>(azimuth), 0.0f, layout);
                    const SpeakerGains sphere = SpeakerPanner::PanSphere(static_cast<AmReal32>(azimuth), 0.0f, layout);
                    for (AmUInt16 c = 0; c < layout.GetChannelCount(); ++c)
                        AM_EXPECT(std::abs(flat[c] - sphere[c]) <= 1e-6f);
                }

                // Power is preserved everywhere, gains are finite and non-negative, the LFE stays silent.
                for (AmInt32 elevation = -90; elevation <= 90; elevation += 15)
                {
                    for (AmInt32 azimuth = -180; azimuth < 180; azimuth += 5)
                    {
                        const SpeakerGains g =
                            SpeakerPanner::PanSphere(static_cast<AmReal32>(azimuth), static_cast<AmReal32>(elevation), layout);
                        AmReal64 power = 0.0;
                        for (AmUInt16 c = 0; c < layout.GetChannelCount(); ++c)
                        {
                            AM_EXPECT(std::isfinite(g[c]) && g[c] >= 0.0f);
                            power += static_cast<AmReal64>(g[c]) * g[c];
                        }
                        AM_EXPECT(std::abs(power - 1.0) <= 1e-5);
                        if (lfe >= 0)
                            AM_EXPECT_EQ(0.0f, g[lfe]);
                    }
                }

                // Straight up or down: every speaker gets the same share; out-of-range elevations clamp.
                for (const AmReal32 elevation : { 90.0f, -90.0f, 120.0f, -135.0f })
                {
                    const SpeakerGains g = SpeakerPanner::PanSphere(37.0f, elevation, layout);
                    const AmReal32 share = 1.0f / std::sqrt(static_cast<AmReal32>(speakers));
                    for (AmUInt16 c = 0; c < layout.GetChannelCount(); ++c)
                        AM_EXPECT(std::abs(g[c] - (layout.IsLfe(c) ? 0.0f : share)) <= 1e-5f);
                }

                // Exactly on a speaker at the horizon: that speaker alone.
                for (AmUInt16 c = 0; c < layout.GetChannelCount(); ++c)
                {
                    if (layout.IsLfe(c) || speakers == 1)
                        continue;

                    const SpeakerGains g = SpeakerPanner::PanSphere(layout.GetAzimuth(c), 0.0f, layout);
                    AM_EXPECT(std::abs(g[c] - 1.0f) <= 1e-5f);
                }
            }

            // Stereo: a rear direction folds to its mirror in front, as Pan does, at any elevation.
            const ChannelLayout& stereo = ChannelLayout::Get(eChannelLayout_Stereo);
            const SpeakerGains rear = SpeakerPanner::PanSphere(150.0f, 30.0f, stereo);
            const SpeakerGains front = SpeakerPanner::PanSphere(30.0f, 30.0f, stereo);
            AM_EXPECT(std::abs(rear[0] - front[0]) <= 1e-6f && std::abs(rear[1] - front[1]) <= 1e-6f);
        }
    };

    AM_REGISTER_TEST(dsp_panning, speaker_panner_pan_sphere);
} // namespace SparkyStudios::Audio::Amplitude::Tests
