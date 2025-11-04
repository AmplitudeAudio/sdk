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
        filter->InitializeLowPass(1000.0f, 0.707107f);

        Filter::Unregister(Filter::Find("BiquadResonant"));
        Filter::Register(filter);

        AM_EXPECT_NOT(Filter::Find("BiquadResonant") == nullptr);
        AM_EXPECT(Filter::Find("BiquadResonant")->GetName() == "BiquadResonant");

        {
            auto instance = Filter::Construct("BiquadResonant");
            AM_EXPECT_NOT(instance == nullptr);
        }

        Filter::Unregister(filter);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
