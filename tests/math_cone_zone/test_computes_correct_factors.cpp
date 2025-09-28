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

#include <Core/EntityInternalState.h>
#include <Core/ListenerInternalState.h>

#include "SimpleTestCase.h"

using namespace SparkyStudios::Audio::Amplitude;

void SimpleTestCase::Run()
{
    auto inner = AmSharedPtr<ConeShape>::Make(50, 50);
    auto outer = AmSharedPtr<ConeShape>::Make(100, 100);
    std::shared_ptr<Zone> zone = AmSharedPtr<ConeZone>::Make(inner, outer);

    constexpr AmVector3 innerPoint = { 25, 25, 0 };
    constexpr AmVector3 outerPoint = { 175, 175, 0 };
    constexpr AmVector3 middlePoint1 = { 0.0f, 87.5f, 0.0f };
    constexpr AmVector3 middlePoint2 = { 25.0f, 75.0f, 0.0f };

    AM_EXPECT_EQ(zone->GetFactor(innerPoint), 1.0f);
    AM_EXPECT_EQ(zone->GetFactor(outerPoint), 0.0f);
    AM_EXPECT_EQ(zone->GetFactor(middlePoint1), 0.25f);
    AM_EXPECT_EQ(zone->GetFactor(middlePoint2), 0.5f);

    {
        EntityInternalState innerState;
        EntityInternalState outerState;
        EntityInternalState middleState1;
        EntityInternalState middleState2;

        innerState.SetLocation(innerPoint);
        outerState.SetLocation(outerPoint);
        middleState1.SetLocation(middlePoint1);
        middleState2.SetLocation(middlePoint2);

        Entity innerEntity = InitTestEntity(&innerState);
        Entity outerEntity = InitTestEntity(&outerState);
        Entity middleEntity1 = InitTestEntity(&middleState1);
        Entity middleEntity2 = InitTestEntity(&middleState2);

        AM_EXPECT_EQ(zone->GetFactor(innerEntity), 1.0f);
        AM_EXPECT_EQ(zone->GetFactor(outerEntity), 0.0f);
        AM_EXPECT_EQ(zone->GetFactor(middleEntity1), 0.25f);
        AM_EXPECT_EQ(zone->GetFactor(middleEntity2), 0.5f);
    }

    {
        ListenerInternalState innerState;
        ListenerInternalState outerState;
        ListenerInternalState middleState1;
        ListenerInternalState middleState2;

        innerState.SetLocation(innerPoint);
        outerState.SetLocation(outerPoint);
        middleState1.SetLocation(middlePoint1);
        middleState2.SetLocation(middlePoint2);

        Listener innerListener = InitTestListener(&innerState);
        Listener outerListener = InitTestListener(&outerState);
        Listener middleListener1 = InitTestListener(&middleState1);
        Listener middleListener2 = InitTestListener(&middleState2);

        AM_EXPECT_EQ(zone->GetFactor(innerListener), 1.0f);
        AM_EXPECT_EQ(zone->GetFactor(outerListener), 0.0f);
        AM_EXPECT_EQ(zone->GetFactor(middleListener1), 0.25f);
        AM_EXPECT_EQ(zone->GetFactor(middleListener2), 0.5f);
    }
}
