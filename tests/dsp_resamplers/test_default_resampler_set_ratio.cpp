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
#include <limits>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <DSP/Resamplers/DefaultResampler.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_resamplers, default_resampler_set_ratio)
    {
    public:
        void Run() override
        {
            DefaultResamplerInstance instance;
            instance.Initialize(1, 44100, 48000);
            AM_EXPECT_EQ(160ULL, instance.GetUpRate());
            AM_EXPECT_EQ(147ULL, instance.GetDownRate());

            // The same ratio as a double keeps the exact polyphase pair.
            instance.SetRatio(44100.0 / 48000.0);
            AM_EXPECT_EQ(160ULL, instance.GetUpRate());
            AM_EXPECT_EQ(147ULL, instance.GetDownRate());

            // A pitched ratio stays within the filter budget and is not rounded to 1/1000.
            instance.SetRatio(44100.0 * 1.0005 / 48000.0);
            AM_EXPECT(instance.GetUpRate() <= kMaxPolyphaseRate);
            AM_EXPECT(instance.GetDownRate() <= kMaxPolyphaseRate);
            const AmReal64 achieved = static_cast<AmReal64>(instance.GetDownRate()) / static_cast<AmReal64>(instance.GetUpRate());
            AM_EXPECT(std::abs(achieved / (44100.0 * 1.0005 / 48000.0) - 1.0) < 1e-5);

            // Hostile values are clamped, never crash.
            instance.SetRatio(0.0);
            instance.SetRatio(-1.0);
            instance.SetRatio(std::numeric_limits<AmReal64>::infinity());
            AM_EXPECT(instance.GetUpRate() > 0 && instance.GetDownRate() > 0);
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, default_resampler_set_ratio);
} // namespace SparkyStudios::Audio::Amplitude::Tests
