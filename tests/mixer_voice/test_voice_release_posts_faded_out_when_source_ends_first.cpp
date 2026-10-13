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

#include <cmath>
#include <memory>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/Voice/Voice.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"
#include "VoiceTestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, mixer_voice, voice_release_posts_faded_out_when_source_ends_first)
    {
    public:
        void Run() override
        {
            // A short, non-looping source: it ends well before the release fade (10 ms = 480 frames) would.
            const AudioBuffer ramp = MakeRamp(1000, 1.0f / 1024.0f);
            auto voice = std::make_unique<Voice>();
            AM_EXPECT(voice->Initialize(MakeVoiceSettings(ramp, 48000, 256)));

            const VoiceRun run = RunVoice(
                *voice, 2048, 256,
                [&](AmUInt64 clock)
                {
                    if (clock == 256)
                        voice->Enqueue(MakeCommand(eVoiceCommandKind::Release, 900, 10.0));
                });

            // The source runs out (at frame 1000) long before the release fade would finish on its own (at 1380):
            // FadedOut{Released} must still be posted, anchoring a virtual cursor at the source's end, before Finished.
            const VoiceEvent* fadedOut = FindEvent(run, eVoiceEventKind::FadedOut);
            AM_EXPECT_NOT(fadedOut == nullptr);
            AM_EXPECT(fadedOut->target == eVoiceFadeTarget::Released);
            AM_EXPECT_EQ(1000ULL, fadedOut->sourcePosition);

            const VoiceEvent* finished = FindEvent(run, eVoiceEventKind::Finished);
            AM_EXPECT_NOT(finished == nullptr);
            AM_EXPECT(voice->GetPublishedState() == eVoiceState::Finished);
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_release_posts_faded_out_when_source_ends_first);
} // namespace SparkyStudios::Audio::Amplitude::Tests
