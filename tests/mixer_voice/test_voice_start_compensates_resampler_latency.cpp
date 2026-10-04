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
    AM_TEST_CASE(DSPTestCase, mixer_voice, voice_start_compensates_resampler_latency)
    {
    public:
        void Run() override
        {
            for (const AmUInt32 rate : { 44100U, 96000U, 22050U })
            {
                AudioBuffer impulse(rate, 1);
                impulse[0][0] = 1.0f;

                VoiceSettings settings = MakeVoiceSettings(impulse, rate, 256);
                settings.startFrame = 1000;
                auto voice = std::make_unique<Voice>();
                AM_EXPECT(voice->Initialize(settings));
                const VoiceRun run = RunVoice(*voice, 2048, 256);

                AmUInt64 peak = 0;
                for (AmUInt64 i = 1; i < run.source.size(); ++i)
                    if (std::abs(run.source[i]) > std::abs(run.source[peak]))
                        peak = i;

                // Source frame 0 is heard at clock 1000, give or take the filter's sub-sample centre.
                AM_EXPECT(peak >= 999 && peak <= 1001);
            }
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_start_compensates_resampler_latency);
} // namespace SparkyStudios::Audio::Amplitude::Tests
