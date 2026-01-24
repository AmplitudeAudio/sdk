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
    AM_TEST_CASE(EngineTestCase, core_engine, bus_can_be_muted)
    {
    public:
        void Run() override
        {
            Bus bus = amEngine->FindBus(kAmMasterBusId);
            AM_EXPECT(bus.Valid());

            // Test muting the bus
            bus.SetMute(true);
            AM_EXPECT(bus.IsMuted());

            // Test unmuting the bus
            bus.SetMute(false);
            AM_EXPECT_NOT(bus.IsMuted());
        }
    };

    AM_REGISTER_TEST(core_engine, bus_can_be_muted);
} // namespace SparkyStudios::Audio::Amplitude::Tests
