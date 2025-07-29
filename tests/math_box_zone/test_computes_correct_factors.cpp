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
    auto inner = AmSharedPtr<BoxShape>::Make(50, 50, 50);
    auto outer = AmSharedPtr<BoxShape>::Make(100, 100, 100);
    std::shared_ptr<Zone> zone = AmSharedPtr<BoxZone>::Make(inner, outer);

    const AmVector3 innerPoint = { 25, 25, 25 };
    const AmVector3 outerPoint = { 175, 175, 175 };
    const AmVector3 middlePoint = { 87.5f, 87.5f, 87.5f };

    AM_EXPECT_EQ(zone->GetFactor(innerPoint), 1.0f);
    AM_EXPECT_EQ(zone->GetFactor(outerPoint), 0.0f);
    AM_EXPECT_EQ(zone->GetFactor(middlePoint), 0.25f);

    {
        EntityInternalState innerState;
        EntityInternalState outerState;
        EntityInternalState middleState;

        innerState.SetLocation(innerPoint);
        outerState.SetLocation(outerPoint);
        middleState.SetLocation(middlePoint);

        Entity innerEntity = InitTestEntity(&innerState);
        Entity outerEntity = InitTestEntity(&outerState);
        Entity middleEntity = InitTestEntity(&middleState);

        AM_EXPECT_EQ(zone->GetFactor(innerEntity), 1.0f);
        AM_EXPECT_EQ(zone->GetFactor(outerEntity), 0.0f);
        AM_EXPECT_EQ(zone->GetFactor(middleEntity), 0.25f);
    }

    {
        ListenerInternalState innerState;
        ListenerInternalState outerState;
        ListenerInternalState middleState;

        innerState.SetLocation(innerPoint);
        outerState.SetLocation(outerPoint);
        middleState.SetLocation(middlePoint);

        Listener innerListener = InitTestListener(&innerState);
        Listener outerListener = InitTestListener(&outerState);
        Listener middleListener = InitTestListener(&middleState);

        AM_EXPECT_EQ(zone->GetFactor(innerListener), 1.0f);
        AM_EXPECT_EQ(zone->GetFactor(outerListener), 0.0f);
        AM_EXPECT_EQ(zone->GetFactor(middleListener), 0.25f);
    }
}
