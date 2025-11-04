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
        // Test adding environments
        Environment e1 = amEngine->AddEnvironment(1);
        Environment e2 = amEngine->AddEnvironment(2);

        AM_EXPECT(e1.Valid());
        AM_EXPECT(e2.Valid());

        // Test adding environment with existing ID should return same environment
        Environment e3 = amEngine->AddEnvironment(1);
        AM_EXPECT(e3.Valid());
        AM_EXPECT_EQ(e3.GetState(), e1.GetState());

        // Test getting existing environment
        Environment e4 = amEngine->GetEnvironment(2);
        AM_EXPECT(e4.Valid());
        AM_EXPECT_EQ(e4.GetState(), e2.GetState());

        // Test getting non-existing environment
        Environment e5 = amEngine->GetEnvironment(3);
        AM_EXPECT_NOT(e5.Valid());

        // Test removing environments
        amEngine->RemoveEnvironment(1);
        amEngine->RemoveEnvironment(&e2);
        amEngine->RemoveEnvironment(3); // Non-existing environment should not cause issues

        AM_EXPECT_NOT(e1.Valid());
        AM_EXPECT_NOT(e2.Valid());
        AM_EXPECT_NOT(e3.Valid());
        AM_EXPECT_NOT(e4.Valid());
        AM_EXPECT_NOT(e5.Valid());

        // Test getting environment with invalid ID
        Environment e6 = amEngine->GetEnvironment(kAmInvalidObjectId);
        AM_EXPECT_NOT(e6.Valid());
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
