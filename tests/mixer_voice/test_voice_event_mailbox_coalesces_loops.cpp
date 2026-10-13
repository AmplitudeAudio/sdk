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
    AM_TEST_CASE(ComponentTestCase, mixer_voice, voice_event_mailbox_coalesces_loops)
    {
    public:
        void Run() override
        {
            auto queue = std::make_unique<VoiceEventQueue>();
            VoiceEventMailbox mailbox;

            // A 10-frame loop inside a 4096-frame block wraps hundreds of times: one event carries them all.
            for (AmUInt64 i = 0; i < 400; ++i)
                mailbox.Post({ 3, 7, eVoiceEventKind::Looped, eVoiceFadeTarget::None, 10 + i * 10, 0, 1 });

            mailbox.Publish(*queue);
            AM_EXPECT(mailbox.IsEmpty());

            VoiceEvent event;
            AM_EXPECT(queue->TryDequeue(event));
            AM_EXPECT(event.kind == eVoiceEventKind::Looped);
            AM_EXPECT_EQ(400U, event.count);
            AM_EXPECT_EQ(10ULL, event.frame); // the first wrap
            AM_EXPECT_EQ(3U, event.layer);
            AM_EXPECT_EQ(7U, event.id);
            AM_EXPECT_NOT(queue->TryDequeue(event));
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_event_mailbox_coalesces_loops);
} // namespace SparkyStudios::Audio::Amplitude::Tests
