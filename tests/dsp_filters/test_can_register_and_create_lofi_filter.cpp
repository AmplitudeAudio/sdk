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

#include <DSP/Filters/LofiFilter.h>

#include "SimpleTestCase.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void SimpleTestCase::Run()
    {
        auto filter = amshared(LofiFilter);
        filter->Initialize(8000.0f, 8.0f);

        Filter::Unregister(Filter::Find("Lofi"));
        Filter::Register(filter);

        AM_EXPECT_NOT(Filter::Find("Lofi") == nullptr);
        AM_EXPECT(Filter::Find("Lofi")->GetName() == "Lofi");

        auto instance = Filter::Construct("Lofi");
        AM_EXPECT_NOT(instance == nullptr);

        // Test parameter count
        AM_EXPECT(filter->GetParameterCount() == LofiFilter::ATTRIBUTE_LAST);
        AM_EXPECT(filter->GetParameterCount() == 3);

        // Test parameter names
        AM_EXPECT(filter->GetParameterName(LofiFilter::ATTRIBUTE_WET) == "Wet");
        AM_EXPECT(filter->GetParameterName(LofiFilter::ATTRIBUTE_SAMPLERATE) == "Samplerate");
        AM_EXPECT(filter->GetParameterName(LofiFilter::ATTRIBUTE_BITDEPTH) == "Bit Depth");
        AM_EXPECT(filter->GetParameterName(LofiFilter::ATTRIBUTE_LAST) == "Unknown");

        // Test parameter types
        AM_EXPECT(filter->GetParameterType(LofiFilter::ATTRIBUTE_WET) == eParameterType_Float);
        AM_EXPECT(filter->GetParameterType(LofiFilter::ATTRIBUTE_SAMPLERATE) == eParameterType_Float);
        AM_EXPECT(filter->GetParameterType(LofiFilter::ATTRIBUTE_BITDEPTH) == eParameterType_Float);

        // Test WET parameter boundaries
        AM_EXPECT(filter->GetParameterMin(LofiFilter::ATTRIBUTE_WET) == 0.0f);
        AM_EXPECT(filter->GetParameterMax(LofiFilter::ATTRIBUTE_WET) == 1.0f);

        // Test SAMPLERATE parameter boundaries
        AM_EXPECT(filter->GetParameterMin(LofiFilter::ATTRIBUTE_SAMPLERATE) == 100.0f);
        AM_EXPECT(filter->GetParameterMax(LofiFilter::ATTRIBUTE_SAMPLERATE) == 22000.0f);

        // Test BITDEPTH parameter boundaries
        AM_EXPECT(filter->GetParameterMin(LofiFilter::ATTRIBUTE_BITDEPTH) == 0.5f);
        AM_EXPECT(filter->GetParameterMax(LofiFilter::ATTRIBUTE_BITDEPTH) == 16.0f);

        Filter::Unregister(filter);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
