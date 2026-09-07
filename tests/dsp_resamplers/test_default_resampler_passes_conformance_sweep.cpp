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
    AM_TEST_CASE(DSPTestCase, dsp_resamplers, default_resampler_passes_conformance_sweep)
    {
    public:
        void Run() override
        {
            auto resampler = amshared(DefaultResampler);
            auto instance = resampler->CreateInstance();

            AM_EXPECT_NOT(instance == nullptr);
            AM_EXPECT(RunResamplerConformanceSweep(*instance, 1));
            AM_EXPECT(RunResamplerConformanceSweep(*instance, 2));
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, default_resampler_passes_conformance_sweep);
} // namespace SparkyStudios::Audio::Amplitude::Tests
