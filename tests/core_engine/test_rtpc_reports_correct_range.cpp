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
    AM_TEST_CASE(EngineTestCase, core_engine, rtpc_reports_correct_range)
    {
    public:
        void Run() override
        {
            RtpcHandle rtpc = amEngine->GetRtpcHandle(1);
            AM_EXPECT_NOT(rtpc == nullptr);

            // Min must be less than or equal to max
            AM_EXPECT(rtpc->GetMinValue() <= rtpc->GetMaxValue());

            // Default must be within [min, max]
            AM_EXPECT(rtpc->GetDefaultValue() >= rtpc->GetMinValue());
            AM_EXPECT(rtpc->GetDefaultValue() <= rtpc->GetMaxValue());
        }
    };

    AM_REGISTER_TEST(core_engine, rtpc_reports_correct_range);
} // namespace SparkyStudios::Audio::Amplitude::Tests
