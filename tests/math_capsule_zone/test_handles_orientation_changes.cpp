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

using namespace SparkyStudios::Audio::Amplitude;

void SimpleTestCase::Run()
{
    auto inner = AmSharedPtr<CapsuleShape>::Make(25, 50);
    auto outer = AmSharedPtr<CapsuleShape>::Make(50, 100);
    std::shared_ptr<Zone> zone = AmSharedPtr<CapsuleZone>::Make(inner, outer);

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
