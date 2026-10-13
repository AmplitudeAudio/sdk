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

#include <DSP/SpeakerPanner.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(PureUnitTestCase, dsp_panning, speaker_panner_midpoint_is_minus_3db)
    {
    public:
        void Run() override
        {
            constexpr AmReal32 k = 0.70710678f;

            for (AmUInt8 type = eChannelLayout_Stereo; type < eChannelLayout_COUNT; ++type)
            {
                const ChannelLayout& layout = ChannelLayout::Get(static_cast<eChannelLayout>(type));

                std::vector<AmUInt16> ring;
                for (AmUInt16 c = 0; c < layout.GetChannelCount(); ++c)
                    if (!layout.IsLfe(c))
                        ring.push_back(c);
                std::sort(
                    ring.begin(), ring.end(),
                    [&](AmUInt16 a, AmUInt16 b)
                    {
                        return layout.GetAzimuth(a) < layout.GetAzimuth(b);
                    });

                // Stereo has one pair; the other layouts close the circle behind the listener.
                const AmSize pairs = ring.size() == 2 ? 1 : ring.size();
                for (AmSize j = 0; j < pairs; ++j)
                {
                    const AmUInt16 a = ring[j];
                    const AmUInt16 b = ring[(j + 1) % ring.size()];
                    AmReal32 span = layout.GetAzimuth(b) - layout.GetAzimuth(a);
                    if (span <= 0.0f)
                        span += 360.0f;

                    const SpeakerGains gains = SpeakerPanner::Pan(WrapDegrees(layout.GetAzimuth(a) + 0.5f * span), 0.0f, layout);
                    AM_EXPECT(std::abs(gains[a] - k) < 1e-6f);
                    AM_EXPECT(std::abs(gains[b] - k) < 1e-6f);
                }
            }
        }
    };

    AM_REGISTER_TEST(dsp_panning, speaker_panner_midpoint_is_minus_3db);
} // namespace SparkyStudios::Audio::Amplitude::Tests
