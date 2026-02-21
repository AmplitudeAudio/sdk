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

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, math_box_zone, handles_location_changes)
    {
    public:
        void Run() override
        {
            auto inner = amshared(BoxShape, 50, 50, 50);
            auto outer = amshared(BoxShape, 100, 100, 100);
            std::shared_ptr<Zone> zone = amshared(BoxZone, inner, outer);

            const AmVector3 newLocation = { 10, 20, 30 };
            zone->SetLocation(newLocation);

            AM_EXPECT_EQ(zone->GetLocation(), newLocation);

            AM_EXPECT_EQ(inner->GetLocation(), newLocation);
            AM_EXPECT_EQ(outer->GetLocation(), newLocation);
        }
    };

    AM_REGISTER_TEST(math_box_zone, handles_location_changes);
} // namespace SparkyStudios::Audio::Amplitude::Tests
