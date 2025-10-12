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

#include <DSP/Filters/BiquadResonantFilter.h>

#include "SimpleTestCase.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void SimpleTestCase::Run()
    {
        auto filter = AmSharedPtr<BiquadResonantFilter>::Make();
        filter->InitializeLowPass(1000.0f, 0.707107f);

        Filter::Unregister(Filter::Find("BiquadResonant"));
        Filter::Register(filter);

        AM_EXPECT_NOT(Filter::Find("BiquadResonant") == nullptr);
        AM_EXPECT(Filter::Find("BiquadResonant")->GetName() == "BiquadResonant");

        auto instance = Filter::Construct("BiquadResonant");
        AM_EXPECT_NOT(instance == nullptr);

        // Test parameter count
        AM_EXPECT(filter->GetParameterCount() == BiquadResonantFilter::ATTRIBUTE_LAST);
        AM_EXPECT(filter->GetParameterCount() == 5);

        // Test parameter names
        AM_EXPECT(filter->GetParameterName(BiquadResonantFilter::ATTRIBUTE_WET) == "Wet");
        AM_EXPECT(filter->GetParameterName(BiquadResonantFilter::ATTRIBUTE_TYPE) == "Type");
        AM_EXPECT(filter->GetParameterName(BiquadResonantFilter::ATTRIBUTE_FREQUENCY) == "Frequency");
        AM_EXPECT(filter->GetParameterName(BiquadResonantFilter::ATTRIBUTE_RESONANCE) == "Q");
        AM_EXPECT(filter->GetParameterName(BiquadResonantFilter::ATTRIBUTE_GAIN) == "Gain");

        // Test parameter types
        AM_EXPECT(filter->GetParameterType(BiquadResonantFilter::ATTRIBUTE_WET) == eParameterType_Float);
        AM_EXPECT(filter->GetParameterType(BiquadResonantFilter::ATTRIBUTE_TYPE) == eParameterType_Int);
        AM_EXPECT(filter->GetParameterType(BiquadResonantFilter::ATTRIBUTE_FREQUENCY) == eParameterType_Float);
        AM_EXPECT(filter->GetParameterType(BiquadResonantFilter::ATTRIBUTE_RESONANCE) == eParameterType_Float);
        AM_EXPECT(filter->GetParameterType(BiquadResonantFilter::ATTRIBUTE_GAIN) == eParameterType_Float);

        // Test WET parameter boundaries
        AM_EXPECT(filter->GetParameterMin(BiquadResonantFilter::ATTRIBUTE_WET) == 0.0f);
        AM_EXPECT(filter->GetParameterMax(BiquadResonantFilter::ATTRIBUTE_WET) == 1.0f);

        // Test TYPE parameter boundaries
        AM_EXPECT(filter->GetParameterMin(BiquadResonantFilter::ATTRIBUTE_TYPE) == 0.0f);
        AM_EXPECT(filter->GetParameterMax(BiquadResonantFilter::ATTRIBUTE_TYPE) == BiquadResonantFilter::TYPE_LAST - 1);

        // Test FREQUENCY parameter boundaries
        AM_EXPECT(filter->GetParameterMin(BiquadResonantFilter::ATTRIBUTE_FREQUENCY) == 10.0f);
        AM_EXPECT(filter->GetParameterMax(BiquadResonantFilter::ATTRIBUTE_FREQUENCY) == 30000.0f);

        // Test RESONANCE parameter boundaries
        AM_EXPECT(filter->GetParameterMin(BiquadResonantFilter::ATTRIBUTE_RESONANCE) == 0.025f);
        AM_EXPECT(filter->GetParameterMax(BiquadResonantFilter::ATTRIBUTE_RESONANCE) == 40.0f);

        // Test GAIN parameter boundaries
        AM_EXPECT(filter->GetParameterMin(BiquadResonantFilter::ATTRIBUTE_GAIN) == -30.0f);
        AM_EXPECT(filter->GetParameterMax(BiquadResonantFilter::ATTRIBUTE_GAIN) == 30.0f);

        Filter::Unregister(filter);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
