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

#include <DSP/Resamplers/BandlimitedResampler.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_resamplers, default_resampler_clamps_zero_sample_rate)
    {
    public:
        void Run() override
        {
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt64 inputFrames = 1024;
            constexpr AmUInt64 outputFrames = 512;

            auto resampler = amshared(BandlimitedResampler, "default", eResamplerPreset::SincBest);
            auto instance = resampler->CreateInstance();

            // A rate of zero reaches the mixer in release builds: the instance must clamp it and stay usable.
            instance->Initialize(channelCount, 0, 1000);

            AM_EXPECT(instance->GetSampleRateIn() > 0);
            AM_EXPECT(instance->GetSampleRateOut() == 1000);

            instance->Initialize(channelCount, 48000, 48000);

            AudioBuffer inputBuffer(inputFrames, channelCount);
            GenerateSineWave(inputBuffer, 48000);

            AudioBuffer outputBuffer(outputFrames, channelCount);

            AmUInt64 processedInputFrames = inputFrames;
            AmUInt64 processedOutputFrames = outputFrames;

            AM_EXPECT(instance->Process(inputBuffer, processedInputFrames, outputBuffer, processedOutputFrames));
            AM_EXPECT_EQ(outputFrames, processedOutputFrames);
            AM_EXPECT(EnsureHasNonZeroOutput(outputBuffer));
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, default_resampler_clamps_zero_sample_rate);
} // namespace SparkyStudios::Audio::Amplitude::Tests
