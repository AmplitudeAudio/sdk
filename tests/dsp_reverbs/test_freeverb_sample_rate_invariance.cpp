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
#include <Utils/Freeverb/ReverbModel.h>

#include <cmath>
#include <vector>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_reverbs, freeverb_sample_rate_invariance)
    {
    public:
        void Run() override
        {
            Freeverb::ReverbModel model48k;
            model48k.SetSampleRate(48000);
            model48k.SetRoomSize(0.5f);
            model48k.SetWet(1.0f);
            model48k.SetDry(0.0f);

            Freeverb::ReverbModel model96k;
            model96k.SetSampleRate(96000);
            model96k.SetRoomSize(0.5f);
            model96k.SetWet(1.0f);
            model96k.SetDry(0.0f);

            // Verify sample rate configuration
            AM_EXPECT_EQ(model48k.GetSampleRate(), 48000u);
            AM_EXPECT_EQ(model96k.GetSampleRate(), 96000u);

            // Feed an impulse at 48k (1 second = 48000 frames)
            constexpr AmUInt64 frames48k = 48000;
            std::vector<AmReal32> inL48(frames48k, 0.0f);
            std::vector<AmReal32> inR48(frames48k, 0.0f);
            std::vector<AmReal32> outL48(frames48k, 0.0f);
            std::vector<AmReal32> outR48(frames48k, 0.0f);
            inL48[0] = 1.0f;
            inR48[0] = 1.0f;

            model48k.ProcessReplace(inL48.data(), inR48.data(), outL48.data(), outR48.data(), frames48k, 1);

            // Feed an impulse at 96k (1 second = 96000 frames)
            constexpr AmUInt64 frames96k = 96000;
            std::vector<AmReal32> inL96(frames96k, 0.0f);
            std::vector<AmReal32> inR96(frames96k, 0.0f);
            std::vector<AmReal32> outL96(frames96k, 0.0f);
            std::vector<AmReal32> outR96(frames96k, 0.0f);
            inL96[0] = 1.0f;
            inR96[0] = 1.0f;

            model96k.ProcessReplace(inL96.data(), inR96.data(), outL96.data(), outR96.data(), frames96k, 1);

            // Verify stability: no NaNs, Infs, or unbounded growth at either sample rate
            bool isStable48 = true;
            for (AmUInt64 f = 0; f < frames48k; ++f)
            {
                if (std::isnan(outL48[f]) || std::isinf(outL48[f]) ||
                    std::isnan(outR48[f]) || std::isinf(outR48[f]) ||
                    std::abs(outL48[f]) >= 2.0f || std::abs(outR48[f]) >= 2.0f)
                {
                    isStable48 = false;
                    break;
                }
            }
            AM_EXPECT(isStable48);

            bool isStable96 = true;
            for (AmUInt64 f = 0; f < frames96k; ++f)
            {
                if (std::isnan(outL96[f]) || std::isinf(outL96[f]) ||
                    std::isnan(outR96[f]) || std::isinf(outR96[f]) ||
                    std::abs(outL96[f]) >= 2.0f || std::abs(outR96[f]) >= 2.0f)
                {
                    isStable96 = false;
                    break;
                }
            }
            AM_EXPECT(isStable96);

            // Measure energy in equivalent physical time windows:
            // Early window: 50ms to 200ms
            // Late window: 500ms to 800ms
            AmReal32 earlyEnergy48 = 0.0f;
            for (AmUInt64 f = 2400; f < 9600; ++f)
                earlyEnergy48 += outL48[f] * outL48[f] + outR48[f] * outR48[f];

            AmReal32 lateEnergy48 = 0.0f;
            for (AmUInt64 f = 24000; f < 38400; ++f)
                lateEnergy48 += outL48[f] * outL48[f] + outR48[f] * outR48[f];

            AmReal32 earlyEnergy96 = 0.0f;
            for (AmUInt64 f = 4800; f < 19200; ++f)
                earlyEnergy96 += outL96[f] * outL96[f] + outR96[f] * outR96[f];

            AmReal32 lateEnergy96 = 0.0f;
            for (AmUInt64 f = 48000; f < 76800; ++f)
                lateEnergy96 += outL96[f] * outL96[f] + outR96[f] * outR96[f];

            // Verify that both have active early reverb energy
            AM_EXPECT(earlyEnergy48 > 0.01f);
            AM_EXPECT(earlyEnergy96 > 0.01f);

            // Verify stable decay: late energy must be strictly less than early energy
            AM_EXPECT(lateEnergy48 < earlyEnergy48);
            AM_EXPECT(lateEnergy96 < earlyEnergy96);

            // Normalize energy by sample count in window (7200 samples at 48k vs 14400 samples at 96k)
            const AmReal32 meanEarlyEnergy48 = earlyEnergy48 / 7200.0f;
            const AmReal32 meanLateEnergy48 = lateEnergy48 / 14400.0f;
            const AmReal32 decayRatio48 = meanLateEnergy48 / meanEarlyEnergy48;

            const AmReal32 meanEarlyEnergy96 = earlyEnergy96 / 14400.0f;
            const AmReal32 meanLateEnergy96 = lateEnergy96 / 28800.0f;
            const AmReal32 decayRatio96 = meanLateEnergy96 / meanEarlyEnergy96;

            // Decay ratios should be consistent across sample rates
            AM_EXPECT(decayRatio48 < 0.2f);
            AM_EXPECT(decayRatio96 < 0.2f);
            AM_EXPECT(std::abs(decayRatio48 - decayRatio96) < 0.1f);
        }
    };

    AM_REGISTER_TEST(dsp_reverbs, freeverb_sample_rate_invariance);
} // namespace SparkyStudios::Audio::Amplitude::Tests
