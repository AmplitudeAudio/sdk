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
            constexpr AmUInt64 inputFrames = 256;

            auto resampler = amshared(DefaultResampler);
            auto instance = std::static_pointer_cast<DefaultResamplerInstance>(resampler->CreateInstance());

            instance->Initialize(channelCount, 48000, 48000);

            // AmplimixImpl::UpdatePitch computes s = (AmUInt64)(ratio * 1000), which truncates to 0 for very
            // small ratios. The instance must survive this and stay usable.
            instance->SetSampleRate(0, 1000);

            AM_EXPECT(instance->GetDownRate() > 0);
            AM_EXPECT(instance->GetUpRate() <= kMaxPolyphaseRate);
            AM_EXPECT(instance->GetDownRate() <= kMaxPolyphaseRate);

            instance->SetSampleRate(48000, 48000);

            AudioBuffer inputBuffer(inputFrames, channelCount);
            GenerateSineWave(inputBuffer, 48000);

            const AmUInt64 expectedOutputFrames = instance->GetExpectedOutputFrames(inputFrames);
            AudioBuffer outputBuffer(expectedOutputFrames, channelCount);

            AmUInt64 processedInputFrames = inputFrames;
            AmUInt64 processedOutputFrames = expectedOutputFrames;

            AM_EXPECT(instance->Process(inputBuffer, processedInputFrames, outputBuffer, processedOutputFrames));
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, default_resampler_clamps_zero_sample_rate);
} // namespace SparkyStudios::Audio::Amplitude::Tests
