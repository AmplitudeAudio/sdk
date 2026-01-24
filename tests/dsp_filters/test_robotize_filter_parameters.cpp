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

#include <DSP/Filters/RobotizeFilter.h>

#include "SimpleTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, dsp_filters, robotize_filter_parameters)
    {
    public:
        void Run() override
        {
            auto filter = amshared(RobotizeFilter);
            auto result = filter->Initialize(10.0f, RobotizeFilter::WAVE_SQUARE);
            AM_EXPECT_EQ(result, eErrorCode_Success);

            auto instance = filter->CreateInstance();
            AM_EXPECT_NOT(instance == nullptr);

            // Test parameter get/set
            instance->SetParameter(RobotizeFilter::ATTRIBUTE_WET, 0.5f);
            AM_EXPECT_EQ(instance->GetParameter(RobotizeFilter::ATTRIBUTE_WET), 0.5f);

            instance->SetParameter(RobotizeFilter::ATTRIBUTE_FREQUENCY, 20.0f);
            AM_EXPECT_EQ(instance->GetParameter(RobotizeFilter::ATTRIBUTE_FREQUENCY), 20.0f);

            instance->SetParameter(RobotizeFilter::ATTRIBUTE_WAVEFORM, static_cast<AmReal32>(RobotizeFilter::WAVE_SIN));
            AM_EXPECT_EQ(instance->GetParameter(RobotizeFilter::ATTRIBUTE_WAVEFORM), static_cast<AmReal32>(RobotizeFilter::WAVE_SIN));

            // Test with full wet
            instance->SetParameter(RobotizeFilter::ATTRIBUTE_WET, 1.0f);
            AM_EXPECT_EQ(instance->GetParameter(RobotizeFilter::ATTRIBUTE_WET), 1.0f);

            // Test with dry signal
            instance->SetParameter(RobotizeFilter::ATTRIBUTE_WET, 0.0f);
            AM_EXPECT_EQ(instance->GetParameter(RobotizeFilter::ATTRIBUTE_WET), 0.0f);

            // Test with incorrect param
            instance->SetParameter(RobotizeFilter::ATTRIBUTE_LAST, 5.0f);
            AM_EXPECT_EQ(instance->GetParameter(RobotizeFilter::ATTRIBUTE_LAST), 0.0f);

            AM_EXPECT_EQ(filter->GetParameterCount(), 3);

            AM_EXPECT_EQ(filter->GetParameterMin(RobotizeFilter::ATTRIBUTE_WET), 0.0f);
            AM_EXPECT_EQ(filter->GetParameterMin(RobotizeFilter::ATTRIBUTE_FREQUENCY), 0.1f);
            AM_EXPECT_EQ(filter->GetParameterMin(RobotizeFilter::ATTRIBUTE_WAVEFORM), 0.0f);
            AM_EXPECT_EQ(filter->GetParameterMin(RobotizeFilter::ATTRIBUTE_LAST), 0.0f);

            AM_EXPECT_EQ(filter->GetParameterMax(RobotizeFilter::ATTRIBUTE_WET), 1.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(RobotizeFilter::ATTRIBUTE_FREQUENCY), 100.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(RobotizeFilter::ATTRIBUTE_WAVEFORM), static_cast<AmReal32>(RobotizeFilter::WAVE_LAST - 1));
            AM_EXPECT_EQ(filter->GetParameterMax(RobotizeFilter::ATTRIBUTE_LAST), 0.0f);

            AM_EXPECT_EQ(filter->GetParameterType(RobotizeFilter::ATTRIBUTE_WET), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(RobotizeFilter::ATTRIBUTE_FREQUENCY), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(RobotizeFilter::ATTRIBUTE_WAVEFORM), eParameterType_Int);
            AM_EXPECT_EQ(filter->GetParameterType(RobotizeFilter::ATTRIBUTE_LAST), eParameterType_Float);

            AM_EXPECT_EQ(filter->GetParameterName(RobotizeFilter::ATTRIBUTE_WET), "Wet");
            AM_EXPECT_EQ(filter->GetParameterName(RobotizeFilter::ATTRIBUTE_FREQUENCY), "Frequency");
            AM_EXPECT_EQ(filter->GetParameterName(RobotizeFilter::ATTRIBUTE_WAVEFORM), "Waveform");
            AM_EXPECT_EQ(filter->GetParameterName(RobotizeFilter::ATTRIBUTE_LAST), "Unknown");
        }
    };

    AM_REGISTER_TEST(dsp_filters, robotize_filter_parameters);
} // namespace SparkyStudios::Audio::Amplitude::Tests
