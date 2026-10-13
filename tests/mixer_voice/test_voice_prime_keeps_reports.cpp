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
    AM_TEST_CASE(DSPTestCase, mixer_voice, voice_prime_keeps_reports)
    {
    public:
        void Run() override
        {
            // A sound shorter than the filter's group delay reaches its end while the start is being primed. The end must
            // still be reported, or the channel settles without an End.
            for (AmUInt64 length = 1; length <= 24; ++length)
            {
                const AudioBuffer tiny = MakeRamp(length, 0.01f);
                auto voice = std::make_unique<Voice>();
                AM_EXPECT(voice->Initialize(MakeVoiceSettings(tiny, 44100, 256)));
                const VoiceRun run = RunVoice(*voice, 1024, 256);

                AM_EXPECT(FindEvent(run, eVoiceEventKind::Ended) != nullptr);
                AM_EXPECT(FindEvent(run, eVoiceEventKind::Finished) != nullptr);
            }
        }
    };

    AM_TEST_CASE(DSPTestCase, mixer_voice, voice_resume_after_scheduled_pause_primes)
    {
    public:
        void Run() override
        {
            for (const AmUInt32 rate : { 44100U, 96000U })
            {
                AudioBuffer impulse(rate, 1);
                impulse[0][0] = 1.0f;

                VoiceSettings settings = MakeVoiceSettings(impulse, rate, 256);
                settings.startFrame = 2000;
                auto voice = std::make_unique<Voice>();
                AM_EXPECT(voice->Initialize(settings));

                const VoiceRun run = RunVoice(
                    *voice, 2048, 256,
                    [&](AmUInt64 clock)
                    {
                        if (clock == 0)
                        {
                            voice->Enqueue(MakeCommand(eVoiceCommandKind::Pause, 0, 0.0));
                            voice->Enqueue(MakeCommand(eVoiceCommandKind::Resume, 512, 0.0));
                        }
                    });

                AmUInt64 peak = 0;
                for (AmUInt64 i = 1; i < run.source.size(); ++i)
                    if (std::abs(run.source[i]) > std::abs(run.source[peak]))
                        peak = i;

                // Source frame 0 is heard at the resume frame, give or take the filter's sub-sample centre.
                AM_EXPECT(peak >= 511 && peak <= 513);
            }
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_prime_keeps_reports);
    AM_REGISTER_TEST(mixer_voice, voice_resume_after_scheduled_pause_primes);
} // namespace SparkyStudios::Audio::Amplitude::Tests
