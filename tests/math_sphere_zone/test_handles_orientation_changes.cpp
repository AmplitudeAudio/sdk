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

#include "SimpleTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, math_sphere_zone, handles_orientation_changes)
    {
    public:
        void Run() override
        {
            auto inner = amshared(SphereShape, 50);
            auto outer = amshared(SphereShape, 100);
            std::shared_ptr<Zone> zone = amshared(SphereZone, inner, outer);

            const auto direction = kVector3UnitX;
            const auto up = kVector3UnitZ;
            const auto newOrientation = Orientation(direction, up);
            zone->SetOrientation(newOrientation);

            constexpr auto kOrientationSize = sizeof(Orientation);

            AM_EXPECT(std::memcmp(&zone->GetOrientation(), &newOrientation, kOrientationSize) == 0);

            AM_EXPECT(std::memcmp(&inner->GetOrientation(), &newOrientation, kOrientationSize) == 0);
            AM_EXPECT(std::memcmp(&outer->GetOrientation(), &newOrientation, kOrientationSize) == 0);

            AM_EXPECT_EQ(zone->GetDirection(), newOrientation.GetForward());
            AM_EXPECT_EQ(zone->GetUp(), newOrientation.GetUp());
        }
    };

    AM_REGISTER_TEST(math_sphere_zone, handles_orientation_changes);
} // namespace SparkyStudios::Audio::Amplitude::Tests
