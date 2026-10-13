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
#include <array>
#include <atomic>
#include <cmath>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        std::array<std::atomic<AmReal32>, 6> gPeak{};

        void RecordPeaks(const Amplimix*, const AudioBuffer* buffer, AmUInt32 frames)
        {
            for (AmUInt16 c = 0; c < 6; ++c)
            {
                AmReal32 peak = gPeak[c].load(std::memory_order_relaxed);
                for (AmUInt32 i = 0; i < frames; ++i)
                    peak = std::max(peak, std::abs(buffer->GetChannel(c)[i]));

                gPeak[c].store(peak, std::memory_order_relaxed);
            }
        }
    } // namespace

    class SurroundEngineTestCase : public EngineTestCase
    {
    public:
        [[nodiscard]] const AmOsChar* GetConfigFile() const override
        {
            return AM_OS_STRING("tests.surround51.config.amconfig");
        }
    };

    // A stereo asset played 2D on a 5.1 output lands on L and R only: the whole chain renders the configured layout.
    AM_TEST_CASE(SurroundEngineTestCase, core_engine, engine_renders_surround_output)
    {
    public:
        void Run() override
        {
            AM_EXPECT_EQ(eChannelLayout_Surround_5_1, amEngine->GetOutputLayout().GetType());

            for (auto& peak : gPeak)
                peak.store(0.0f);

            amEngine->GetMixer()->SetAfterMixCallback(&RecordPeaks);

            Channel channel = amEngine->Play(amEngine->GetSoundHandle("test_sound_06"));
            AM_EXPECT(channel.Valid());

            for (AmUInt32 i = 0; i < 100 && gPeak[0].load() == 0.0f; ++i)
                Thread::Sleep(10);
            Thread::Sleep(50);

            channel.Stop(0.0);
            amEngine->GetMixer()->SetAfterMixCallback(nullptr);

            AM_EXPECT(gPeak[0].load() > 0.0f); // L
            AM_EXPECT(gPeak[1].load() > 0.0f); // R
            AM_EXPECT_EQ(0.0f, gPeak[2].load()); // C
            AM_EXPECT_EQ(0.0f, gPeak[3].load()); // LFE
            AM_EXPECT_EQ(0.0f, gPeak[4].load()); // Ls
            AM_EXPECT_EQ(0.0f, gPeak[5].load()); // Rs
        }
    };

    AM_REGISTER_TEST(core_engine, engine_renders_surround_output);
} // namespace SparkyStudios::Audio::Amplitude::Tests
