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

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, dsp_filters, can_register_and_create_robotize_filter)
    {
    public:
        void Run() override
        {
            auto filter = amshared(RobotizeFilter);
            filter->Initialize(30.0f, RobotizeFilter::WAVE_SIN);

            Filter::Unregister(Filter::Find("Robotize"));
            Filter::Register(filter);

            AM_EXPECT_NOT(Filter::Find("Robotize") == nullptr);
            AM_EXPECT(Filter::Find("Robotize")->GetName() == "Robotize");

            auto instance = Filter::Construct("Robotize");
            AM_EXPECT_NOT(instance == nullptr);

            // Test parameter count
            AM_EXPECT(filter->GetParameterCount() == RobotizeFilter::ATTRIBUTE_LAST);
            AM_EXPECT(filter->GetParameterCount() == 3);

            // Test parameter names
            AM_EXPECT(filter->GetParameterName(RobotizeFilter::ATTRIBUTE_WET) == "Wet");
            AM_EXPECT(filter->GetParameterName(RobotizeFilter::ATTRIBUTE_FREQUENCY) == "Frequency");
            AM_EXPECT(filter->GetParameterName(RobotizeFilter::ATTRIBUTE_WAVEFORM) == "Waveform");

            // Test parameter types
            AM_EXPECT(filter->GetParameterType(RobotizeFilter::ATTRIBUTE_WET) == eParameterType_Float);
            AM_EXPECT(filter->GetParameterType(RobotizeFilter::ATTRIBUTE_FREQUENCY) == eParameterType_Float);
            AM_EXPECT(filter->GetParameterType(RobotizeFilter::ATTRIBUTE_WAVEFORM) == eParameterType_Int);

            // Test WET parameter boundaries
            AM_EXPECT(filter->GetParameterMin(RobotizeFilter::ATTRIBUTE_WET) == 0.0f);
            AM_EXPECT(filter->GetParameterMax(RobotizeFilter::ATTRIBUTE_WET) == 1.0f);

            // Test FREQUENCY parameter boundaries
            AM_EXPECT(filter->GetParameterMin(RobotizeFilter::ATTRIBUTE_FREQUENCY) == 0.1f);
            AM_EXPECT(filter->GetParameterMax(RobotizeFilter::ATTRIBUTE_FREQUENCY) == 100.0f);

            // Test WAVEFORM parameter boundaries
            AM_EXPECT(filter->GetParameterMin(RobotizeFilter::ATTRIBUTE_WAVEFORM) == 0.0f);
            AM_EXPECT(filter->GetParameterMax(RobotizeFilter::ATTRIBUTE_WAVEFORM) == RobotizeFilter::WAVE_LAST - 1);

            Filter::Unregister(filter);
        }
    };

    AM_REGISTER_TEST(dsp_filters, can_register_and_create_robotize_filter);
} // namespace SparkyStudios::Audio::Amplitude::Tests
