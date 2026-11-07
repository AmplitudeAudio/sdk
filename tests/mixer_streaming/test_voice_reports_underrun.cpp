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
    } // namespace

    AM_TEST_CASE(DSPTestCase, mixer_streaming, voice_reports_underrun)
    {
    public:
        void Run() override
        {
            AmUInt64 pre = 0;
            auto decoder = std::make_shared<StreamTestDecoder>(kLength, 1);
            StreamSource stream(decoder, PlanStream(0, kLength, false, 0, 0, 0, pre), 4096, 1, 512, 48000, false);
            stream.Fill(300);

            VoiceSettings settings;
            settings.source.stream = &stream;
            settings.source.length = kLength;
            settings.source.channels = 1;
            settings.source.sampleRate = 48000;
            settings.outputRate = 48000;
            settings.maxBlockFrames = kBlock;
            settings.resamplerName = "linear";
            settings.layer = 1;
            settings.id = 1;

            Voice voice;
            AM_EXPECT(voice.Initialize(settings));

            // Two blocks over 300 frames: the second runs short.
            auto first = RunVoice(voice, 2 * kBlock, kBlock);
            AM_EXPECT(FindEvent(first, eVoiceEventKind::Underrun) != nullptr);
            AM_EXPECT(FindEvent(first, eVoiceEventKind::Ended) == nullptr);
            AM_EXPECT(voice.GetPublishedState() == eVoiceState::Playing);

            // Once the ring is refilled the stream resumes where it held, without gaps in the source sequence.
            stream.Fill(2000);
            auto second = RunVoice(voice, 4 * kBlock, kBlock, [](AmUInt64) {}, 2 * kBlock);
            AM_EXPECT(FindEvent(second, eVoiceEventKind::Underrun) == nullptr);
            AM_EXPECT(FindEvent(second, eVoiceEventKind::Ended) == nullptr);
            AM_EXPECT(voice.GetPublishedState() == eVoiceState::Playing);

            std::vector<AmReal32> audible;
            for (const auto* run : { &first, &second })
                for (const AmReal32 sample : run->source)
                    if (sample != 0.0f)
                        audible.push_back(sample);

            AM_EXPECT(audible.size() > 300);
            for (AmUInt64 i = 0; i < audible.size(); ++i)
                AM_EXPECT_EQ(StreamTestDecoder::ValueAt(i, 0), audible[i]);
        }
    };

    AM_REGISTER_TEST(mixer_streaming, voice_reports_underrun);
} // namespace SparkyStudios::Audio::Amplitude::Tests
