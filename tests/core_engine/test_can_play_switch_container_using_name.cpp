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
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, core_engine, can_play_switch_container_using_name)
    {
    public:
        void Run() override
        {
            Entity entity = amEngine->AddEntity(100);

            Channel channel = amEngine->Play("footsteps");
            AM_EXPECT_NOT(channel.Valid()); // switch container is entity scoped

            channel = amEngine->Play("footsteps", entity);
            amEngine->WaitUntilFrames(2); // Playing is done in the next frame

            AM_EXPECT(channel.Valid());
            AM_EXPECT(channel.Playing());

            // Clean up
            amEngine->RemoveEntity(100);
        }
    };

    AM_REGISTER_TEST(core_engine, can_play_switch_container_using_name);
} // namespace SparkyStudios::Audio::Amplitude::Tests
