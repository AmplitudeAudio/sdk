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
    AM_TEST_CASE(PureUnitTestCase, dsp_panning, speaker_panner_preserves_power)
    {
    public:
        void Run() override
        {
            for (AmUInt8 type = 0; type < eChannelLayout_COUNT; ++type)
            {
                const ChannelLayout& layout = ChannelLayout::Get(static_cast<eChannelLayout>(type));
                for (AmInt32 step = -1800; step < 1800; ++step)
                {
                    const AmReal32 azimuth = static_cast<AmReal32>(step) * 0.1f;
                    const SpeakerGains gains = SpeakerPanner::Pan(azimuth, 0.0f, layout);

                    AmReal64 power = 0.0;
                    for (AmUInt16 c = 0; c < layout.GetChannelCount(); ++c)
                    {
                        AM_EXPECT(gains[c] >= 0.0f);
                        power += static_cast<AmReal64>(gains[c]) * gains[c];
                    }
                    AM_EXPECT(std::abs(power - 1.0) < 1e-6);

                    // The LFE never takes a panned signal.
                    if (const AmInt32 lfe = layout.GetLfeChannel(); lfe >= 0)
                        AM_EXPECT_EQ(0.0f, gains[lfe]);

                    // No layout has height speakers: elevation leaves the gains unchanged.
                    const SpeakerGains raised = SpeakerPanner::Pan(azimuth, 45.0f, layout);
                    for (AmUInt16 c = 0; c < layout.GetChannelCount(); ++c)
                        AM_EXPECT_EQ(gains[c], raised[c]);
                }
            }
        }
    };

    AM_REGISTER_TEST(dsp_panning, speaker_panner_preserves_power);
} // namespace SparkyStudios::Audio::Amplitude::Tests
