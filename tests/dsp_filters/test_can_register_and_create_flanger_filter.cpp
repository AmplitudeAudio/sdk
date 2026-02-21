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

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, dsp_filters, can_register_and_create_flanger_filter)
    {
    public:
        void Run() override
        {
            auto filter = amshared(FlangerFilter);
            filter->Initialize(0.005f, 10.0f);

            Filter::Unregister(Filter::Find("Flanger"));
            Filter::Register(filter);

            AM_EXPECT_NOT(Filter::Find("Flanger") == nullptr);
            AM_EXPECT(Filter::Find("Flanger")->GetName() == "Flanger");

            auto instance = Filter::Construct("Flanger");
            AM_EXPECT_NOT(instance == nullptr);

            // Test parameter count
            AM_EXPECT(filter->GetParameterCount() == FlangerFilter::ATTRIBUTE_LAST);
            AM_EXPECT(filter->GetParameterCount() == 3);

            // Test parameter names
            AM_EXPECT(filter->GetParameterName(FlangerFilter::ATTRIBUTE_WET) == "Wet");
            AM_EXPECT(filter->GetParameterName(FlangerFilter::ATTRIBUTE_DELAY) == "Delay");
            AM_EXPECT(filter->GetParameterName(FlangerFilter::ATTRIBUTE_FREQUENCY) == "Frequency");

            // Test parameter types
            AM_EXPECT(filter->GetParameterType(FlangerFilter::ATTRIBUTE_WET) == eParameterType_Float);
            AM_EXPECT(filter->GetParameterType(FlangerFilter::ATTRIBUTE_DELAY) == eParameterType_Float);
            AM_EXPECT(filter->GetParameterType(FlangerFilter::ATTRIBUTE_FREQUENCY) == eParameterType_Float);

            // Test WET parameter boundaries
            AM_EXPECT(filter->GetParameterMin(FlangerFilter::ATTRIBUTE_WET) == 0.0f);
            AM_EXPECT(filter->GetParameterMax(FlangerFilter::ATTRIBUTE_WET) == 1.0f);

            // Test DELAY parameter boundaries
            AM_EXPECT(filter->GetParameterMin(FlangerFilter::ATTRIBUTE_DELAY) == 0.001f);
            AM_EXPECT(filter->GetParameterMax(FlangerFilter::ATTRIBUTE_DELAY) == 0.1f);

            // Test FREQUENCY parameter boundaries
            AM_EXPECT(filter->GetParameterMin(FlangerFilter::ATTRIBUTE_FREQUENCY) == 0.1f);
            AM_EXPECT(filter->GetParameterMax(FlangerFilter::ATTRIBUTE_FREQUENCY) == 100.0f);

            Filter::Unregister(filter);
        }
    };

    AM_REGISTER_TEST(dsp_filters, can_register_and_create_flanger_filter);
} // namespace SparkyStudios::Audio::Amplitude::Tests
