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
    auto inner = std::make_shared<ConeShape>(50, 50);
    auto outer = std::make_shared<ConeShape>(100, 100);
    auto zone = std::make_shared<ConeZone>(inner, outer);

    const AmVector3 innerPoint = { 25, 25, 0 };
    const AmVector3 outerPoint = { 175, 175, 0 };
    const AmVector3 middlePoint1 = { 0.0f, 87.5f, 0.0f };
    const AmVector3 middlePoint2 = { 25.0f, 75.0f, 0.0f };

    ExpectEqual(zone->GetFactor(innerPoint), 1.0f);
    ExpectEqual(zone->GetFactor(outerPoint), 0.0f);
    ExpectEqual(zone->GetFactor(middlePoint1), 0.25f);
    ExpectEqual(zone->GetFactor(middlePoint2), 0.5f);
}
