// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#include <Core/Engine.h>
#include <Core/EngineInternalState.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, core_engine, tag_registry_clears_with_engine_deinitialize)
    {
    public:
        void Run() override
        {
            const AmSize loaded = amEngine->GetState()->tag_registry.Size();
            AM_EXPECT(loaded >= 5);

            AM_EXPECT(amEngine->Deinitialize());
            AM_EXPECT(amEngine->Initialize(GetConfigFile()));

            AM_EXPECT_EQ(static_cast<AmSize>(0), amEngine->GetState()->tag_registry.Size());

            amEngine->EnsureSoundBankLoaded(AM_OS_STRING("tests.init.ambank"));
            amEngine->StartLoadSoundFiles();
            while (!amEngine->TryFinalizeLoadSoundFiles())
                Thread::Sleep(1);

            AM_EXPECT_EQ(loaded, amEngine->GetState()->tag_registry.Size());
        }
    };

    AM_REGISTER_TEST(core_engine, tag_registry_clears_with_engine_deinitialize);
} // namespace SparkyStudios::Audio::Amplitude::Tests
