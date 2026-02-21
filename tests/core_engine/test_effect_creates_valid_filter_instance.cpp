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
    AM_TEST_CASE(EngineTestCase, core_engine, effect_creates_valid_filter_instance)
    {
    public:
        void Run() override
        {
            // Test LPF effect (BiquadResonant, ID=6)
            {
                auto* effect = amEngine->GetEffectHandle("lpf");
                AM_EXPECT_NOT(effect == nullptr);
                AM_EXPECT_EQ(effect->GetId(), static_cast<AmEffectID>(6));

                auto instance = effect->CreateInstance();
                AM_EXPECT_NOT(instance == nullptr);

                auto filter = instance->GetFilter();
                AM_EXPECT_NOT(filter == nullptr);

                // LPF config: wet=1, type=0 (lowpass), freq=2400, resonance=1, gain=0
                AM_EXPECT(std::abs(filter->GetParameter(0) - 1.0f) < kEpsilon); // Wet
                AM_EXPECT(std::abs(filter->GetParameter(2) - 2400.0f) < kEpsilon); // Frequency
            }

            // Test BassBoost effect (ID=5)
            {
                auto* effect = amEngine->GetEffectHandle("bassboost");
                AM_EXPECT_NOT(effect == nullptr);
                AM_EXPECT_EQ(effect->GetId(), static_cast<AmEffectID>(5));

                auto instance = effect->CreateInstance();
                AM_EXPECT_NOT(instance == nullptr);

                auto filter = instance->GetFilter();
                AM_EXPECT_NOT(filter == nullptr);

                // BassBoost config: wet=1, boost=11
                AM_EXPECT(std::abs(filter->GetParameter(0) - 1.0f) < kEpsilon); // Wet
                AM_EXPECT(std::abs(filter->GetParameter(1) - 11.0f) < kEpsilon); // Boost amount
            }
        }
    };

    AM_REGISTER_TEST(core_engine, effect_creates_valid_filter_instance);
} // namespace SparkyStudios::Audio::Amplitude::Tests
