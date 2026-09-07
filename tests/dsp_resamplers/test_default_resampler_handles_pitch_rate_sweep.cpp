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
    AM_TEST_CASE(DSPTestCase, dsp_resamplers, default_resampler_handles_pitch_rate_sweep)
    {
    public:
        void Run() override
        {
            // AmplimixImpl::UpdatePitch maps playback speed to SetSampleRate(ratio * 1000, 1000) on the audio
            // thread. Ratios from 1.261 upwards used to overflow the coefficient buffers.
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt64 inputFrames = 256;

            auto resampler = amshared(DefaultResampler);
            auto instance = std::static_pointer_cast<DefaultResamplerInstance>(resampler->CreateInstance());

            instance->Initialize(channelCount, 48000, 48000);

            bool bounded = true;
            for (AmUInt32 s = 1; s <= 4000 && bounded; ++s)
            {
                instance->SetSampleRate(s, 1000);

                bounded = instance->GetUpRate() <= kMaxPolyphaseRate && instance->GetDownRate() <= kMaxPolyphaseRate;

                // Process on a subset of the sweep to keep the test fast while still exercising the filter.
                if (s % 250 == 0)
                {
                    AudioBuffer inputBuffer(inputFrames, channelCount);
                    GenerateSineWave(inputBuffer, 48000);

                    const AmUInt64 expectedOutputFrames = instance->GetExpectedOutputFrames(inputFrames);
                    AudioBuffer outputBuffer(expectedOutputFrames, channelCount);

                    AmUInt64 processedInputFrames = inputFrames;
                    AmUInt64 processedOutputFrames = expectedOutputFrames;

                    AM_EXPECT(instance->Process(inputBuffer, processedInputFrames, outputBuffer, processedOutputFrames));
                }
            }

            AM_EXPECT(bounded);
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, default_resampler_handles_pitch_rate_sweep);
} // namespace SparkyStudios::Audio::Amplitude::Tests
