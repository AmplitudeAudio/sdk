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
    AM_TEST_CASE(DSPTestCase, dsp_filters, delay_filter_decay_stability)
    {
    public:
        void Run() override
        {
            // Verify constructor path clamps decay from Initialize
            auto filter2 = amshared(DelayFilter);
            filter2->Initialize(0.005f, 1.0f, 0.0f);

            auto instance2 = filter2->CreateInstance();
            AM_EXPECT_NOT(instance2 == nullptr);
            AM_EXPECT(instance2->GetParameter(DelayFilter::ATTRIBUTE_DECAY) <= 0.999f);

            constexpr AmUInt64 frameCount = 1024;
            constexpr AmUInt32 sampleRate = 48000;

            AudioBuffer in2(frameCount, 1);
            AudioBuffer out2(frameCount, 1);
            in2[0][0] = 1.0f;

            for (int block = 0; block < 10; ++block)
            {
                instance2->Process(in2, out2, frameCount, sampleRate);
                in2[0].clear();
            }

            for (AmUInt64 f = 0; f < frameCount; ++f)
            {
                AM_EXPECT(!std::isnan(out2[0][f]));
                AM_EXPECT(!std::isinf(out2[0][f]));
                AM_EXPECT(std::abs(out2[0][f]) < 1.0f);
            }

            // Verify SetParameter path also clamps decay
            auto filter = amshared(DelayFilter);
            filter->Initialize(0.005f, 0.5f, 0.0f);

            auto instance = filter->CreateInstance();
            AM_EXPECT_NOT(instance == nullptr);

            instance->SetParameter(DelayFilter::ATTRIBUTE_DELAY, 0.002f);
            instance->SetParameter(DelayFilter::ATTRIBUTE_DECAY, 1.0f); // Should clamp to <= 0.999f
            instance->SetParameter(DelayFilter::ATTRIBUTE_WET, 1.0f);

            AudioBuffer in(frameCount, 1);
            AudioBuffer out(frameCount, 1);

            in[0][0] = 1.0f;

            // Process several consecutive blocks
            for (int block = 0; block < 10; ++block)
            {
                instance->Process(in, out, frameCount, sampleRate);
                in[0].clear();
            }

            // Energy must decay, never diverge above 2.0 or become NaN
            for (AmUInt64 f = 0; f < frameCount; ++f)
            {
                AM_EXPECT(!std::isnan(out[0][f]));
                AM_EXPECT(!std::isinf(out[0][f]));
                AM_EXPECT(std::abs(out[0][f]) < 1.0f);
            }
        }
    };

    AM_REGISTER_TEST(dsp_filters, delay_filter_decay_stability);
} // namespace SparkyStudios::Audio::Amplitude::Tests
