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
#include "VoiceSwapTestUtils.h"
#include "StreamTestDecoder.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        constexpr AmUInt64 kLength = 48000;
        constexpr AmUInt64 kBlock = 256;
        constexpr AmUInt64 kSeekFrame = 4 * kBlock;
        constexpr AmUInt64 kSeekPosition = 24000;
        constexpr AmUInt64 kCrossfade = 384; // 8 ms at 48 kHz.
    } // namespace

    AM_TEST_CASE(DSPTestCase, mixer_streaming, voice_seek_swaps_to_pending_stream)
    {
    public:
        void Run() override
        {
            auto decoderA = std::make_shared<StreamTestDecoder>(kLength, 1);
            AmUInt64 pre = 0;
            StreamSource a(decoderA, PlanStream(0, kLength, false, 0, 0, 0, pre), 4096, 1, 512, 48000, true);

            VoiceSettings settings;
            settings.source.stream = &a;
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
            voice.BindStream(&a);

            auto decoderB = std::make_shared<StreamTestDecoder>(kLength, 1);
            const AmUInt64 wanted = voice.GetPrerollFrames();
            StreamSource b(decoderB, PlanStream(0, kLength, false, 0, kSeekPosition, wanted, pre), 4096, 1, 512, 48000, true);

            AmUInt64 aReleasedAfterSwap = 0;
            const auto run = RunVoice(voice, 2048, kBlock, [&](AmUInt64 clock) {
                if (clock == kSeekFrame)
                {
                    VoiceCommand command = MakeCommand(eVoiceCommandKind::Seek, clock, 0.0, kSeekPosition);
                    command.stream = &b;
                    voice.Enqueue(command);
                }
                else if (clock == kSeekFrame + 2 * kBlock)
                {
                    aReleasedAfterSwap = a.GetRing().GetReleaseIndex();
                }
            });

            const VoiceEvent* swapped = FindEvent(run, eVoiceEventKind::StreamSwapped);
            AM_EXPECT(swapped != nullptr);
            if (swapped != nullptr)
                AM_EXPECT(swapped->stream == &b);

            // The replaced stream is reported dropped, exactly once; the adopted one never is.
            AM_EXPECT_EQ(static_cast<AmSize>(1), EventsOf(run, eVoiceEventKind::StreamDropped).size());
            const VoiceEvent* dropped = FindEvent(run, eVoiceEventKind::StreamDropped);
            if (dropped != nullptr)
                AM_EXPECT(dropped->stream == &a);

            AM_EXPECT(FindEvent(run, eVoiceEventKind::Underrun) == nullptr);

            for (AmUInt64 i = 0; i < kSeekFrame; ++i)
                AM_EXPECT_EQ(StreamTestDecoder::ValueAt(i, 0), run.source[i]);

            for (AmUInt64 i = kSeekFrame + kCrossfade; i < run.source.size(); ++i)
                AM_EXPECT_EQ(StreamTestDecoder::ValueAt(kSeekPosition + (i - kSeekFrame), 0), run.source[i]);

            AM_EXPECT(b.GetRing().GetReleaseIndex() >= pre + 500);
            AM_EXPECT(aReleasedAfterSwap > 0);
            AM_EXPECT_EQ(aReleasedAfterSwap, a.GetRing().GetReleaseIndex());
        }
    };

    AM_REGISTER_TEST(mixer_streaming, voice_seek_swaps_to_pending_stream);
} // namespace SparkyStudios::Audio::Amplitude::Tests
