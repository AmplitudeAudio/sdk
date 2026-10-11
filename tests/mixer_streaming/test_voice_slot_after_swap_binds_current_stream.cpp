// Copyright (c) 2020-present Sparky Studios. All rights reserved.
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

#include "../mixer_voice/VoiceTestUtils.h"
#include "DSPTestCase.h"
#include "VoiceSwapTestUtils.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        constexpr AmUInt64 kLength = 48000;
        constexpr AmUInt64 kSeekFrame = 4 * kSwapBlock;
        constexpr AmUInt64 kSeekPosition = 24000;
    } // namespace

    // Voice::BindStream records the first stream in the settings, and swaps never update them: a slot initialized after
    // a swap must bind the voice's current stream, because the first one is retired and freed by then.
    AM_TEST_CASE(DSPTestCase, mixer_streaming, voice_slot_after_swap_binds_current_stream)
    {
    public:
        void Run() override
        {
            auto first = MakeSwapStream(kLength, 0, 0);

            Voice voice;
            AM_EXPECT(voice.Initialize(MakeSwapSettings(*first, kLength)));
            voice.BindStream(first.get());

            auto second = MakeSwapStream(kLength, kSeekPosition, voice.GetPrerollFrames());
            StreamSource* secondPtr = second.get();

            const auto run = RunVoice(voice, 2048, kSwapBlock, [&](AmUInt64 clock) {
                if (clock == kSeekFrame)
                    voice.Enqueue(MakeStreamSeek(clock, kSeekPosition, secondPtr));
            });

            AM_EXPECT(FindEvent(run, eVoiceEventKind::StreamSwapped) != nullptr);
            AM_EXPECT(voice.GetCurrentStream() == secondPtr);

            // The voice dropped the first stream: it is gone, as it is once the game side retires it.
            first.reset();

            VoiceStreamSlot slot;
            AM_EXPECT(voice.InitializeSlot(slot, 0));
            AM_EXPECT(slot.reader.GetStream() == secondPtr);
        }
    };

    AM_REGISTER_TEST(mixer_streaming, voice_slot_after_swap_binds_current_stream);
} // namespace SparkyStudios::Audio::Amplitude::Tests
