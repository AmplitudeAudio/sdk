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
        // Test setting master gain to 0.1f
        amEngine->SetMasterGain(0.1f);
        AM_EXPECT(amEngine->GetMasterGain() == 0.1f);

        // Test setting master gain back to 1.0f
        amEngine->SetMasterGain(1.0f);
        AM_EXPECT(amEngine->GetMasterGain() == 1.0f);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
