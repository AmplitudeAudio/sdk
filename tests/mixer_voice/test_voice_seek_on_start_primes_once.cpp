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
    AM_TEST_CASE(DSPTestCase, mixer_voice, voice_seek_on_start_primes_once)
    {
    public:
        void Run() override
        {
            // A seek landing in the block that starts the voice must not prime the new stream a second time, which would
            // skip about GetLatency() source frames right after the crossfade.
            const AudioBuffer ramp = MakeRamp(44100 * 2, 1.0f / 65536.0f);

            auto seeked = std::make_unique<Voice>();
            AM_EXPECT(seeked->Initialize(MakeVoiceSettings(ramp, 44100, 256)));
            const VoiceRun a = RunVoice(
                *seeked, 4096, 256,
                [&](AmUInt64 clock)
                {
                    if (clock == 0)
                        seeked->Enqueue(MakeCommand(eVoiceCommandKind::Seek, 0, 0.0, 10000));
                });

            VoiceSettings reference = MakeVoiceSettings(ramp, 44100, 256);
            reference.startPosition = 10000;
            auto direct = std::make_unique<Voice>();
            AM_EXPECT(direct->Initialize(reference));
            const VoiceRun b = RunVoice(*direct, 4096, 256);

            AmReal32 worst = 0.0f;
            for (AmUInt64 i = 600; i < 4000; ++i)
                worst = std::max(worst, std::abs(a.source[i] - b.source[i]));

            // One source frame is 1/65536 = 1.5e-5.
            AM_EXPECT(worst < 1.0e-6f);
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_seek_on_start_primes_once);
} // namespace SparkyStudios::Audio::Amplitude::Tests
