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
    AM_TEST_CASE(ComponentTestCase, mixer_streaming, voice_event_mailbox_dropped_overflow_is_a_single_marker)
    {
    public:
        void Run() override
        {
            constexpr AmSize kPosted = VoiceEventMailbox::kMaxDroppedStreams + 6;

            // Identity only: the mailbox never dereferences these.
            std::vector<char> tokens(kPosted);

            VoiceEventMailbox mailbox;
            for (AmSize i = 0; i < kPosted; ++i)
            {
                VoiceEvent event;
                event.kind = eVoiceEventKind::StreamDropped;
                event.stream = reinterpret_cast<const StreamSource*>(&tokens[i]);
                mailbox.Post(event);
            }

            auto queue = std::make_unique<VoiceEventQueue>();
            mailbox.Publish(*queue);
            AM_EXPECT(mailbox.IsEmpty());

            AmSize valid = 0;
            AmSize markers = 0;
            VoiceEvent event;
            while (queue->TryDequeue(event))
            {
                AM_EXPECT(event.kind == eVoiceEventKind::StreamDropped);
                if (event.stream == nullptr)
                    ++markers;
                else
                    ++valid;
            }

            // The first kMaxDroppedStreams are published as they are; the lost ones collapse into one null marker. The
            // marker is not a drop of any stream: consumers keep everything alive until the voice ends.
            AM_EXPECT_EQ(VoiceEventMailbox::kMaxDroppedStreams, valid);
            AM_EXPECT_EQ(static_cast<AmSize>(1), markers);
        }
    };

    AM_REGISTER_TEST(mixer_streaming, voice_event_mailbox_dropped_overflow_is_a_single_marker);
} // namespace SparkyStudios::Audio::Amplitude::Tests
