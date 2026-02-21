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

#include <DSP/Filters/DCRemovalFilter.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"
#include "SparkyStudios/Audio/Amplitude/Core/Common/Constants.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, dsp_filters, dc_removal_filter_parameters)
    {
    public:
        void Run() override
        {
            auto filter = amshared(DCRemovalFilter);
            auto result = filter->Initialize(-1.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            result = filter->Initialize(0.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            result = filter->Initialize(0.1f);
            AM_EXPECT_EQ(result, eErrorCode_Success);

            auto instance = filter->CreateInstance();
            AM_EXPECT_NOT(instance == nullptr);

            // Test parameter get/set
            instance->SetParameter(DCRemovalFilter::ATTRIBUTE_WET, 0.5f);
            AM_EXPECT_EQ(instance->GetParameter(DCRemovalFilter::ATTRIBUTE_WET), 0.5f);

            instance->SetParameter(DCRemovalFilter::ATTRIBUTE_LENGTH, 0.2f);
            AM_EXPECT_EQ(instance->GetParameter(DCRemovalFilter::ATTRIBUTE_LENGTH), 0.2f);

            // Test with full wet
            instance->SetParameter(DCRemovalFilter::ATTRIBUTE_WET, 1.0f);
            AM_EXPECT_EQ(instance->GetParameter(DCRemovalFilter::ATTRIBUTE_WET), 1.0f);

            // Test with dry signal
            instance->SetParameter(DCRemovalFilter::ATTRIBUTE_WET, 0.0f);
            AM_EXPECT_EQ(instance->GetParameter(DCRemovalFilter::ATTRIBUTE_WET), 0.0f);

            // Test with incorrect param
            instance->SetParameter(DCRemovalFilter::ATTRIBUTE_LAST, 5.0f);
            AM_EXPECT_EQ(instance->GetParameter(DCRemovalFilter::ATTRIBUTE_LAST), 0.0f);

            AM_EXPECT_EQ(filter->GetParameterCount(), 2);

            AM_EXPECT_EQ(filter->GetParameterMin(DCRemovalFilter::ATTRIBUTE_WET), 0.0f);
            AM_EXPECT_EQ(filter->GetParameterMin(DCRemovalFilter::ATTRIBUTE_LENGTH), kEpsilon);
            AM_EXPECT_EQ(filter->GetParameterMin(DCRemovalFilter::ATTRIBUTE_LAST), 0.0f);

            AM_EXPECT_EQ(filter->GetParameterMax(DCRemovalFilter::ATTRIBUTE_WET), 1.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(DCRemovalFilter::ATTRIBUTE_LENGTH), 1.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(DCRemovalFilter::ATTRIBUTE_LAST), 0.0f);

            AM_EXPECT_EQ(filter->GetParameterType(DCRemovalFilter::ATTRIBUTE_WET), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(DCRemovalFilter::ATTRIBUTE_LENGTH), eParameterType_Float);

            AM_EXPECT_EQ(filter->GetParameterName(DCRemovalFilter::ATTRIBUTE_WET), "Wet");
            AM_EXPECT_EQ(filter->GetParameterName(DCRemovalFilter::ATTRIBUTE_LENGTH), "Length");
            AM_EXPECT_EQ(filter->GetParameterName(DCRemovalFilter::ATTRIBUTE_LAST), "Unknown");
        }
    };

    AM_REGISTER_TEST(dsp_filters, dc_removal_filter_parameters);
} // namespace SparkyStudios::Audio::Amplitude::Tests
