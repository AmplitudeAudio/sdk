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
#include <Mixer/Amplimix.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, core_engine, mixer_rejects_invalid_playback_cursor_operations)
    {
    public:
        void Run() override
        {
            AmplimixImpl& mixer = amEngine->GetState()->mixer;
            AmUInt64 cursor = 0;

            AM_EXPECT_NOT(mixer.SetCursor(kAmInvalidObjectId, 0, 10));
            AM_EXPECT_NOT(mixer.GetCursor(kAmInvalidObjectId, 0, cursor));
            AM_EXPECT_NOT(mixer.ResetLayerState(kAmInvalidObjectId, 0));
        }
    };

    AM_REGISTER_TEST(core_engine, mixer_rejects_invalid_playback_cursor_operations);
} // namespace SparkyStudios::Audio::Amplitude::Tests
