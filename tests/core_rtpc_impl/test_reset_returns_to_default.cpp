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

#include <Sound/Rtpc.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, core_rtpc_impl, reset_returns_to_default)
    {
    public:
        void Run() override
        {
            // Default range is [0, 1], default value is 0, no faders
            RtpcImpl rtpc;

            rtpc.SetValue(0.8);
            AM_EXPECT(std::abs(rtpc.GetValue() - 0.8) < 1e-9);

            rtpc.Reset();
            AM_EXPECT(std::abs(rtpc.GetValue() - rtpc.GetDefaultValue()) < 1e-9);
        }
    };

    AM_REGISTER_TEST(core_rtpc_impl, reset_returns_to_default);
} // namespace SparkyStudios::Audio::Amplitude::Tests
