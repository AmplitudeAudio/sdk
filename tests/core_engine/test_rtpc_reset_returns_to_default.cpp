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
    AM_TEST_CASE(EngineTestCase, core_engine, rtpc_reset_returns_to_default)
    {
    public:
        void Run() override
        {
            RtpcHandle rtpc = amEngine->GetRtpcHandle(1);
            AM_EXPECT_NOT(rtpc == nullptr);

            AmReal64 defaultValue = rtpc->GetDefaultValue();

            // Change value away from default
            rtpc->SetValue(rtpc->GetMaxValue());
            amEngine->WaitUntilFrames(2);

            // Reset should return to default
            rtpc->Reset();
            amEngine->WaitUntilFrames(2);

            AM_EXPECT(std::abs(rtpc->GetValue() - defaultValue) < 0.01);
        }
    };

    AM_REGISTER_TEST(core_engine, rtpc_reset_returns_to_default);
} // namespace SparkyStudios::Audio::Amplitude::Tests
