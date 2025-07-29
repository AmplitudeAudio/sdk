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

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void EngineTestCase::Run()
    {
        const auto environment = amEngine->AddEnvironment(1234);

        // Set up zone for the environment
        auto inner = AmSharedPtr<SphereShape>::Make(10);
        auto outer = AmSharedPtr<SphereShape>::Make(20);
        auto zone = AmSharedPtr<SphereZone>::Make(inner, outer);
        environment.SetZone(zone);

        // Test setting effect by handle
        auto* effect = amEngine->GetEffectHandle("equalizer");
        environment.SetEffect(effect);

        // Verify it returns the new effect
        AM_EXPECT_EQ(environment.GetEffect(), effect);

        // Clean up
        amEngine->RemoveEnvironment(1234);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
