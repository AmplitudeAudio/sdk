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

#include <DSP/Filters/DelayFilter.h>

#include "SimpleTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, dsp_filters, delay_filter_parameters)
    {
    public:
        void Run() override
        {
            auto filter = amshared(DelayFilter);

            // Test failure paths - negative delay
            auto result = filter->Initialize(-1.0f, 0.7f, 0.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            // Test failure paths - zero delay
            result = filter->Initialize(0.0f, 0.7f, 0.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            // Test failure paths - negative decay
            result = filter->Initialize(0.5f, -1.0f, 0.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            // Test failure paths - zero decay
            result = filter->Initialize(0.5f, 0.0f, 0.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            // Test success path
            result = filter->Initialize(0.5f, 0.7f, 0.0f);
            AM_EXPECT_EQ(result, eErrorCode_Success);

            auto instance = filter->CreateInstance();
            AM_EXPECT_NOT(instance == nullptr);

            // Test parameter get/set
            instance->SetParameter(DelayFilter::ATTRIBUTE_WET, 0.5f);
            AM_EXPECT_EQ(instance->GetParameter(DelayFilter::ATTRIBUTE_WET), 0.5f);

            instance->SetParameter(DelayFilter::ATTRIBUTE_DELAY, 0.3f);
            AM_EXPECT_EQ(instance->GetParameter(DelayFilter::ATTRIBUTE_DELAY), 0.3f);

            instance->SetParameter(DelayFilter::ATTRIBUTE_DECAY, 0.8f);
            AM_EXPECT_EQ(instance->GetParameter(DelayFilter::ATTRIBUTE_DECAY), 0.8f);

            instance->SetParameter(DelayFilter::ATTRIBUTE_DELAY_START, 0.1f);
            AM_EXPECT_EQ(instance->GetParameter(DelayFilter::ATTRIBUTE_DELAY_START), 0.1f);

            // Test with full wet
            instance->SetParameter(DelayFilter::ATTRIBUTE_WET, 1.0f);
            AM_EXPECT_EQ(instance->GetParameter(DelayFilter::ATTRIBUTE_WET), 1.0f);

            // Test with dry signal
            instance->SetParameter(DelayFilter::ATTRIBUTE_WET, 0.0f);
            AM_EXPECT_EQ(instance->GetParameter(DelayFilter::ATTRIBUTE_WET), 0.0f);

            // Test with incorrect param
            instance->SetParameter(DelayFilter::ATTRIBUTE_LAST, 5.0f);
            AM_EXPECT_EQ(instance->GetParameter(DelayFilter::ATTRIBUTE_LAST), 0.0f);

            AM_EXPECT_EQ(filter->GetParameterCount(), 4);

            AM_EXPECT_EQ(filter->GetParameterMin(DelayFilter::ATTRIBUTE_WET), 0.0f);
            AM_EXPECT_EQ(filter->GetParameterMin(DelayFilter::ATTRIBUTE_DELAY), 0.0f);
            AM_EXPECT_EQ(filter->GetParameterMin(DelayFilter::ATTRIBUTE_DECAY), 0.0f);
            AM_EXPECT_EQ(filter->GetParameterMin(DelayFilter::ATTRIBUTE_DELAY_START), 0.0f);
            AM_EXPECT_EQ(filter->GetParameterMin(DelayFilter::ATTRIBUTE_LAST), 0.0f);

            AM_EXPECT_EQ(filter->GetParameterMax(DelayFilter::ATTRIBUTE_WET), 1.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(DelayFilter::ATTRIBUTE_DELAY), 1.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(DelayFilter::ATTRIBUTE_DECAY), 1.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(DelayFilter::ATTRIBUTE_DELAY_START), 1.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(DelayFilter::ATTRIBUTE_LAST), 0.0f);

            AM_EXPECT_EQ(filter->GetParameterType(DelayFilter::ATTRIBUTE_WET), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(DelayFilter::ATTRIBUTE_DELAY), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(DelayFilter::ATTRIBUTE_DECAY), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(DelayFilter::ATTRIBUTE_DELAY_START), eParameterType_Bool);
            AM_EXPECT_EQ(filter->GetParameterType(DelayFilter::ATTRIBUTE_LAST), eParameterType_Float);

            AM_EXPECT_EQ(filter->GetParameterName(DelayFilter::ATTRIBUTE_WET), "Wet");
            AM_EXPECT_EQ(filter->GetParameterName(DelayFilter::ATTRIBUTE_DELAY), "Delay");
            AM_EXPECT_EQ(filter->GetParameterName(DelayFilter::ATTRIBUTE_DECAY), "Decay");
            AM_EXPECT_EQ(filter->GetParameterName(DelayFilter::ATTRIBUTE_DELAY_START), "Delay Start");
            AM_EXPECT_EQ(filter->GetParameterName(DelayFilter::ATTRIBUTE_LAST), "Unknown");
        }
    };

    AM_REGISTER_TEST(dsp_filters, delay_filter_parameters);
} // namespace SparkyStudios::Audio::Amplitude::Tests
