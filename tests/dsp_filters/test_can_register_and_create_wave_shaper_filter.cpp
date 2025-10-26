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

#include <DSP/Filters/WaveShaperFilter.h>

#include "SimpleTestCase.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void SimpleTestCase::Run()
    {
        auto filter = amshared(WaveShaperFilter);
        filter->Init(5.0f);

        Filter::Unregister(Filter::Find("WaveShaper"));
        Filter::Register(filter);

        AM_EXPECT_NOT(Filter::Find("WaveShaper") == nullptr);
        AM_EXPECT(Filter::Find("WaveShaper")->GetName() == "WaveShaper");

        auto instance = Filter::Construct("WaveShaper");
        AM_EXPECT_NOT(instance == nullptr);

        // Test parameter count
        AM_EXPECT(filter->GetParameterCount() == WaveShaperFilter::ATTRIBUTE_LAST);
        AM_EXPECT(filter->GetParameterCount() == 2);

        // Test parameter names
        AM_EXPECT(filter->GetParameterName(WaveShaperFilter::ATTRIBUTE_WET) == "Wet");
        AM_EXPECT(filter->GetParameterName(WaveShaperFilter::ATTRIBUTE_AMOUNT) == "Amount");

        // Test parameter types
        AM_EXPECT(filter->GetParameterType(WaveShaperFilter::ATTRIBUTE_WET) == eParameterType_Float);
        AM_EXPECT(filter->GetParameterType(WaveShaperFilter::ATTRIBUTE_AMOUNT) == eParameterType_Float);

        // Test WET parameter boundaries
        AM_EXPECT(filter->GetParameterMin(WaveShaperFilter::ATTRIBUTE_WET) == 0.0f);
        AM_EXPECT(filter->GetParameterMax(WaveShaperFilter::ATTRIBUTE_WET) == 1.0f);

        // Test AMOUNT parameter boundaries
        AM_EXPECT(filter->GetParameterMin(WaveShaperFilter::ATTRIBUTE_AMOUNT) == -1.0f);
        AM_EXPECT(filter->GetParameterMax(WaveShaperFilter::ATTRIBUTE_AMOUNT) == 1.0f);

        Filter::Unregister(filter);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
