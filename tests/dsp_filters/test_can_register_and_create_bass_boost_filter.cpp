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
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, dsp_filters, can_register_and_create_bass_boost_filter)
    {
    public:
        void Run() override
        {
            auto filter = amshared(BassBoostFilter);
            filter->Initialize(2.0f);

            Filter::Unregister(Filter::Find("BassBoost"));
            Filter::Register(filter);

            AM_EXPECT_NOT(Filter::Find("BassBoost") == nullptr);
            AM_EXPECT(Filter::Find("BassBoost")->GetName() == "BassBoost");

            auto instance = Filter::Construct("BassBoost");
            AM_EXPECT_NOT(instance == nullptr);

            // Test parameter count
            AM_EXPECT(filter->GetParameterCount() == BassBoostFilter::ATTRIBUTE_LAST);
            AM_EXPECT(filter->GetParameterCount() == 2);

            // Test parameter names
            AM_EXPECT(filter->GetParameterName(BassBoostFilter::ATTRIBUTE_WET) == "Wet");
            AM_EXPECT(filter->GetParameterName(BassBoostFilter::ATTRIBUTE_BOOST) == "Boost");

            // Test parameter types
            AM_EXPECT(filter->GetParameterType(BassBoostFilter::ATTRIBUTE_WET) == eParameterType_Float);
            AM_EXPECT(filter->GetParameterType(BassBoostFilter::ATTRIBUTE_BOOST) == eParameterType_Float);

            // Test WET parameter boundaries
            AM_EXPECT(filter->GetParameterMin(BassBoostFilter::ATTRIBUTE_WET) == 0.0f);
            AM_EXPECT(filter->GetParameterMax(BassBoostFilter::ATTRIBUTE_WET) == 1.0f);

            // Test BOOST parameter boundaries
            AM_EXPECT(filter->GetParameterMin(BassBoostFilter::ATTRIBUTE_BOOST) == 0.0f);
            AM_EXPECT(filter->GetParameterMax(BassBoostFilter::ATTRIBUTE_BOOST) == 10.0f);

            Filter::Unregister(filter);
        }
    };

    AM_REGISTER_TEST(dsp_filters, can_register_and_create_bass_boost_filter);
} // namespace SparkyStudios::Audio::Amplitude::Tests
