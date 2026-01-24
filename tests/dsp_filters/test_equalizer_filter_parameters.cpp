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

#include <DSP/Filters/EqualizerFilter.h>

#include "SimpleTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, dsp_filters, equalizer_filter_parameters)
    {
    public:
        void Run() override
        {
            auto filter = amshared(EqualizerFilter);

            // Test failure paths - values below minimum (< 0.0f)
            auto result = filter->Initialize(-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            result = filter->Initialize(0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            result = filter->Initialize(0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            result = filter->Initialize(0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            result = filter->Initialize(0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            result = filter->Initialize(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            result = filter->Initialize(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            result = filter->Initialize(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            // Test failure paths - values above maximum (> 4.0f)
            result = filter->Initialize(5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            result = filter->Initialize(0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            result = filter->Initialize(0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            result = filter->Initialize(0.0f, 0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            result = filter->Initialize(0.0f, 0.0f, 0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            result = filter->Initialize(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 5.0f, 0.0f, 0.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            result = filter->Initialize(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 5.0f, 0.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            result = filter->Initialize(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 5.0f);
            AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

            // Test success path
            result = filter->Initialize(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
            AM_EXPECT_EQ(result, eErrorCode_Success);

            auto instance = filter->CreateInstance();
            AM_EXPECT_NOT(instance == nullptr);

            // Test parameter get/set
            instance->SetParameter(EqualizerFilter::ATTRIBUTE_WET, 0.5f);
            AM_EXPECT_EQ(instance->GetParameter(EqualizerFilter::ATTRIBUTE_WET), 0.5f);

            instance->SetParameter(EqualizerFilter::ATTRIBUTE_BAND_1, 2.0f);
            AM_EXPECT_EQ(instance->GetParameter(EqualizerFilter::ATTRIBUTE_BAND_1), 2.0f);

            instance->SetParameter(EqualizerFilter::ATTRIBUTE_BAND_2, -1.5f);
            AM_EXPECT_EQ(instance->GetParameter(EqualizerFilter::ATTRIBUTE_BAND_2), -1.5f);

            instance->SetParameter(EqualizerFilter::ATTRIBUTE_BAND_3, 3.0f);
            AM_EXPECT_EQ(instance->GetParameter(EqualizerFilter::ATTRIBUTE_BAND_3), 3.0f);

            instance->SetParameter(EqualizerFilter::ATTRIBUTE_BAND_4, -2.0f);
            AM_EXPECT_EQ(instance->GetParameter(EqualizerFilter::ATTRIBUTE_BAND_4), -2.0f);

            instance->SetParameter(EqualizerFilter::ATTRIBUTE_BAND_5, 1.0f);
            AM_EXPECT_EQ(instance->GetParameter(EqualizerFilter::ATTRIBUTE_BAND_5), 1.0f);

            instance->SetParameter(EqualizerFilter::ATTRIBUTE_BAND_6, -0.5f);
            AM_EXPECT_EQ(instance->GetParameter(EqualizerFilter::ATTRIBUTE_BAND_6), -0.5f);

            instance->SetParameter(EqualizerFilter::ATTRIBUTE_BAND_7, 2.5f);
            AM_EXPECT_EQ(instance->GetParameter(EqualizerFilter::ATTRIBUTE_BAND_7), 2.5f);

            instance->SetParameter(EqualizerFilter::ATTRIBUTE_BAND_8, -3.0f);
            AM_EXPECT_EQ(instance->GetParameter(EqualizerFilter::ATTRIBUTE_BAND_8), -3.0f);

            // Test with full wet
            instance->SetParameter(EqualizerFilter::ATTRIBUTE_WET, 1.0f);
            AM_EXPECT_EQ(instance->GetParameter(EqualizerFilter::ATTRIBUTE_WET), 1.0f);

            // Test with dry signal
            instance->SetParameter(EqualizerFilter::ATTRIBUTE_WET, 0.0f);
            AM_EXPECT_EQ(instance->GetParameter(EqualizerFilter::ATTRIBUTE_WET), 0.0f);

            // Test with incorrect param
            instance->SetParameter(EqualizerFilter::ATTRIBUTE_LAST, 5.0f);
            AM_EXPECT_EQ(instance->GetParameter(EqualizerFilter::ATTRIBUTE_LAST), 0.0f);

            AM_EXPECT_EQ(filter->GetParameterCount(), 9);

            AM_EXPECT_EQ(filter->GetParameterMin(EqualizerFilter::ATTRIBUTE_WET), 0.0f);
            AM_EXPECT_EQ(filter->GetParameterMin(EqualizerFilter::ATTRIBUTE_BAND_1), 0.0f);
            AM_EXPECT_EQ(filter->GetParameterMin(EqualizerFilter::ATTRIBUTE_BAND_2), 0.0f);
            AM_EXPECT_EQ(filter->GetParameterMin(EqualizerFilter::ATTRIBUTE_BAND_3), 0.0f);
            AM_EXPECT_EQ(filter->GetParameterMin(EqualizerFilter::ATTRIBUTE_BAND_4), 0.0f);
            AM_EXPECT_EQ(filter->GetParameterMin(EqualizerFilter::ATTRIBUTE_BAND_5), 0.0f);
            AM_EXPECT_EQ(filter->GetParameterMin(EqualizerFilter::ATTRIBUTE_BAND_6), 0.0f);
            AM_EXPECT_EQ(filter->GetParameterMin(EqualizerFilter::ATTRIBUTE_BAND_7), 0.0f);
            AM_EXPECT_EQ(filter->GetParameterMin(EqualizerFilter::ATTRIBUTE_BAND_8), 0.0f);
            AM_EXPECT_EQ(filter->GetParameterMin(EqualizerFilter::ATTRIBUTE_LAST), 0.0f);

            AM_EXPECT_EQ(filter->GetParameterMax(EqualizerFilter::ATTRIBUTE_WET), 1.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(EqualizerFilter::ATTRIBUTE_BAND_1), 4.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(EqualizerFilter::ATTRIBUTE_BAND_2), 4.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(EqualizerFilter::ATTRIBUTE_BAND_3), 4.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(EqualizerFilter::ATTRIBUTE_BAND_4), 4.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(EqualizerFilter::ATTRIBUTE_BAND_5), 4.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(EqualizerFilter::ATTRIBUTE_BAND_6), 4.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(EqualizerFilter::ATTRIBUTE_BAND_7), 4.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(EqualizerFilter::ATTRIBUTE_BAND_8), 4.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(EqualizerFilter::ATTRIBUTE_LAST), 0.0f);

            AM_EXPECT_EQ(filter->GetParameterType(EqualizerFilter::ATTRIBUTE_WET), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(EqualizerFilter::ATTRIBUTE_BAND_1), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(EqualizerFilter::ATTRIBUTE_BAND_2), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(EqualizerFilter::ATTRIBUTE_BAND_3), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(EqualizerFilter::ATTRIBUTE_BAND_4), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(EqualizerFilter::ATTRIBUTE_BAND_5), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(EqualizerFilter::ATTRIBUTE_BAND_6), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(EqualizerFilter::ATTRIBUTE_BAND_7), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(EqualizerFilter::ATTRIBUTE_BAND_8), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(EqualizerFilter::ATTRIBUTE_LAST), eParameterType_Float);

            AM_EXPECT_EQ(filter->GetParameterName(EqualizerFilter::ATTRIBUTE_WET), "Wet");
            AM_EXPECT_EQ(filter->GetParameterName(EqualizerFilter::ATTRIBUTE_BAND_1), "Band 1");
            AM_EXPECT_EQ(filter->GetParameterName(EqualizerFilter::ATTRIBUTE_BAND_2), "Band 2");
            AM_EXPECT_EQ(filter->GetParameterName(EqualizerFilter::ATTRIBUTE_BAND_3), "Band 3");
            AM_EXPECT_EQ(filter->GetParameterName(EqualizerFilter::ATTRIBUTE_BAND_4), "Band 4");
            AM_EXPECT_EQ(filter->GetParameterName(EqualizerFilter::ATTRIBUTE_BAND_5), "Band 5");
            AM_EXPECT_EQ(filter->GetParameterName(EqualizerFilter::ATTRIBUTE_BAND_6), "Band 6");
            AM_EXPECT_EQ(filter->GetParameterName(EqualizerFilter::ATTRIBUTE_BAND_7), "Band 7");
            AM_EXPECT_EQ(filter->GetParameterName(EqualizerFilter::ATTRIBUTE_BAND_8), "Band 8");
            AM_EXPECT_EQ(filter->GetParameterName(EqualizerFilter::ATTRIBUTE_LAST), "Unknown");
        }
    };

    AM_REGISTER_TEST(dsp_filters, equalizer_filter_parameters);
} // namespace SparkyStudios::Audio::Amplitude::Tests
