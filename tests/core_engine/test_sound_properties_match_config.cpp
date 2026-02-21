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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, core_engine, sound_properties_match_config)
    {
    public:
        void Run() override
        {
            // test_sound_01: non-streaming, non-looping, HRTF spatialization, world scope
            {
                SoundHandle sound = amEngine->GetSoundHandle("test_sound_01");
                AM_EXPECT_NOT(sound == nullptr);

                AM_EXPECT_EQ(sound->GetId(), static_cast<AmSoundID>(9991));
                AM_EXPECT(sound->IsStream() == false);
                AM_EXPECT(sound->IsLoop() == false);
                AM_EXPECT(sound->GetSpatialization() == eSpatialization_HRTF);
                AM_EXPECT(sound->GetScope() == eScope_World);

                // Gain, pitch, priority should be 1.0 (static)
                AM_EXPECT(std::abs(sound->GetGain().GetValue() - 1.0f) < kEpsilon);
                AM_EXPECT(std::abs(sound->GetPitch().GetValue() - 1.0f) < kEpsilon);
                AM_EXPECT(std::abs(sound->GetPriority().GetValue() - 1.0f) < kEpsilon);

                // No effect or attenuation attached
                AM_EXPECT(sound->GetEffect() == nullptr);
                AM_EXPECT(sound->GetAttenuation() == nullptr);
            }

            // test_sound_02: streaming, non-looping, no spatialization
            {
                SoundHandle sound = amEngine->GetSoundHandle("test_sound_02");
                AM_EXPECT_NOT(sound == nullptr);

                AM_EXPECT_EQ(sound->GetId(), static_cast<AmSoundID>(9992));
                AM_EXPECT(sound->IsStream() == true);
                AM_EXPECT(sound->IsLoop() == false);
                AM_EXPECT(sound->GetSpatialization() == eSpatialization_None);
                AM_EXPECT(sound->GetScope() == eScope_World);
            }

            // test_sound_03: non-streaming, non-looping, no spatialization
            {
                SoundHandle sound = amEngine->GetSoundHandle("test_sound_03");
                AM_EXPECT_NOT(sound == nullptr);

                AM_EXPECT_EQ(sound->GetId(), static_cast<AmSoundID>(9993));
                AM_EXPECT(sound->IsStream() == false);
                AM_EXPECT(sound->IsLoop() == false);
                AM_EXPECT(sound->GetSpatialization() == eSpatialization_None);
            }
        }
    };

    AM_REGISTER_TEST(core_engine, sound_properties_match_config);
} // namespace SparkyStudios::Audio::Amplitude::Tests
