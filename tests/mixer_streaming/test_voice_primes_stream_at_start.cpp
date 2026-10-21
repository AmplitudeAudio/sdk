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
        constexpr AmUInt64 kStart = 1000;
    } // namespace

    AM_TEST_CASE(DSPTestCase, mixer_streaming, voice_primes_stream_at_start)
    {
    public:
        void Run() override
        {
            // The ring the voice is created over is empty: it only stands in until the planned stream is bound.
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
            settings.layer = 1;
            settings.id = 1;

            Voice voice;
            AM_EXPECT(voice.Initialize(settings));
            AM_EXPECT_EQ(0ULL, placeholder.GetRing().GetReleaseIndex());
            AM_EXPECT_EQ(0ULL, placeholder.GetBufferedFrames());

            auto decoder = std::make_shared<StreamTestDecoder>(kLength, 1);
            StreamSource stream(decoder, PlanStream(0, kLength, false, 0, kStart, voice.GetPrerollFrames(), pre), 4096, 1, 512, 48000, false);
            voice.BindStream(&stream);
            AM_EXPECT_EQ(0ULL, stream.GetRing().GetReleaseIndex());

            stream.Fill(2000);

            const auto run = RunVoice(voice, kBlock, kBlock);
            AM_EXPECT(FindEvent(run, eVoiceEventKind::Underrun) == nullptr);
            for (AmUInt64 i = 0; i < kBlock; ++i)
                AM_EXPECT_EQ(StreamTestDecoder::ValueAt(kStart + i, 0), run.source[i]);

            AM_EXPECT_EQ(0ULL, placeholder.GetRing().GetReleaseIndex());
        }
    };

    AM_REGISTER_TEST(mixer_streaming, voice_primes_stream_at_start);
} // namespace SparkyStudios::Audio::Amplitude::Tests
