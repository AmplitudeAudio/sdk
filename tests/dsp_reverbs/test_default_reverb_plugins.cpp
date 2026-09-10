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
#include <SparkyStudios/Audio/Amplitude/DSP/Reverb.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, dsp_reverbs, default_reverb_plugins)
    {
    public:
        void Run() override
        {
            AM_EXPECT_NOT(Reverb::Find("Freeverb") == nullptr);
            AM_EXPECT_NOT(Reverb::Find("Dattorro") == nullptr);
            AM_EXPECT_NOT(Reverb::Find("RoomReverb") == nullptr);
        }
    };

    AM_REGISTER_TEST(dsp_reverbs, default_reverb_plugins);
} // namespace SparkyStudios::Audio::Amplitude::Tests
