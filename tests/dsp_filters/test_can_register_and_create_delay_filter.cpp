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

#include <DSP/Filters/DelayFilter.h>

#include "SimpleTestCase.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void SimpleTestCase::Run()
    {
        auto filter = amshared(DelayFilter);
        filter->Initialize(0.3f, 0.7f);

        Filter::Unregister(Filter::Find("Delay"));
        Filter::Register(filter);

        AM_EXPECT_NOT(Filter::Find("Delay") == nullptr);
        AM_EXPECT(Filter::Find("Delay")->GetName() == "Delay");

        auto instance = Filter::Construct("Delay");
        AM_EXPECT_NOT(instance == nullptr);

        // Test parameter count
        AM_EXPECT(filter->GetParameterCount() == DelayFilter::ATTRIBUTE_LAST);
        AM_EXPECT(filter->GetParameterCount() == 4);

        // Test parameter names
        AM_EXPECT(filter->GetParameterName(DelayFilter::ATTRIBUTE_WET) == "Wet");
        AM_EXPECT(filter->GetParameterName(DelayFilter::ATTRIBUTE_DELAY) == "Delay");
        AM_EXPECT(filter->GetParameterName(DelayFilter::ATTRIBUTE_DECAY) == "Decay");
        AM_EXPECT(filter->GetParameterName(DelayFilter::ATTRIBUTE_DELAY_START) == "DelayStart");

        // Test parameter types
        AM_EXPECT(filter->GetParameterType(DelayFilter::ATTRIBUTE_WET) == eParameterType_Float);
        AM_EXPECT(filter->GetParameterType(DelayFilter::ATTRIBUTE_DELAY) == eParameterType_Float);
        AM_EXPECT(filter->GetParameterType(DelayFilter::ATTRIBUTE_DECAY) == eParameterType_Float);
        AM_EXPECT(filter->GetParameterType(DelayFilter::ATTRIBUTE_DELAY_START) == eParameterType_Bool);

        // Test WET parameter boundaries
        AM_EXPECT(filter->GetParameterMin(DelayFilter::ATTRIBUTE_WET) == 0.0f);
        AM_EXPECT(filter->GetParameterMax(DelayFilter::ATTRIBUTE_WET) == 1.0f);

        // Test DELAY parameter boundaries
        AM_EXPECT(filter->GetParameterMin(DelayFilter::ATTRIBUTE_DELAY) == 0.0f);
        AM_EXPECT(filter->GetParameterMax(DelayFilter::ATTRIBUTE_DELAY) == 1.0f);

        // Test DECAY parameter boundaries
        AM_EXPECT(filter->GetParameterMin(DelayFilter::ATTRIBUTE_DECAY) == 0.0f);
        AM_EXPECT(filter->GetParameterMax(DelayFilter::ATTRIBUTE_DECAY) == 1.0f);

        // Test DELAY_START parameter boundaries
        AM_EXPECT(filter->GetParameterMin(DelayFilter::ATTRIBUTE_DELAY_START) == 0.0f);
        AM_EXPECT(filter->GetParameterMax(DelayFilter::ATTRIBUTE_DELAY_START) == 1.0f);

        Filter::Unregister(filter);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
