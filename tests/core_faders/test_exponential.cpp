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

using namespace SparkyStudios::Audio::Amplitude;

void EngineTestCase::Run()
{
    std::shared_ptr<Fader> fader = Fader::Find("Exponential");
    constexpr BezierCurveControlPoints cp1 = { 1.0f, 0.0f, 1.0f, 1.0f };
    const BezierCurveControlPoints cp2 = fader->GetControlPoints();
    AM_EXPECT((cp1.x1 == cp2.x1 && cp1.y1 == cp2.y1 && cp1.x2 == cp2.x2 && cp1.y2 == cp2.y2));

    auto instance = Fader::Construct("Exponential");

    instance->Set(0.0, 1.0, kAmSecond);

    AM_EXPECT(instance->GetFromPercentage(0.00) == 0.0);
    AM_EXPECT(instance->GetFromPercentage(0.25) - 0.37813813779209771 < kEpsilon);
    AM_EXPECT(instance->GetFromPercentage(0.50) - 0.68464319530730855 < kEpsilon);
    AM_EXPECT(instance->GetFromPercentage(0.75) - 0.90653535347727843 < kEpsilon);
    AM_EXPECT(instance->GetFromPercentage(1.00) == 1.0);
}
