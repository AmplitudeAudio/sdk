// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#include <DSP/Filters/CompressorFilter.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, dsp_filters, compressor_filter_parameters)
    {
    public:
        void Run() override
        {
            auto filter = amshared(CompressorFilter);
            AM_EXPECT_EQ(filter->GetParameterCount(), CompressorFilter::ATTRIBUTE_LAST);

            AM_EXPECT_EQ(filter->GetParameterName(CompressorFilter::ATTRIBUTE_WET), "Wet");
            AM_EXPECT_EQ(filter->GetParameterName(CompressorFilter::ATTRIBUTE_THRESHOLD), "Threshold");
            AM_EXPECT_EQ(filter->GetParameterName(CompressorFilter::ATTRIBUTE_RATIO), "Ratio");
            AM_EXPECT_EQ(filter->GetParameterName(CompressorFilter::ATTRIBUTE_ATTACK), "Attack");
            AM_EXPECT_EQ(filter->GetParameterName(CompressorFilter::ATTRIBUTE_RELEASE), "Release");
            AM_EXPECT_EQ(filter->GetParameterName(CompressorFilter::ATTRIBUTE_KNEE), "Knee");
            AM_EXPECT_EQ(filter->GetParameterName(CompressorFilter::ATTRIBUTE_MAKEUP_GAIN), "MakeupGain");

            AM_EXPECT_EQ(filter->GetParameterType(CompressorFilter::ATTRIBUTE_WET), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(CompressorFilter::ATTRIBUTE_THRESHOLD), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(CompressorFilter::ATTRIBUTE_RATIO), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(CompressorFilter::ATTRIBUTE_ATTACK), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(CompressorFilter::ATTRIBUTE_RELEASE), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(CompressorFilter::ATTRIBUTE_KNEE), eParameterType_Float);
            AM_EXPECT_EQ(filter->GetParameterType(CompressorFilter::ATTRIBUTE_MAKEUP_GAIN), eParameterType_Float);

            AM_EXPECT_EQ(filter->GetParameterMin(CompressorFilter::ATTRIBUTE_WET), 0.0f);
            AM_EXPECT_EQ(filter->GetParameterMin(CompressorFilter::ATTRIBUTE_THRESHOLD), -60.0f);
            AM_EXPECT_EQ(filter->GetParameterMin(CompressorFilter::ATTRIBUTE_RATIO), 1.0f);
            AM_EXPECT_EQ(filter->GetParameterMin(CompressorFilter::ATTRIBUTE_ATTACK), 0.1f);
            AM_EXPECT_EQ(filter->GetParameterMin(CompressorFilter::ATTRIBUTE_RELEASE), 10.0f);
            AM_EXPECT_EQ(filter->GetParameterMin(CompressorFilter::ATTRIBUTE_KNEE), 0.0f);
            AM_EXPECT_EQ(filter->GetParameterMin(CompressorFilter::ATTRIBUTE_MAKEUP_GAIN), 0.0f);

            AM_EXPECT_EQ(filter->GetParameterMax(CompressorFilter::ATTRIBUTE_WET), 1.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(CompressorFilter::ATTRIBUTE_THRESHOLD), 0.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(CompressorFilter::ATTRIBUTE_RATIO), 20.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(CompressorFilter::ATTRIBUTE_ATTACK), 100.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(CompressorFilter::ATTRIBUTE_RELEASE), 1000.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(CompressorFilter::ATTRIBUTE_KNEE), 12.0f);
            AM_EXPECT_EQ(filter->GetParameterMax(CompressorFilter::ATTRIBUTE_MAKEUP_GAIN), 24.0f);

            auto instance = filter->CreateInstance();
            AM_EXPECT_NOT(instance == nullptr);

            instance->SetParameter(CompressorFilter::ATTRIBUTE_WET, 0.5f);
            AM_EXPECT_EQ(instance->GetParameter(CompressorFilter::ATTRIBUTE_WET), 0.5f);

            instance->SetParameter(CompressorFilter::ATTRIBUTE_THRESHOLD, -10.0f);
            AM_EXPECT_EQ(instance->GetParameter(CompressorFilter::ATTRIBUTE_THRESHOLD), -10.0f);

            instance->SetParameter(CompressorFilter::ATTRIBUTE_RATIO, 8.0f);
            AM_EXPECT_EQ(instance->GetParameter(CompressorFilter::ATTRIBUTE_RATIO), 8.0f);

            instance->SetParameter(CompressorFilter::ATTRIBUTE_ATTACK, 10.0f);
            AM_EXPECT_EQ(instance->GetParameter(CompressorFilter::ATTRIBUTE_ATTACK), 10.0f);

            instance->SetParameter(CompressorFilter::ATTRIBUTE_RELEASE, 50.0f);
            AM_EXPECT_EQ(instance->GetParameter(CompressorFilter::ATTRIBUTE_RELEASE), 50.0f);

            instance->SetParameter(CompressorFilter::ATTRIBUTE_KNEE, 6.0f);
            AM_EXPECT_EQ(instance->GetParameter(CompressorFilter::ATTRIBUTE_KNEE), 6.0f);

            instance->SetParameter(CompressorFilter::ATTRIBUTE_MAKEUP_GAIN, 4.0f);
            AM_EXPECT_EQ(instance->GetParameter(CompressorFilter::ATTRIBUTE_MAKEUP_GAIN), 4.0f);
        }
    };

    AM_REGISTER_TEST(dsp_filters, compressor_filter_parameters);
} // namespace SparkyStudios::Audio::Amplitude::Tests
