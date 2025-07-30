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
    auto inner = std::make_shared<CapsuleShape>(25, 50);
    auto outer = std::make_shared<CapsuleShape>(50, 100);
    auto zone = std::make_shared<CapsuleZone>(inner, outer);

    const auto direction = kVector3UnitX;
    const auto up = kVector3UnitZ;
    const auto newOrientation = Orientation(direction, up);
    zone->SetOrientation(newOrientation);

    constexpr auto kOrientationSize = sizeof(Orientation);

    ExpectTrue(std::memcmp(&zone->GetOrientation(), &newOrientation, kOrientationSize) == 0, "Orientation should be updated");

    ExpectTrue(
        std::memcmp(&inner->GetOrientation(), &newOrientation, kOrientationSize) == 0, "Inner capsule orientation should be updated");
    ExpectTrue(
        std::memcmp(&outer->GetOrientation(), &newOrientation, kOrientationSize) == 0, "Outer capsule orientation should be updated");

    ExpectEqual(zone->GetDirection(), newOrientation.GetForward(), "Direction should be updated");
    ExpectEqual(zone->GetUp(), newOrientation.GetUp(), "Up should be updated");
}
