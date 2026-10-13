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

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, core_engine, sound_and_attenuation_read_spread)
    {
    public:
        void Run() override
        {
            const SoundHandle spread = amEngine->GetSoundHandle("test_sound_07");
            AM_EXPECT_NOT(spread == nullptr);
            if (spread != nullptr)
                AM_EXPECT_EQ(90.0f, spread->GetSpread().GetValue());

            // No spread in the definition: the asset's native width.
            const SoundHandle native = amEngine->GetSoundHandle("test_sound_06");
            AM_EXPECT_NOT(native == nullptr);
            if (native != nullptr)
                AM_EXPECT_EQ(kAmSpreadNative, native->GetSpread().GetValue());

            const AttenuationHandle curve = amEngine->GetAttenuationHandle("tests_spread");
            AM_EXPECT_NOT(curve == nullptr);
            if (curve != nullptr)
            {
                AM_EXPECT(std::abs(curve->GetSpreadFactor(0.0) - 1.0f) < 1e-6f);
                AM_EXPECT(std::abs(curve->GetSpreadFactor(50.0) - 0.5f) < 1e-6f);
                AM_EXPECT(std::abs(curve->GetSpreadFactor(100.0)) < 1e-6f);
                AM_EXPECT(curve->GetSpreadFactor(500.0) >= 0.0f && curve->GetSpreadFactor(500.0) <= 1.0f);
            }

            // No spread curve: factor 1 at every distance.
            const AttenuationHandle plain = amEngine->GetAttenuationHandle("impact");
            AM_EXPECT_NOT(plain == nullptr);
            if (plain != nullptr)
            {
                AM_EXPECT_EQ(1.0f, plain->GetSpreadFactor(0.0));
                AM_EXPECT_EQ(1.0f, plain->GetSpreadFactor(900.0));
            }
        }
    };

    AM_REGISTER_TEST(core_engine, sound_and_attenuation_read_spread);
} // namespace SparkyStudios::Audio::Amplitude::Tests
