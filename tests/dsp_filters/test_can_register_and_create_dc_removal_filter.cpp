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

#include "SimpleTestCase.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void SimpleTestCase::Run()
    {
        auto filter = amshared(DCRemovalFilter);
        filter->Initialize(0.1f);

        Filter::Unregister(Filter::Find("DCRemoval"));
        Filter::Register(filter);

        AM_EXPECT_NOT(Filter::Find("DCRemoval") == nullptr);
        AM_EXPECT(Filter::Find("DCRemoval")->GetName() == "DCRemoval");

        auto instance = Filter::Construct("DCRemoval");
        AM_EXPECT_NOT(instance == nullptr);

        // Test parameter count
        AM_EXPECT(filter->GetParameterCount() == DCRemovalFilter::ATTRIBUTE_LAST);
        AM_EXPECT(filter->GetParameterCount() == 2);

        // Test parameter names
        AM_EXPECT(filter->GetParameterName(DCRemovalFilter::ATTRIBUTE_WET) == "Wet");
        AM_EXPECT(filter->GetParameterName(DCRemovalFilter::ATTRIBUTE_LENGTH) == "Length");

        // Test parameter types
        AM_EXPECT(filter->GetParameterType(DCRemovalFilter::ATTRIBUTE_WET) == eParameterType_Float);
        AM_EXPECT(filter->GetParameterType(DCRemovalFilter::ATTRIBUTE_LENGTH) == eParameterType_Float);

        // Test WET parameter boundaries
        AM_EXPECT(filter->GetParameterMin(DCRemovalFilter::ATTRIBUTE_WET) == 0.0f);
        AM_EXPECT(filter->GetParameterMax(DCRemovalFilter::ATTRIBUTE_WET) == 1.0f);

        // Test LENGTH parameter boundaries
        AM_EXPECT(filter->GetParameterMin(DCRemovalFilter::ATTRIBUTE_LENGTH) == kEpsilon);
        AM_EXPECT(filter->GetParameterMax(DCRemovalFilter::ATTRIBUTE_LENGTH) == 1.0f);

        Filter::Unregister(filter);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
