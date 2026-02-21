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

#include <DSP/Filters/LofiFilter.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, dsp_filters, lofi_filter_parameters)
    {
    public:
        void Run() override
        {
            auto filter = amshared(LofiFilter);
            // Test failure paths - negative sample rate
            auto result = filter->Initialize(-1.0f, 8.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            // Test failure paths - zero sample rate
            result = filter->Initialize(0.0f, 8.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            // Test failure paths - negative bit depth
            result = filter->Initialize(22050.0f, -1.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            // Test failure paths - zero bit depth
            result = filter->Initialize(22050.0f, 0.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            // Test success path
            result = filter->Initialize(22050.0f, 8.0f);
            AM_EXPECT_EQ(result, eErrorCode_Success);

            auto instance = filter->CreateInstance();
            AM_EXPECT_NOT(instance == nullptr);

            // Test parameter get/set
            instance->SetParameter(LofiFilter::ATTRIBUTE_WET, 0.5f);
            AM_EXPECT_EQ(instance->GetParameter(LofiFilter::ATTRIBUTE_WET), 0.5f);

            instance->SetParameter(LofiFilter::ATTRIBUTE_SAMPLERATE, 11025.0f);
            AM_EXPECT_EQ(instance->GetParameter(LofiFilter::ATTRIBUTE_SAMPLERATE), 11025.0f);

            instance->SetParameter(LofiFilter::ATTRIBUTE_BITDEPTH, 4.0f);
            AM_EXPECT_EQ(instance->GetParameter(LofiFilter::ATTRIBUTE_BITDEPTH), 4.0f);

            // Test with full wet
            instance->SetParameter(LofiFilter::ATTRIBUTE_WET, 1.0f);
            AM_EXPECT_EQ(instance->GetParameter(LofiFilter::ATTRIBUTE_WET), 1.0f);

            // Test with dry signal
            instance->SetParameter(LofiFilter::ATTRIBUTE_WET, 0.0f);
            AM_EXPECT_EQ(instance->GetParameter(LofiFilter::ATTRIBUTE_WET), 0.0f);

            // Test with incorrect param
            instance->SetParameter(LofiFilter::ATTRIBUTE_LAST, 5.0f);
            AM_EXPECT_EQ(instance->GetParameter(LofiFilter::ATTRIBUTE_LAST), 0.0f);

            AM_EXPECT_EQ(filter->GetParameterCount(), 3);

            AM_EXPECT_EQ(filter->GetParameterMin(LofiFilter::ATTRIBUTE_WET), 0.0f);
            AM_EXPECT_EQ(filter->GetParameterMin(LofiFilter::ATTRIBUTE_SAMPLERATE), 100.0f);
            AM_EXPECT_EQ(filter->GetParameterMin(LofiFilter::ATTRIBUTE_BITDEPTH), 0.5f);
            AM_EXPECT_EQ(filter->GetParameterMin(LofiFilter::ATTRIBUTE_LAST), 0.0f);

            AM_EXPECT_EQ(filter->GetParameterMax(LofiFilter::ATTRIBUTE_WET), 1.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(LofiFilter::ATTRIBUTE_SAMPLERATE), 22000.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(LofiFilter::ATTRIBUTE_BITDEPTH), 16.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(LofiFilter::ATTRIBUTE_LAST), 0.0f);

            AM_EXPECT_EQ(filter->GetParameterType(LofiFilter::ATTRIBUTE_WET), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(LofiFilter::ATTRIBUTE_SAMPLERATE), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(LofiFilter::ATTRIBUTE_BITDEPTH), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(LofiFilter::ATTRIBUTE_LAST), eParameterType_Float);

            AM_EXPECT_EQ(filter->GetParameterName(LofiFilter::ATTRIBUTE_WET), "Wet");
            AM_EXPECT_EQ(filter->GetParameterName(LofiFilter::ATTRIBUTE_SAMPLERATE), "Samplerate");
            AM_EXPECT_EQ(filter->GetParameterName(LofiFilter::ATTRIBUTE_BITDEPTH), "Bit Depth");
            AM_EXPECT_EQ(filter->GetParameterName(LofiFilter::ATTRIBUTE_LAST), "Unknown");
        }
    };

    AM_REGISTER_TEST(dsp_filters, lofi_filter_parameters);
} // namespace SparkyStudios::Audio::Amplitude::Tests
