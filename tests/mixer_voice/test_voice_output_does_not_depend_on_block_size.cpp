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
    AM_TEST_CASE(DSPTestCase, mixer_voice, voice_output_does_not_depend_on_block_size)
    {
    public:
        void Run() override
        {
            // 441 frames at 44.1 kHz is one 100 Hz period, resampled to 48 kHz and looped.
            AudioBuffer period(441, 1);
            for (AmUInt64 i = 0; i < 441; ++i)
                period[0][i] = static_cast<AmReal32>(0.5 * std::sin(2.0 * 3.14159265358979323846 * static_cast<AmReal64>(i) / 441.0));

            auto reference = std::make_unique<Voice>();
            AM_EXPECT(reference->Initialize(MakeVoiceSettings(period, 44100, 4096, true)));
            const VoiceRun expected = RunVoice(*reference, 16384, 4096);

            for (const AmUInt64 block : { 7ULL, 256ULL, 1023ULL, 1024ULL })
            {
                auto voice = std::make_unique<Voice>();
                AM_EXPECT(voice->Initialize(MakeVoiceSettings(period, 44100, block, true)));
                const VoiceRun actual = RunVoice(*voice, 16384, block);

                AmReal32 worst = 0.0f;
                for (AmUInt64 n = 0; n < 16384; ++n)
                    worst = std::max(worst, std::abs(actual.source[n] - expected.source[n]));
                AM_EXPECT(worst < 1e-6f);
            }
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_output_does_not_depend_on_block_size);
} // namespace SparkyStudios::Audio::Amplitude::Tests
