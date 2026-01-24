// Copyright (c) 2021-present Sparky Studios. All rights reserved.
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
    AM_TEST_CASE(EngineTestCase, core_engine, can_load_sound_banks)
    {
    public:
        void Run() override
        {
            // Test loading the same sound bank again
            AM_EXPECT(amEngine->LoadSoundBank(AM_OS_STRING("tests.init.ambank")));

            // Test loading other sound banks
            AM_EXPECT(amEngine->LoadSoundBank(AM_OS_STRING("sample_01.ambank")));
            AM_EXPECT(amEngine->LoadSoundBank(AM_OS_STRING("sample_02.ambank")));
        }
    };

    AM_REGISTER_TEST(core_engine, can_load_sound_banks);
} // namespace SparkyStudios::Audio::Amplitude::Tests
