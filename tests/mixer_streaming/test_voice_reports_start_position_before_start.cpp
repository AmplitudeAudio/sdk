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

#include <Mixer/Streaming/StreamSource.h>
#include <Mixer/Voice/Voice.h>

#include "../mixer_voice/VoiceTestUtils.h"
#include "DSPTestCase.h"
#include "StreamTestDecoder.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        constexpr AmUInt64 kLength = 48000;
        constexpr AmUInt64 kBlock = 256;
        constexpr AmUInt64 kStart = 20000;
        constexpr AmUInt64 kStartFrame = 4 * kBlock;
    } // namespace

    AM_TEST_CASE(DSPTestCase, mixer_streaming, voice_reports_start_position_before_start)
    {
    public:
        void Run() override
        {
            AmUInt64 pre = 0;
            auto placeholderDecoder = std::make_shared<StreamTestDecoder>(kLength, 1);
            StreamSource placeholder(placeholderDecoder, PlanStream(0, kLength, false, 0, 0, 0, pre), 4096, 1, 512, 48000, false);

            VoiceSettings settings;
            settings.source.stream = &placeholder;
            settings.source.length = kLength;
            settings.source.channels = 1;
            settings.source.sampleRate = 48000;
            settings.outputRate = 48000;
            settings.maxBlockFrames = kBlock;
            settings.resamplerName = "linear";
            settings.startPosition = kStart;
            settings.startFrame = kStartFrame;
            settings.layer = 1;
            settings.id = 1;

            // The ring is still empty, but the voice already reports where it will start: a demotion in this window
            // must anchor its virtual cursor there, not at the top of the sound.
            Voice voice;
            AM_EXPECT(voice.Initialize(settings));
            AM_EXPECT_EQ(kStart, voice.GetPublishedPosition());

            auto decoder = std::make_shared<StreamTestDecoder>(kLength, 1);
            StreamSource stream(
                decoder, PlanStream(0, kLength, false, 0, kStart, voice.GetPrerollFrames(), pre), 4096, 1, 512, 48000, false);
            voice.BindStream(&stream);
            AM_EXPECT_EQ(kStart, voice.GetPublishedPosition());

            stream.Fill(4000);

            // Blocks rendered before the start frame keep reporting the start position.
            RunVoice(voice, kStartFrame, kBlock);
            AM_EXPECT_EQ(kStart, voice.GetPublishedPosition());

            // Once started, the position follows the reader from the start position.
            RunVoice(
                voice, kBlock, kBlock,
                [](AmUInt64)
                {
                },
                kStartFrame);
            AM_EXPECT_EQ(kStart + kBlock, voice.GetPublishedPosition());

            // A voice resuming a virtual cursor reports the cursor while it waits: it is where the voice would start.
            settings.startPositionClock = 0;
            Voice resumed;
            AM_EXPECT(resumed.Initialize(settings));
            AM_EXPECT_EQ(kStart, resumed.GetPublishedPosition());

            auto resumedDecoder = std::make_shared<StreamTestDecoder>(kLength, 1);
            StreamSource resumedStream(
                resumedDecoder, PlanStream(0, kLength, false, 0, kStart, resumed.GetPrerollFrames(), pre), 8192, 1, 512, 48000, false);
            resumed.BindStream(&resumedStream);
            resumedStream.Fill(6000);

            RunVoice(resumed, 2 * kBlock, kBlock);
            AM_EXPECT_EQ(kStart + 2 * kBlock, resumed.GetPublishedPosition());

            RunVoice(
                resumed, kBlock, kBlock,
                [](AmUInt64)
                {
                },
                kStartFrame);
            AM_EXPECT_EQ(kStart + kStartFrame + kBlock, resumed.GetPublishedPosition());
        }
    };

    AM_REGISTER_TEST(mixer_streaming, voice_reports_start_position_before_start);
} // namespace SparkyStudios::Audio::Amplitude::Tests
