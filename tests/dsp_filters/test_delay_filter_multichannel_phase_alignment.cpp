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
#include <DSP/Filters/DelayFilter.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_filters, delay_filter_multichannel_phase_alignment)
    {
    public:
        void Run() override
        {
            auto filter = amshared(DelayFilter);
            filter->Initialize(0.01f, 0.5f, 1.0f); // 10 ms delay, delay start enabled

            auto instance = filter->CreateInstance();
            AM_EXPECT_NOT(instance == nullptr);

            instance->SetParameter(DelayFilter::ATTRIBUTE_DELAY, 0.001f); // ~48 samples
            instance->SetParameter(DelayFilter::ATTRIBUTE_DECAY, 0.0f); // no feedback decay
            instance->SetParameter(DelayFilter::ATTRIBUTE_WET, 1.0f);

            constexpr AmUInt64 frameCount = 512;
            constexpr AmUInt16 channelCount = 2;
            constexpr AmUInt32 sampleRate = 48000;

            AudioBuffer in(frameCount, channelCount);
            AudioBuffer out(frameCount, channelCount);

            // Feed identical impulse on channel 0 and channel 1
            in[0][0] = 1.0f;
            in[1][0] = 1.0f;

            instance->Process(in, out, frameCount, sampleRate);

            // Channel 0 and Channel 1 must have their delayed peak at the exact same index
            AmUInt64 peak0 = 0;
            AmUInt64 peak1 = 0;
            for (AmUInt64 f = 0; f < frameCount; ++f)
            {
                if (out[0][f] > 0.5f) peak0 = f;
                if (out[1][f] > 0.5f) peak1 = f;
            }

            AM_EXPECT_EQ(peak0, peak1);
            AM_EXPECT(peak0 > 0);
        }
    };

    AM_REGISTER_TEST(dsp_filters, delay_filter_multichannel_phase_alignment);
} // namespace SparkyStudios::Audio::Amplitude::Tests
