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

#include <DSP/Filters/MonoPoleFilter.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, dsp_filters, can_register_and_create_monopole_filter)
    {
    public:
        void Run() override
        {
            auto filter = amshared(MonoPoleFilter);
            filter->Initialize(0.5f);

            Filter::Unregister(Filter::Find("MonoPole"));
            Filter::Register(filter);

            AM_EXPECT_NOT(Filter::Find("MonoPole") == nullptr);
            AM_EXPECT(Filter::Find("MonoPole")->GetName() == "MonoPole");

            auto instance = Filter::Construct("MonoPole");
            AM_EXPECT_NOT(instance == nullptr);

            // Test parameter count
            AM_EXPECT(filter->GetParameterCount() == MonoPoleFilter::ATTRIBUTE_LAST);
            AM_EXPECT(filter->GetParameterCount() == 2);

            // Test parameter names
            AM_EXPECT(filter->GetParameterName(MonoPoleFilter::ATTRIBUTE_WET) == "Wet");
            AM_EXPECT(filter->GetParameterName(MonoPoleFilter::ATTRIBUTE_COEFFICIENT) == "Coefficient");

            // Test parameter types
            AM_EXPECT(filter->GetParameterType(MonoPoleFilter::ATTRIBUTE_WET) == eParameterType_Float);
            AM_EXPECT(filter->GetParameterType(MonoPoleFilter::ATTRIBUTE_COEFFICIENT) == eParameterType_Float);

            // Test WET parameter boundaries
            AM_EXPECT(filter->GetParameterMin(MonoPoleFilter::ATTRIBUTE_WET) == 0.0f);
            AM_EXPECT(filter->GetParameterMax(MonoPoleFilter::ATTRIBUTE_WET) == 1.0f);

            // Test COEFFICIENT parameter boundaries
            AM_EXPECT(filter->GetParameterMin(MonoPoleFilter::ATTRIBUTE_COEFFICIENT) == 0.0f);
            AM_EXPECT(filter->GetParameterMax(MonoPoleFilter::ATTRIBUTE_COEFFICIENT) == 1.0f);

            Filter::Unregister(filter);
        }
    };

    AM_REGISTER_TEST(dsp_filters, can_register_and_create_monopole_filter);
} // namespace SparkyStudios::Audio::Amplitude::Tests
