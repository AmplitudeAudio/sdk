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
    AM_TEST_CASE(DSPTestCase, dsp_filters, delay_filter_zero_delay_does_not_crash)
    {
    public:
        void Run() override
        {
            auto filter = amshared(DelayFilter);
            filter->Initialize(0.1f, 0.5f, 0.0f);

            auto instance = filter->CreateInstance();
            AM_EXPECT_NOT(instance == nullptr);

            instance->SetParameter(DelayFilter::ATTRIBUTE_DELAY, 0.0f);
            instance->SetParameter(DelayFilter::ATTRIBUTE_WET, 1.0f);

            constexpr AmUInt64 frameCount = 256;
            constexpr AmUInt16 channelCount = 2;
            constexpr AmUInt32 sampleRate = 48000;

            AudioBuffer in(frameCount, channelCount);
            AudioBuffer out(frameCount, channelCount);

            for (AmUInt16 c = 0; c < channelCount; ++c)
                for (AmUInt64 f = 0; f < frameCount; ++f)
                    in[c][f] = 0.5f;

            instance->Process(in, out, frameCount, sampleRate);

            // With delay == 0, output should not crash and should equal input
            for (AmUInt16 c = 0; c < channelCount; ++c)
                for (AmUInt64 f = 0; f < frameCount; ++f)
                    AM_EXPECT_EQ(out[c][f], 0.5f);
        }
    };

    AM_REGISTER_TEST(dsp_filters, delay_filter_zero_delay_does_not_crash);
} // namespace SparkyStudios::Audio::Amplitude::Tests
