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
#include <DSP/SpreadEmitters.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(PureUnitTestCase, dsp_panning, spread_emitters_never_coincide)
    {
    public:
        void Run() override
        {
            for (AmUInt8 type = eChannelLayout_Stereo; type < eChannelLayout_COUNT; ++type)
            {
                const ChannelLayout& layout = ChannelLayout::Get(static_cast<eChannelLayout>(type));
                for (AmInt32 spread = 1; spread <= 360; ++spread)
                {
                    const SpreadEmitterSet set = SpreadEmitters::Compute(layout, 0.0f, 0.0f, static_cast<AmReal32>(spread));
                    for (AmUInt16 a = 0; a < set.count; ++a)
                        for (AmUInt16 b = a + 1; b < set.count; ++b)
                            AM_EXPECT(std::abs(WrapDegrees(set.emitters[a].azimuth - set.emitters[b].azimuth)) > 1e-3f);
                }
            }
        }
    };

    AM_REGISTER_TEST(dsp_panning, spread_emitters_never_coincide);
} // namespace SparkyStudios::Audio::Amplitude::Tests
