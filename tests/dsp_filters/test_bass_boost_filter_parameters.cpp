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

#include <DSP/Filters/BassBoostFilter.h>

#include "SimpleTestCase.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void SimpleTestCase::Run()
    {
        auto filter = amshared(BassBoostFilter);
        auto result = filter->Initialize(-1.0f);
        AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

        result = filter->Initialize(2.0f);
        AM_EXPECT_EQ(result, eErrorCode_Success);

        auto instance = filter->CreateInstance();
        AM_EXPECT_NOT(instance == nullptr);

        // Test parameter get/set
        instance->SetParameter(BassBoostFilter::ATTRIBUTE_WET, 0.5f);
        AM_EXPECT_EQ(instance->GetParameter(BassBoostFilter::ATTRIBUTE_WET), 0.5f);

        instance->SetParameter(BassBoostFilter::ATTRIBUTE_BOOST, 5.0f);
        AM_EXPECT_EQ(instance->GetParameter(BassBoostFilter::ATTRIBUTE_BOOST), 5.0f);

        // Test with full wet
        instance->SetParameter(BassBoostFilter::ATTRIBUTE_WET, 1.0f);
        AM_EXPECT_EQ(instance->GetParameter(BassBoostFilter::ATTRIBUTE_WET), 1.0f);

        // Test with dry signal
        instance->SetParameter(BassBoostFilter::ATTRIBUTE_WET, 0.0f);
        AM_EXPECT_EQ(instance->GetParameter(BassBoostFilter::ATTRIBUTE_WET), 0.0f);

        // Test with incorrect param
        instance->SetParameter(BassBoostFilter::ATTRIBUTE_LAST, 5.0f);
        AM_EXPECT_EQ(instance->GetParameter(BassBoostFilter::ATTRIBUTE_LAST), 0.0f);

        AM_EXPECT_EQ(filter->GetParameterCount(), 2);

        AM_EXPECT_EQ(filter->GetParameterMin(BassBoostFilter::ATTRIBUTE_WET), 0.0f);
        AM_EXPECT_EQ(filter->GetParameterMin(BassBoostFilter::ATTRIBUTE_BOOST), 0.0f);

        AM_EXPECT_EQ(filter->GetParameterMax(BassBoostFilter::ATTRIBUTE_WET), 1.0f);
        AM_EXPECT_EQ(filter->GetParameterMax(BassBoostFilter::ATTRIBUTE_BOOST), 10.0f);

        AM_EXPECT_EQ(filter->GetParameterType(BassBoostFilter::ATTRIBUTE_WET), eParameterType_Float);
        AM_EXPECT_EQ(filter->GetParameterType(BassBoostFilter::ATTRIBUTE_BOOST), eParameterType_Float);

        AM_EXPECT_EQ(filter->GetParameterName(BassBoostFilter::ATTRIBUTE_WET), "Wet");
        AM_EXPECT_EQ(filter->GetParameterName(BassBoostFilter::ATTRIBUTE_BOOST), "Boost");
        AM_EXPECT_EQ(filter->GetParameterName(BassBoostFilter::ATTRIBUTE_LAST), "Unknown");
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
