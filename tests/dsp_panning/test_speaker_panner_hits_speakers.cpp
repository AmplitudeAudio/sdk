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
    AM_TEST_CASE(PureUnitTestCase, dsp_panning, speaker_panner_hits_speakers)
    {
    public:
        void Run() override
        {
            for (AmUInt8 type = 0; type < eChannelLayout_COUNT; ++type)
            {
                const ChannelLayout& layout = ChannelLayout::Get(static_cast<eChannelLayout>(type));
                for (AmUInt16 speaker = 0; speaker < layout.GetChannelCount(); ++speaker)
                {
                    if (layout.IsLfe(speaker))
                        continue;

                    const SpeakerGains gains = SpeakerPanner::Pan(layout.GetAzimuth(speaker), 0.0f, layout);
                    for (AmUInt16 c = 0; c < layout.GetChannelCount(); ++c)
                        AM_EXPECT(std::abs(gains[c] - (c == speaker ? 1.0f : 0.0f)) < 1e-6f);
                }

                // Left/right symmetry: a mirrored direction gives the mirrored speaker the same gain.
                for (AmInt32 step = -179; step <= 179; ++step)
                {
                    const auto azimuth = static_cast<AmReal32>(step);
                    const SpeakerGains gains = SpeakerPanner::Pan(azimuth, 0.0f, layout);
                    const SpeakerGains mirrored = SpeakerPanner::Pan(-azimuth, 0.0f, layout);

                    for (AmUInt16 c = 0; c < layout.GetChannelCount(); ++c)
                    {
                        if (layout.IsLfe(c))
                            continue;

                        AmInt32 mirror = -1;
                        for (AmUInt16 m = 0; m < layout.GetChannelCount(); ++m)
                            if (!layout.IsLfe(m) && layout.GetAzimuth(m) == -layout.GetAzimuth(c))
                                mirror = m;

                        AM_EXPECT(mirror >= 0);
                        if (mirror >= 0)
                            AM_EXPECT(std::abs(gains[c] - mirrored[mirror]) < 1e-6f);
                    }
                }
            }
        }
    };

    AM_REGISTER_TEST(dsp_panning, speaker_panner_hits_speakers);
} // namespace SparkyStudios::Audio::Amplitude::Tests
