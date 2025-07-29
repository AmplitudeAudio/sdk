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
    auto inner = AmSharedPtr<SphereShape>::Make(50);
    auto outer = AmSharedPtr<SphereShape>::Make(100);
    std::shared_ptr<Zone> zone = AmSharedPtr<SphereZone>::Make(inner, outer);

    const AmVector3 newLocation = { 10, 20, 30 };
    zone->SetLocation(newLocation);

    AM_EXPECT_EQ(zone->GetLocation(), newLocation);

    AM_EXPECT_EQ(inner->GetLocation(), newLocation);
    AM_EXPECT_EQ(outer->GetLocation(), newLocation);
}
