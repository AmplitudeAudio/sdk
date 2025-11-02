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
        auto filter = amshared(BiquadResonantFilter);

        // Test failure paths - invalid filter type
        auto result = filter->Initialize(static_cast<BiquadResonantFilter::TYPE>(BiquadResonantFilter::TYPE_LAST), 1000.0f, 1.0f, 0.0f);
        AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

        // Test failure paths - negative frequency
        result = filter->Initialize(BiquadResonantFilter::TYPE_LOW_PASS, -1.0f, 1.0f, 0.0f);
        AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

        // Test failure paths - zero frequency
        result = filter->Initialize(BiquadResonantFilter::TYPE_LOW_PASS, 0.0f, 1.0f, 0.0f);
        AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

        // Test failure paths - negative resonance
        result = filter->Initialize(BiquadResonantFilter::TYPE_LOW_PASS, 1000.0f, -1.0f, 0.0f);
        AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

        // Test failure paths - zero resonance
        result = filter->Initialize(BiquadResonantFilter::TYPE_LOW_PASS, 1000.0f, 0.0f, 0.0f);
        AM_EXPECT_EQ(result, eErrorCode_InvalidParameter);

        // Test success path
        result = filter->Initialize(BiquadResonantFilter::TYPE_LOW_PASS, 1000.0f, 1.0f, 0.0f);
        AM_EXPECT_EQ(result, eErrorCode_Success);

        auto instance = filter->CreateInstance();
        AM_EXPECT_NOT(instance == nullptr);

        // Test parameter get/set
        instance->SetParameter(BiquadResonantFilter::ATTRIBUTE_WET, 0.5f);
        AM_EXPECT_EQ(instance->GetParameter(BiquadResonantFilter::ATTRIBUTE_WET), 0.5f);

        instance->SetParameter(BiquadResonantFilter::ATTRIBUTE_TYPE, static_cast<AmReal32>(BiquadResonantFilter::TYPE_HIGH_PASS));
        AM_EXPECT_EQ(instance->GetParameter(BiquadResonantFilter::ATTRIBUTE_TYPE), static_cast<AmReal32>(BiquadResonantFilter::TYPE_HIGH_PASS));

        instance->SetParameter(BiquadResonantFilter::ATTRIBUTE_FREQUENCY, 2000.0f);
        AM_EXPECT_EQ(instance->GetParameter(BiquadResonantFilter::ATTRIBUTE_FREQUENCY), 2000.0f);

        instance->SetParameter(BiquadResonantFilter::ATTRIBUTE_RESONANCE, 2.0f);
        AM_EXPECT_EQ(instance->GetParameter(BiquadResonantFilter::ATTRIBUTE_RESONANCE), 2.0f);

        instance->SetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN, 6.0f);
        AM_EXPECT_EQ(instance->GetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN), 6.0f);

        // Test with full wet
        instance->SetParameter(BiquadResonantFilter::ATTRIBUTE_WET, 1.0f);
        AM_EXPECT_EQ(instance->GetParameter(BiquadResonantFilter::ATTRIBUTE_WET), 1.0f);

        // Test with dry signal
        instance->SetParameter(BiquadResonantFilter::ATTRIBUTE_WET, 0.0f);
        AM_EXPECT_EQ(instance->GetParameter(BiquadResonantFilter::ATTRIBUTE_WET), 0.0f);

        // Test with incorrect param
        instance->SetParameter(BiquadResonantFilter::ATTRIBUTE_LAST, 5.0f);
        AM_EXPECT_EQ(instance->GetParameter(BiquadResonantFilter::ATTRIBUTE_LAST), 0.0f);

        AM_EXPECT_EQ(filter->GetParameterCount(), 5);

        AM_EXPECT_EQ(filter->GetParameterMin(BiquadResonantFilter::ATTRIBUTE_WET), 0.0f);
        AM_EXPECT_EQ(filter->GetParameterMin(BiquadResonantFilter::ATTRIBUTE_TYPE), 0.0f);
        AM_EXPECT_EQ(filter->GetParameterMin(BiquadResonantFilter::ATTRIBUTE_FREQUENCY), 10.0f);
        AM_EXPECT_EQ(filter->GetParameterMin(BiquadResonantFilter::ATTRIBUTE_RESONANCE), 0.025f);
        AM_EXPECT_EQ(filter->GetParameterMin(BiquadResonantFilter::ATTRIBUTE_GAIN), -30.0f);
        AM_EXPECT_EQ(filter->GetParameterMin(BiquadResonantFilter::ATTRIBUTE_LAST), 0.0f);

        AM_EXPECT_EQ(filter->GetParameterMax(BiquadResonantFilter::ATTRIBUTE_WET), 1.0f);
        AM_EXPECT_EQ(filter->GetParameterMax(BiquadResonantFilter::ATTRIBUTE_TYPE), static_cast<AmReal32>(BiquadResonantFilter::TYPE_LAST - 1));
        AM_EXPECT_EQ(filter->GetParameterMax(BiquadResonantFilter::ATTRIBUTE_FREQUENCY), 30000.0f);
        AM_EXPECT_EQ(filter->GetParameterMax(BiquadResonantFilter::ATTRIBUTE_RESONANCE), 40.0f);
        AM_EXPECT_EQ(filter->GetParameterMax(BiquadResonantFilter::ATTRIBUTE_GAIN), 30.0f);
        AM_EXPECT_EQ(filter->GetParameterMax(BiquadResonantFilter::ATTRIBUTE_LAST), 0.0f);

        AM_EXPECT_EQ(filter->GetParameterType(BiquadResonantFilter::ATTRIBUTE_WET), eParameterType_Float);
        AM_EXPECT_EQ(filter->GetParameterType(BiquadResonantFilter::ATTRIBUTE_TYPE), eParameterType_Int);
        AM_EXPECT_EQ(filter->GetParameterType(BiquadResonantFilter::ATTRIBUTE_FREQUENCY), eParameterType_Float);
        AM_EXPECT_EQ(filter->GetParameterType(BiquadResonantFilter::ATTRIBUTE_RESONANCE), eParameterType_Float);
        AM_EXPECT_EQ(filter->GetParameterType(BiquadResonantFilter::ATTRIBUTE_GAIN), eParameterType_Float);
        AM_EXPECT_EQ(filter->GetParameterType(BiquadResonantFilter::ATTRIBUTE_LAST), eParameterType_Float);

        AM_EXPECT_EQ(filter->GetParameterName(BiquadResonantFilter::ATTRIBUTE_WET), "Wet");
        AM_EXPECT_EQ(filter->GetParameterName(BiquadResonantFilter::ATTRIBUTE_TYPE), "Type");
        AM_EXPECT_EQ(filter->GetParameterName(BiquadResonantFilter::ATTRIBUTE_FREQUENCY), "Frequency");
        AM_EXPECT_EQ(filter->GetParameterName(BiquadResonantFilter::ATTRIBUTE_RESONANCE), "Resonance");
        AM_EXPECT_EQ(filter->GetParameterName(BiquadResonantFilter::ATTRIBUTE_GAIN), "Gain");
        AM_EXPECT_EQ(filter->GetParameterName(BiquadResonantFilter::ATTRIBUTE_LAST), "Unknown");
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
