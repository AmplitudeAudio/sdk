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

#include <memory>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/Voice/VoiceTypes.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, mixer_voice, voice_event_mailbox_publishes_in_order)
    {
    public:
        void Run() override
        {
            auto queue = std::make_unique<VoiceEventQueue>();
            VoiceEventMailbox mailbox;

            mailbox.Post({ 1, 1, eVoiceEventKind::Finished, eVoiceFadeTarget::None, 900, 0, 1 });
            mailbox.Post({ 1, 1, eVoiceEventKind::FadedOut, eVoiceFadeTarget::Paused, 800, 0, 1 });
            mailbox.Post({ 1, 1, eVoiceEventKind::Started, eVoiceFadeTarget::None, 100, 0, 1 });
            mailbox.Amend(eVoiceEventKind::FadedOut, 12345);
            mailbox.Publish(*queue);

            std::vector<VoiceEvent> events;
            VoiceEvent event;
            while (queue->TryDequeue(event))
                events.push_back(event);

            AM_EXPECT_EQ(3ULL, events.size());
            AM_EXPECT(events[0].kind == eVoiceEventKind::Started);
            AM_EXPECT(events[1].kind == eVoiceEventKind::FadedOut);
            AM_EXPECT(events[1].target == eVoiceFadeTarget::Paused);
            AM_EXPECT_EQ(12345ULL, events[1].sourcePosition);
            AM_EXPECT(events[2].kind == eVoiceEventKind::Finished);
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_event_mailbox_publishes_in_order);
} // namespace SparkyStudios::Audio::Amplitude::Tests
