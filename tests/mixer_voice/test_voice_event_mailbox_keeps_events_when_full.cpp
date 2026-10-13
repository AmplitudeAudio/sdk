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
    AM_TEST_CASE(ComponentTestCase, mixer_voice, voice_event_mailbox_keeps_events_when_full)
    {
    public:
        void Run() override
        {
            auto queue = std::make_unique<VoiceEventQueue>();
            for (AmSize i = 0; i < VoiceEventQueue::GetCapacity(); ++i)
                AM_EXPECT(queue->TryEnqueue(VoiceEvent{}));

            VoiceEventMailbox mailbox;
            mailbox.Post({ 1, 1, eVoiceEventKind::Started, eVoiceFadeTarget::None, 5, 0, 1 });
            mailbox.Post({ 1, 1, eVoiceEventKind::Finished, eVoiceFadeTarget::None, 6, 0, 1 });
            mailbox.Publish(*queue);

            // Nothing fitted: nothing is lost.
            AM_EXPECT_NOT(mailbox.IsEmpty());

            VoiceEvent event;
            AM_EXPECT(queue->TryDequeue(event));
            mailbox.Publish(*queue);

            // One slot freed: Started goes first and Finished waits, so order is kept.
            AM_EXPECT_NOT(mailbox.IsEmpty());

            while (queue->TryDequeue(event))
                ;
            mailbox.Publish(*queue);
            AM_EXPECT(mailbox.IsEmpty());
            AM_EXPECT(queue->TryDequeue(event));
            AM_EXPECT(event.kind == eVoiceEventKind::Finished);
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_event_mailbox_keeps_events_when_full);
} // namespace SparkyStudios::Audio::Amplitude::Tests
