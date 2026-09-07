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
    AM_TEST_CASE(ComponentTestCase, dsp_resamplers, default_resampler_reports_exact_conversion)
    {
    public:
        void Run() override
        {
            auto resampler = amshared(DefaultResampler);
            auto instance = resampler->CreateInstance();

            AM_EXPECT_NOT(instance == nullptr);

            // Standard rate pairs reduce to small fractions and are converted exactly.
            AM_EXPECT(instance->IsConversionExact(44100, 48000));
            AM_EXPECT(instance->IsConversionExact(48000, 44100));
            AM_EXPECT(instance->IsConversionExact(22050, 44100));
            AM_EXPECT(instance->IsConversionExact(44100, 22050));
            AM_EXPECT(instance->IsConversionExact(44100, 44100));
            AM_EXPECT(instance->IsConversionExact(48000, 48000));

            // 44056 -> 48000 reduces to 6000 / 5507 and must be reported as approximated.
            AM_EXPECT_NOT(instance->IsConversionExact(44056, 48000));

            // 1000 -> 1171 passed the old IsConversionSupported() guard yet overflowed the transposed
            // coefficient buffer; it must be reported as approximated.
            AM_EXPECT_NOT(instance->IsConversionExact(1000, 1171));

            // Zero rates are not a valid exact conversion.
            AM_EXPECT_NOT(instance->IsConversionExact(0, 48000));
            AM_EXPECT_NOT(instance->IsConversionExact(48000, 0));
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, default_resampler_reports_exact_conversion);
} // namespace SparkyStudios::Audio::Amplitude::Tests
