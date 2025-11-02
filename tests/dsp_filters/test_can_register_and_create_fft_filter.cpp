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

#include <DSP/Filters/FFTFilter.h>

#include "SimpleTestCase.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void SimpleTestCase::Run()
    {
        auto filter = amshared(FFTFilter, "TestFFTFilter");
        AM_EXPECT_NOT(filter == nullptr);

        auto instance = filter->CreateInstance();
        AM_EXPECT_NOT(instance == nullptr);

        // Test basic functionality
        AM_EXPECT_EQ(filter->GetParameterCount(), 1);
        AM_EXPECT_EQ(filter->GetParameterName(0), "Wet");
        AM_EXPECT_EQ(filter->GetParameterType(0), eParameterType_Float);
        AM_EXPECT_EQ(filter->GetParameterMin(0), 0.0f);
        AM_EXPECT_EQ(filter->GetParameterMax(0), 1.0f);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
