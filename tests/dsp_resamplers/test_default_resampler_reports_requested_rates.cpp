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
    AM_TEST_CASE(ComponentTestCase, dsp_resamplers, default_resampler_reports_requested_rates)
    {
    public:
        void Run() override
        {
            auto resampler = amshared(DefaultResampler);
            auto instance = resampler->CreateInstance();

            AM_EXPECT_NOT(instance == nullptr);

            instance->Initialize(1, 24000, 48000);

            AM_EXPECT_EQ(instance->GetSampleRateIn(), 24000U);
            AM_EXPECT_EQ(instance->GetSampleRateOut(), 48000U);

            // 22050 / 44100 reduces to the same 2 / 1 ratio as 24000 / 48000, so the internal rates do not
            // change. The accessors must still report what the caller requested.
            instance->Initialize(1, 22050, 44100);

            AM_EXPECT_EQ(instance->GetSampleRateIn(), 22050U);
            AM_EXPECT_EQ(instance->GetSampleRateOut(), 44100U);

            // Snapped pairs report the requested rates too, not the approximated ratio.
            instance->Initialize(1, 44056, 48000);

            AM_EXPECT_EQ(instance->GetSampleRateIn(), 44056U);
            AM_EXPECT_EQ(instance->GetSampleRateOut(), 48000U);
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, default_resampler_reports_requested_rates);
} // namespace SparkyStudios::Audio::Amplitude::Tests
