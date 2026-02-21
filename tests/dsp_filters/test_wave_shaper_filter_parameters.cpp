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

#include <DSP/Filters/WaveShaperFilter.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, dsp_filters, wave_shaper_filter_parameters)
    {
    public:
        void Run() override
        {
            auto filter = amshared(WaveShaperFilter);
            // Test failure paths - amount < -1.0f
            auto result = filter->Initialize(-2.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            // Test failure paths - amount > 1.0f
            result = filter->Initialize(2.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            // Test success path
            result = filter->Initialize(0.5f);
            AM_EXPECT_EQ(result, eErrorCode_Success);

            auto instance = filter->CreateInstance();
            AM_EXPECT_NOT(instance == nullptr);

            // Test parameter get/set
            instance->SetParameter(WaveShaperFilter::ATTRIBUTE_WET, 0.5f);
            AM_EXPECT_EQ(instance->GetParameter(WaveShaperFilter::ATTRIBUTE_WET), 0.5f);

            instance->SetParameter(WaveShaperFilter::ATTRIBUTE_AMOUNT, 1.5f);
            AM_EXPECT_EQ(instance->GetParameter(WaveShaperFilter::ATTRIBUTE_AMOUNT), 1.5f);

            // Test with full wet
            instance->SetParameter(WaveShaperFilter::ATTRIBUTE_WET, 1.0f);
            AM_EXPECT_EQ(instance->GetParameter(WaveShaperFilter::ATTRIBUTE_WET), 1.0f);

            // Test with dry signal
            instance->SetParameter(WaveShaperFilter::ATTRIBUTE_WET, 0.0f);
            AM_EXPECT_EQ(instance->GetParameter(WaveShaperFilter::ATTRIBUTE_WET), 0.0f);

            // Test with incorrect param
            instance->SetParameter(WaveShaperFilter::ATTRIBUTE_LAST, 5.0f);
            AM_EXPECT_EQ(instance->GetParameter(WaveShaperFilter::ATTRIBUTE_LAST), 0.0f);

            AM_EXPECT_EQ(filter->GetParameterCount(), 2);

            AM_EXPECT_EQ(filter->GetParameterMin(WaveShaperFilter::ATTRIBUTE_WET), 0.0f);
            AM_EXPECT_EQ(filter->GetParameterMin(WaveShaperFilter::ATTRIBUTE_AMOUNT), -1.0f);
            AM_EXPECT_EQ(filter->GetParameterMin(WaveShaperFilter::ATTRIBUTE_LAST), 0.0f);

            AM_EXPECT_EQ(filter->GetParameterMax(WaveShaperFilter::ATTRIBUTE_WET), 1.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(WaveShaperFilter::ATTRIBUTE_AMOUNT), 1.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(WaveShaperFilter::ATTRIBUTE_LAST), 0.0f);

            AM_EXPECT_EQ(filter->GetParameterType(WaveShaperFilter::ATTRIBUTE_WET), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(WaveShaperFilter::ATTRIBUTE_AMOUNT), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(WaveShaperFilter::ATTRIBUTE_LAST), eParameterType_Float);

            AM_EXPECT_EQ(filter->GetParameterName(WaveShaperFilter::ATTRIBUTE_WET), "Wet");
            AM_EXPECT_EQ(filter->GetParameterName(WaveShaperFilter::ATTRIBUTE_AMOUNT), "Amount");
            AM_EXPECT_EQ(filter->GetParameterName(WaveShaperFilter::ATTRIBUTE_LAST), "Unknown");
        }
    };

    AM_REGISTER_TEST(dsp_filters, wave_shaper_filter_parameters);
} // namespace SparkyStudios::Audio::Amplitude::Tests
