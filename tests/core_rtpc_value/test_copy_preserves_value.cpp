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

#include "SimpleTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, core_rtpc_value, copy_preserves_value)
    {
    public:
        void Run() override
        {
            RtpcValue original;
            original.Init(0.42f);

            RtpcValue copy(original);

            AM_EXPECT(copy.IsStatic());
            AM_EXPECT(std::abs(copy.GetValue() - 0.42f) < 1e-6f);
        }
    };

    AM_REGISTER_TEST(core_rtpc_value, copy_preserves_value);
} // namespace SparkyStudios::Audio::Amplitude::Tests
