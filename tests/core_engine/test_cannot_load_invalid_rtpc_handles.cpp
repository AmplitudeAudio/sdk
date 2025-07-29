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
        // Test loading invalid RTPC handles
        RtpcHandle invalidRtpc1 = amEngine->GetRtpcHandle("invalid_rtpc");
        AM_EXPECT(invalidRtpc1 == nullptr);

        RtpcHandle invalidRtpc2 = amEngine->GetRtpcHandle(99999);
        AM_EXPECT(invalidRtpc2 == nullptr);

        // Test setting RTPC values with invalid RTPC IDs (should not crash)
        amEngine->SetRtpcValue(99999, 1);

        // Test setting RTPC values with invalid RTPC names (should not crash)
        amEngine->SetRtpcValue("invalid_rtpc", 1);

        // Test setting RTPC values with invalid RTPC handles (should not crash)
        amEngine->SetRtpcValue(invalidRtpc1, 1);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
