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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <DSP/Filters/CompressorFilter.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, dsp_filters, compressor_filter_parameter_clamping)
    {
    public:
        void Run() override
        {
            auto filter = amshared(CompressorFilter);
            filter->Initialize();
            auto instance = filter->CreateInstance();

            // Ratio must be clamped to [1.0, 20.0] — setting 0.0 would cause division by zero
            instance->SetParameter(CompressorFilter::ATTRIBUTE_RATIO, 0.0f);
            AM_EXPECT_EQ(instance->GetParameter(CompressorFilter::ATTRIBUTE_RATIO), 1.0f);

            instance->SetParameter(CompressorFilter::ATTRIBUTE_RATIO, 50.0f);
            AM_EXPECT_EQ(instance->GetParameter(CompressorFilter::ATTRIBUTE_RATIO), 20.0f);

            // Attack must be clamped to [0.1, 100.0]
            instance->SetParameter(CompressorFilter::ATTRIBUTE_ATTACK, 0.0f);
            AM_EXPECT_EQ(instance->GetParameter(CompressorFilter::ATTRIBUTE_ATTACK), 0.1f);

            // Release must be clamped to [10.0, 1000.0]
            instance->SetParameter(CompressorFilter::ATTRIBUTE_RELEASE, 0.0f);
            AM_EXPECT_EQ(instance->GetParameter(CompressorFilter::ATTRIBUTE_RELEASE), 10.0f);

            // Threshold must be clamped to [-60.0, 0.0]
            instance->SetParameter(CompressorFilter::ATTRIBUTE_THRESHOLD, -100.0f);
            AM_EXPECT_EQ(instance->GetParameter(CompressorFilter::ATTRIBUTE_THRESHOLD), -60.0f);

            instance->SetParameter(CompressorFilter::ATTRIBUTE_THRESHOLD, 10.0f);
            AM_EXPECT_EQ(instance->GetParameter(CompressorFilter::ATTRIBUTE_THRESHOLD), 0.0f);

            // Wet must be clamped to [0.0, 1.0]
            instance->SetParameter(CompressorFilter::ATTRIBUTE_WET, -1.0f);
            AM_EXPECT_EQ(instance->GetParameter(CompressorFilter::ATTRIBUTE_WET), 0.0f);

            instance->SetParameter(CompressorFilter::ATTRIBUTE_WET, 2.0f);
            AM_EXPECT_EQ(instance->GetParameter(CompressorFilter::ATTRIBUTE_WET), 1.0f);

            // Knee must be clamped to [0.0, 12.0]
            instance->SetParameter(CompressorFilter::ATTRIBUTE_KNEE, -5.0f);
            AM_EXPECT_EQ(instance->GetParameter(CompressorFilter::ATTRIBUTE_KNEE), 0.0f);

            // Makeup gain must be clamped to [0.0, 24.0]
            instance->SetParameter(CompressorFilter::ATTRIBUTE_MAKEUP_GAIN, 30.0f);
            AM_EXPECT_EQ(instance->GetParameter(CompressorFilter::ATTRIBUTE_MAKEUP_GAIN), 24.0f);
        }
    };

    AM_REGISTER_TEST(dsp_filters, compressor_filter_parameter_clamping);
} // namespace SparkyStudios::Audio::Amplitude::Tests
