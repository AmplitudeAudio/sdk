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

#include <algorithm>
#include <cmath>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/Nodes/AttenuationNode.h>

#include "NodeTestCase.h"
#include "TestRegistry.h"
#include "TestUtils.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        constexpr AmUInt64 kFrames = 1024;
        constexpr AmUInt32 kSampleRate = 48000;

        // A 12 kHz tone sits above the 8 kHz high-shelf corner, so a -12 dB high band changes it clearly.
        AudioBuffer MakeTone()
        {
            AudioBuffer tone(kFrames, 1);
            for (AmUInt64 i = 0; i < kFrames; ++i)
                tone[0][i] = 0.5f * std::sin(2.0f * AM_PI32 * 12000.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(kSampleRate));
            return tone;
        }

        // Runs `blocks` blocks of `input` and returns a copy of the last output block.
        AudioBuffer RunBlocks(AirAbsorptionEQFilter& eq, const AudioBuffer& input, AmUInt32 blocks)
        {
            AudioBuffer output(kFrames, 1);
            for (AmUInt32 b = 0; b < blocks; ++b)
                eq.Process(input, output, static_cast<AmReal32>(kSampleRate));

            AudioBuffer last(kFrames, 1);
            AudioBuffer::Copy(output, 0, last, 0, kFrames);
            return last;
        }
    } // namespace

    AM_TEST_CASE(NodeTestCase, mixer_nodes, air_absorption_eq_crossfades_to_new_gains)
    {
    public:
        void Run() override
        {
            const AudioBuffer tone = MakeTone();

            // Reference: old (flat, 0 dB) response in steady state.
            AirAbsorptionEQFilter oldEq;
            oldEq.Configure(kFrames, 1);
            const AudioBuffer oldOut = RunBlocks(oldEq, tone, 3);

            // Reference: new response in steady state.
            AirAbsorptionEQFilter newEq;
            newEq.Configure(kFrames, 1);
            newEq.SetGains(0.0f, -6.0f, -12.0f);
            const AudioBuffer newOut = RunBlocks(newEq, tone, 3);

            // Under test: two flat blocks, then a gain change.
            AirAbsorptionEQFilter eq;
            eq.Configure(kFrames, 1);
            RunBlocks(eq, tone, 2);
            eq.SetGains(0.0f, -6.0f, -12.0f);
            const AudioBuffer changed = RunBlocks(eq, tone, 1);

            AmReal32 responseDiff = 0.0f;
            for (AmUInt64 i = 0; i < kFrames; ++i)
                responseDiff = std::max(responseDiff, std::abs(oldOut[0][i] - newOut[0][i]));

            // Precondition: the two responses differ audibly for this tone.
            AM_EXPECT(responseDiff > 0.05f);

            // The block starts on the old response...
            for (AmUInt64 i = 0; i < 4; ++i)
                AM_EXPECT(std::abs(changed[0][i] - oldOut[0][i]) < 1e-2f);

            // ...and ends on the new one.
            for (AmUInt64 i = kFrames - 4; i < kFrames; ++i)
                AM_EXPECT(std::abs(changed[0][i] - newOut[0][i]) < 1e-2f);

            // The following block is fully on the new response.
            const AudioBuffer after = RunBlocks(eq, tone, 1);
            for (AmUInt64 i = kFrames / 2; i < kFrames; ++i)
                AM_EXPECT(std::abs(after[0][i] - newOut[0][i]) < 1e-3f);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, air_absorption_eq_crossfades_to_new_gains);

    AM_TEST_CASE(NodeTestCase, mixer_nodes, air_absorption_eq_latest_set_gains_wins)
    {
    public:
        void Run() override
        {
            const AudioBuffer tone = MakeTone();

            AirAbsorptionEQFilter reference;
            reference.Configure(kFrames, 1);
            const AudioBuffer flat = RunBlocks(reference, tone, 3);

            AirAbsorptionEQFilter eq;
            eq.Configure(kFrames, 1);
            RunBlocks(eq, tone, 2);

            // Stage a change, then set the active (flat) gains again before processing: the change is cancelled.
            eq.SetGains(0.0f, -6.0f, -12.0f);
            eq.SetGains(0.0f, 0.0f, 0.0f);
            const AudioBuffer out = RunBlocks(eq, tone, 1);

            for (AmUInt64 i = 0; i < kFrames; ++i)
                AM_EXPECT(std::abs(out[0][i] - flat[0][i]) < 1e-5f);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, air_absorption_eq_latest_set_gains_wins);
} // namespace SparkyStudios::Audio::Amplitude::Tests
