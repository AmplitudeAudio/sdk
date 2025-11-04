// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#include <DSP/Filters/MonoPoleFilter.h>

#include "SimpleTestCase.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void SimpleTestCase::Run()
    {
        auto filter = amshared(MonoPoleFilter);
        // Test failure paths - negative coefficient
        auto result = filter->Initialize(-1.0f);
        AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

        // Test failure paths - coefficient > 1.0f
        result = filter->Initialize(2.0f);
        AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

        // Test success path
        result = filter->Initialize(0.5f);
        AM_EXPECT_EQ(result, eErrorCode_Success);

        auto instance = filter->CreateInstance();
        AM_EXPECT_NOT(instance == nullptr);

        // Test parameter get/set
        instance->SetParameter(MonoPoleFilter::ATTRIBUTE_WET, 0.5f);
        AM_EXPECT_EQ(instance->GetParameter(MonoPoleFilter::ATTRIBUTE_WET), 0.5f);

        instance->SetParameter(MonoPoleFilter::ATTRIBUTE_COEFFICIENT, 0.8f);
        AM_EXPECT_EQ(instance->GetParameter(MonoPoleFilter::ATTRIBUTE_COEFFICIENT), 0.8f);

        // Test with full wet
        instance->SetParameter(MonoPoleFilter::ATTRIBUTE_WET, 1.0f);
        AM_EXPECT_EQ(instance->GetParameter(MonoPoleFilter::ATTRIBUTE_WET), 1.0f);

        // Test with dry signal
        instance->SetParameter(MonoPoleFilter::ATTRIBUTE_WET, 0.0f);
        AM_EXPECT_EQ(instance->GetParameter(MonoPoleFilter::ATTRIBUTE_WET), 0.0f);

        // Test with incorrect param
        instance->SetParameter(MonoPoleFilter::ATTRIBUTE_LAST, 5.0f);
        AM_EXPECT_EQ(instance->GetParameter(MonoPoleFilter::ATTRIBUTE_LAST), 0.0f);

        AM_EXPECT_EQ(filter->GetParameterCount(), 2);

        AM_EXPECT_EQ(filter->GetParameterMin(MonoPoleFilter::ATTRIBUTE_WET), 0.0f);
        AM_EXPECT_EQ(filter->GetParameterMin(MonoPoleFilter::ATTRIBUTE_COEFFICIENT), 0.0f);
        AM_EXPECT_EQ(filter->GetParameterMin(MonoPoleFilter::ATTRIBUTE_LAST), 0.0f);

        AM_EXPECT_EQ(filter->GetParameterMax(MonoPoleFilter::ATTRIBUTE_WET), 1.0f);
        AM_EXPECT_EQ(filter->GetParameterMax(MonoPoleFilter::ATTRIBUTE_COEFFICIENT), 1.0f);
        AM_EXPECT_EQ(filter->GetParameterMax(MonoPoleFilter::ATTRIBUTE_LAST), 0.0f);

        AM_EXPECT_EQ(filter->GetParameterType(MonoPoleFilter::ATTRIBUTE_WET), eParameterType_Float);
        AM_EXPECT_EQ(filter->GetParameterType(MonoPoleFilter::ATTRIBUTE_COEFFICIENT), eParameterType_Float);

        AM_EXPECT_EQ(filter->GetParameterName(MonoPoleFilter::ATTRIBUTE_WET), "Wet");
        AM_EXPECT_EQ(filter->GetParameterName(MonoPoleFilter::ATTRIBUTE_COEFFICIENT), "Coefficient");
        AM_EXPECT_EQ(filter->GetParameterName(MonoPoleFilter::ATTRIBUTE_LAST), "Unknown");
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
