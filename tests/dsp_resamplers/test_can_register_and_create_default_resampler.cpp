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
    AM_TEST_CASE(ComponentTestCase, dsp_resamplers, can_register_and_create_default_resampler)
    {
    public:
        void Run() override
        {
            auto resampler = amshared(DefaultResampler);

            Resampler::Unregister(Resampler::Find("default"));
            Resampler::Register(resampler);

            AM_EXPECT_NOT(Resampler::Find("default") == nullptr);
            AM_EXPECT(Resampler::Find("default")->GetName() == "default");

            auto instance = Resampler::Construct("default");
            AM_EXPECT_NOT(instance == nullptr);

            Resampler::Unregister(resampler);
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, can_register_and_create_default_resampler);
} // namespace SparkyStudios::Audio::Amplitude::Tests
