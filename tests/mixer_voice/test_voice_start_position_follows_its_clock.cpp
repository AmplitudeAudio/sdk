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
    AM_TEST_CASE(DSPTestCase, mixer_voice, voice_start_position_follows_its_clock)
    {
    public:
        void Run() override
        {
            const AudioBuffer ramp = MakeRamp(48000, 1.0f / 65536.0f);
            VoiceSettings settings = MakeVoiceSettings(ramp, 48000, 256);
            settings.startPosition = 1000;
            settings.startPositionClock = 0; // frame 1000 would have been heard at clock 0
            settings.startFrame = 480;

            auto voice = std::make_unique<Voice>();
            AM_EXPECT(voice->Initialize(settings));
            const VoiceRun run = RunVoice(*voice, 1024, 256);

            // Started at clock 480, so it resumes where the virtual playback got to: source frame 1480.
            AM_EXPECT_EQ(ramp[0][1480], run.source[480]);
            AM_EXPECT_EQ(ramp[0][1481], run.source[481]);
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_start_position_follows_its_clock);
} // namespace SparkyStudios::Audio::Amplitude::Tests
