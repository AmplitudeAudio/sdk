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

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void SimpleTestCase::Run()
    {
        auto filter = amshared(EqualizerFilter);
        filter->Init(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

        Filter::Unregister(Filter::Find("Equalizer"));
        Filter::Register(filter);

        AM_EXPECT_NOT(Filter::Find("Equalizer") == nullptr);
        AM_EXPECT(Filter::Find("Equalizer")->GetName() == "Equalizer");

        auto instance = Filter::Construct("Equalizer");
        AM_EXPECT_NOT(instance == nullptr);

        // Test parameter count
        AM_EXPECT(filter->GetParameterCount() == EqualizerFilter::ATTRIBUTE_LAST);
        AM_EXPECT(filter->GetParameterCount() == 9);

        // Test parameter names
        AM_EXPECT(filter->GetParameterName(EqualizerFilter::ATTRIBUTE_WET) == "Wet");
        AM_EXPECT(filter->GetParameterName(EqualizerFilter::ATTRIBUTE_BAND_1) == "Band 1");
        AM_EXPECT(filter->GetParameterName(EqualizerFilter::ATTRIBUTE_BAND_2) == "Band 2");
        AM_EXPECT(filter->GetParameterName(EqualizerFilter::ATTRIBUTE_BAND_3) == "Band 3");
        AM_EXPECT(filter->GetParameterName(EqualizerFilter::ATTRIBUTE_BAND_4) == "Band 4");
        AM_EXPECT(filter->GetParameterName(EqualizerFilter::ATTRIBUTE_BAND_5) == "Band 5");
        AM_EXPECT(filter->GetParameterName(EqualizerFilter::ATTRIBUTE_BAND_6) == "Band 6");
        AM_EXPECT(filter->GetParameterName(EqualizerFilter::ATTRIBUTE_BAND_7) == "Band 7");
        AM_EXPECT(filter->GetParameterName(EqualizerFilter::ATTRIBUTE_BAND_8) == "Band 8");

        // Test parameter types
        AM_EXPECT(filter->GetParameterType(EqualizerFilter::ATTRIBUTE_WET) == eParameterType_Float);
        AM_EXPECT(filter->GetParameterType(EqualizerFilter::ATTRIBUTE_BAND_1) == eParameterType_Float);
        AM_EXPECT(filter->GetParameterType(EqualizerFilter::ATTRIBUTE_BAND_8) == eParameterType_Float);

        // Test WET parameter boundaries
        AM_EXPECT(filter->GetParameterMin(EqualizerFilter::ATTRIBUTE_WET) == 0.0f);
        AM_EXPECT(filter->GetParameterMax(EqualizerFilter::ATTRIBUTE_WET) == 1.0f);

        // Test BAND parameters boundaries (all bands have same range)
        for (AmUInt32 i = EqualizerFilter::ATTRIBUTE_BAND_1; i <= EqualizerFilter::ATTRIBUTE_BAND_8; ++i)
        {
            AM_EXPECT(filter->GetParameterMin(i) == 0.0f);
            AM_EXPECT(filter->GetParameterMax(i) == 4.0f);
        }

        Filter::Unregister(filter);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
