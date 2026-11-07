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

    // A slot initialized after the real stream was bound reads that stream, never the stand-in the voice was created
    // over (which dies with its creator): separate-mode instances are initialized this way.
    AM_TEST_CASE(DSPTestCase, mixer_streaming, voice_slot_binds_real_stream_not_stand_in)
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
            settings.layer = 1;
            settings.id = 1;

            Voice voice;
            AM_EXPECT(voice.Initialize(settings));

            auto decoder = std::make_shared<StreamTestDecoder>(kLength, 1);
            StreamSource stream(decoder, PlanStream(0, kLength, false, 0, 0, voice.GetPrerollFrames(), pre), 4096, 1, 512, 48000, false);
            voice.BindStream(&stream);

            VoiceStreamSlot slot;
            AM_EXPECT(voice.InitializeSlot(slot, 0));
            AM_EXPECT(slot.reader.GetStream() == &stream);
            AM_EXPECT(slot.reader.GetStream() != &placeholder);
            AM_EXPECT(voice.GetCurrentStream() == &stream);
        }
    };

    AM_REGISTER_TEST(mixer_streaming, voice_slot_binds_real_stream_not_stand_in);
} // namespace SparkyStudios::Audio::Amplitude::Tests
