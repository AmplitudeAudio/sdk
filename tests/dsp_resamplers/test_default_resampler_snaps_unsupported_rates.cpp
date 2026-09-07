// Copyright (c) 2021-present Sparky Studios. All rights reserved.
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

#include <DSP/Resamplers/DefaultResampler.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, dsp_resamplers, default_resampler_snaps_unsupported_rates)
    {
    public:
        void Run() override
        {
            auto resampler = amshared(DefaultResampler);
            auto instance = std::static_pointer_cast<DefaultResamplerInstance>(resampler->CreateInstance());

            AM_EXPECT_NOT(instance == nullptr);

            // 44056 -> 48000 reduces to 6000 / 5507; unsnapped it needs 78000 coefficients.
            instance->Initialize(1, 44056, 48000);

            AM_EXPECT(instance->GetUpRate() <= kMaxPolyphaseRate);
            AM_EXPECT(instance->GetDownRate() <= kMaxPolyphaseRate);

            // The requested rates are still reported, not the internal approximation.
            AM_EXPECT_EQ(instance->GetSampleRateIn(), 44056U);
            AM_EXPECT_EQ(instance->GetSampleRateOut(), 48000U);

            // A standard pair is left exact.
            instance->Initialize(1, 44100, 48000);

            AM_EXPECT_EQ(instance->GetUpRate(), 160ULL);
            AM_EXPECT_EQ(instance->GetDownRate(), 147ULL);
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, default_resampler_snaps_unsupported_rates);
} // namespace SparkyStudios::Audio::Amplitude::Tests
