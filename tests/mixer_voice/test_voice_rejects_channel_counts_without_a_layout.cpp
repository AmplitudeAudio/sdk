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

#include <Mixer/Voice/Voice.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"
#include "VoiceTestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, mixer_voice, voice_rejects_channel_counts_without_a_layout)
    {
    public:
        void Run() override
        {
            for (const AmUInt16 channels : { 1, 2, 4, 6, 8 })
            {
                AudioBuffer source(256, channels);
                Voice voice;
                AM_EXPECT(voice.Initialize(MakeVoiceSettings(source, 48000, 64)));
                AM_EXPECT_EQ(channels, voice.GetChannelCount());
                AM_EXPECT_EQ(channels, voice.GetChannelLayout().GetChannelCount());
            }

            for (const AmUInt16 channels : { 3, 5, 7, 10 })
            {
                AudioBuffer source(256, channels);
                Voice voice;
                AM_EXPECT_NOT(voice.Initialize(MakeVoiceSettings(source, 48000, 64)));
            }
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_rejects_channel_counts_without_a_layout);
} // namespace SparkyStudios::Audio::Amplitude::Tests
