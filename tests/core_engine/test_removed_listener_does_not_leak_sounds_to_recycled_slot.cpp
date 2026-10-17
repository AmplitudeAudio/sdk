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

#include <Core/ListenerInternalState.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // Listener states are pooled. Removing a listener that a channel is still spatialized against must detach it
    // right away, or the free list can hand that very state to the next AddListener() call while it still "owns"
    // a voice that belongs to nobody.
    AM_TEST_CASE(EngineTestCase, core_engine, removed_listener_does_not_leak_sounds_to_recycled_slot)
    {
    public:
        void Run() override
        {
            // Only one listener at a time, so the "Nearest" fetch mode has no ambiguity about which one a played
            // channel attaches to.
            amEngine->RemoveListener(1);

            Listener listener = amEngine->AddListener(50);
            AM_EXPECT(listener.Valid());

            Channel channel = amEngine->Play(9992);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.Valid());

            // Paused: keeps the voice (and its attachment to the listener) alive for the rest of the test.
            channel.Pause();

            ListenerInternalState* state = listener.GetState();
            AM_EXPECT_EQ(state->GetPlayingSoundList().size(), static_cast<AmSize>(1));

            amEngine->RemoveListener(50);

            // The just-removed state must not still list the channel: nothing stops the free list from handing
            // this very slot to the next AddListener() call.
            AM_EXPECT_EQ(state->GetPlayingSoundList().size(), static_cast<AmSize>(0));

            channel.Stop(0.0);
        }
    };

    AM_REGISTER_TEST(core_engine, removed_listener_does_not_leak_sounds_to_recycled_slot);
} // namespace SparkyStudios::Audio::Amplitude::Tests
