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

#include <DSP/Filters/FlangerFilter.h>

#include "SimpleTestCase.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void SimpleTestCase::Run()
    {
        auto filter = amshared(FlangerFilter);
        // Test failure paths - negative delay
        auto result = filter->Initialize(-1.0f, 1.0f);
        AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

        // Test failure paths - zero delay
        result = filter->Initialize(0.0f, 1.0f);
        AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

        // Test failure paths - negative frequency
        result = filter->Initialize(0.01f, -1.0f);
        AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

        // Test failure paths - zero frequency
        result = filter->Initialize(0.01f, 0.0f);
        AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

        // Test success path
        result = filter->Initialize(0.01f, 1.0f);
        AM_EXPECT_EQ(result, eErrorCode_Success);

        auto instance = filter->CreateInstance();
        AM_EXPECT_NOT(instance == nullptr);

        // Test parameter get/set
        instance->SetParameter(FlangerFilter::ATTRIBUTE_WET, 0.5f);
        AM_EXPECT_EQ(instance->GetParameter(FlangerFilter::ATTRIBUTE_WET), 0.5f);

        instance->SetParameter(FlangerFilter::ATTRIBUTE_DELAY, 0.02f);
        AM_EXPECT_EQ(instance->GetParameter(FlangerFilter::ATTRIBUTE_DELAY), 0.02f);

        instance->SetParameter(FlangerFilter::ATTRIBUTE_FREQUENCY, 2.0f);
        AM_EXPECT_EQ(instance->GetParameter(FlangerFilter::ATTRIBUTE_FREQUENCY), 2.0f);

        // Test with full wet
        instance->SetParameter(FlangerFilter::ATTRIBUTE_WET, 1.0f);
        AM_EXPECT_EQ(instance->GetParameter(FlangerFilter::ATTRIBUTE_WET), 1.0f);

        // Test with dry signal
        instance->SetParameter(FlangerFilter::ATTRIBUTE_WET, 0.0f);
        AM_EXPECT_EQ(instance->GetParameter(FlangerFilter::ATTRIBUTE_WET), 0.0f);

        // Test with incorrect param
        instance->SetParameter(FlangerFilter::ATTRIBUTE_LAST, 5.0f);
        AM_EXPECT_EQ(instance->GetParameter(FlangerFilter::ATTRIBUTE_LAST), 0.0f);

        AM_EXPECT_EQ(filter->GetParameterCount(), 3);

        AM_EXPECT_EQ(filter->GetParameterMin(FlangerFilter::ATTRIBUTE_WET), 0.0f);
        AM_EXPECT_EQ(filter->GetParameterMin(FlangerFilter::ATTRIBUTE_DELAY), 0.001f);
        AM_EXPECT_EQ(filter->GetParameterMin(FlangerFilter::ATTRIBUTE_FREQUENCY), 0.1f);
        AM_EXPECT_EQ(filter->GetParameterMin(FlangerFilter::ATTRIBUTE_LAST), 0.0f);

        AM_EXPECT_EQ(filter->GetParameterMax(FlangerFilter::ATTRIBUTE_WET), 1.0f);
        AM_EXPECT_EQ(filter->GetParameterMax(FlangerFilter::ATTRIBUTE_DELAY), 0.1f);
        AM_EXPECT_EQ(filter->GetParameterMax(FlangerFilter::ATTRIBUTE_FREQUENCY), 100.0f);
        AM_EXPECT_EQ(filter->GetParameterMax(FlangerFilter::ATTRIBUTE_LAST), 0.0f);

        AM_EXPECT_EQ(filter->GetParameterType(FlangerFilter::ATTRIBUTE_WET), eParameterType_Float);
        AM_EXPECT_EQ(filter->GetParameterType(FlangerFilter::ATTRIBUTE_DELAY), eParameterType_Float);
        AM_EXPECT_EQ(filter->GetParameterType(FlangerFilter::ATTRIBUTE_FREQUENCY), eParameterType_Float);
        AM_EXPECT_EQ(filter->GetParameterType(FlangerFilter::ATTRIBUTE_LAST), eParameterType_Float);

        AM_EXPECT_EQ(filter->GetParameterName(FlangerFilter::ATTRIBUTE_WET), "Wet");
        AM_EXPECT_EQ(filter->GetParameterName(FlangerFilter::ATTRIBUTE_DELAY), "Delay");
        AM_EXPECT_EQ(filter->GetParameterName(FlangerFilter::ATTRIBUTE_FREQUENCY), "Frequency");
        AM_EXPECT_EQ(filter->GetParameterName(FlangerFilter::ATTRIBUTE_LAST), "Unknown");
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
