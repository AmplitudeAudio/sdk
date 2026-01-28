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

#include "SimpleTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, dsp_resamplers, default_resampler_can_reset)
    {
    public:
        void Run() override
        {
            auto resampler = amshared(DefaultResampler);
            auto instance = resampler->CreateInstance();

            constexpr AmUInt16 channelCount = 2;
            constexpr AmUInt32 sampleRateIn = 44100;
            constexpr AmUInt32 sampleRateOut = 48000;

            instance->Initialize(channelCount, sampleRateIn, sampleRateOut);

            // Reset should not cause any issues
            instance->Reset();

            AM_EXPECT(instance->GetChannelCount() == channelCount);
            AM_EXPECT(instance->GetSampleRateIn() == sampleRateIn);
            AM_EXPECT(instance->GetSampleRateOut() == sampleRateOut);

            // Clear should clean up everything
            instance->Clear();

            AM_EXPECT(instance->GetChannelCount() == 0);
            AM_EXPECT(instance->GetSampleRateIn() == 0);
            AM_EXPECT(instance->GetSampleRateOut() == 0);
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, default_resampler_can_reset);
} // namespace SparkyStudios::Audio::Amplitude::Tests
