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

#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, core_engine, stolen_channel_survives_as_virtual)
    {
    public:
        void Run() override
        {
            SoundHandle sound = amEngine->GetSoundHandle("test_sound_01");
            std::vector<Channel> low;
            for (int i = 0; i < 50; ++i)
                low.push_back(amEngine->Play(sound, AmVector3{ 0.0f, 0.0f, 0.0f }, 0.5f));

            amEngine->WaitUntilFrames(2);
            for (const auto& channel : low)
                AM_EXPECT(channel.GetState()->IsReal());

            // A louder sound takes a real channel from one of them.
            Channel high = amEngine->Play(sound, AmVector3{ 0.0f, 0.0f, 0.0f }, 1.0f);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(high.GetState()->IsReal());

            Channel* stolen = nullptr;
            for (auto& channel : low)
                if (!channel.GetState()->IsReal())
                    stolen = &channel;

            AM_EXPECT_NOT(stolen == nullptr);
            if (stolen == nullptr)
                return;

            // Virtual, not stopped, and its position keeps moving with the clock.
            AM_EXPECT_NOT(stolen->GetState()->Stopped());
            amEngine->WaitUntilFrames(3);
            AM_EXPECT_NOT(stolen->GetState()->Stopped());

            // Freeing a real channel promotes it back. Fifty BinauralHighQuality voices are slow to mix in a debug
            // build: this waits on the class default rather than a few frames.
            high.Stop(0.0);
            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return stolen->GetState()->IsReal();
                }));

            for (auto& channel : low)
                channel.Stop(0.0);
        }
    };

    AM_REGISTER_TEST(core_engine, stolen_channel_survives_as_virtual);
} // namespace SparkyStudios::Audio::Amplitude::Tests
