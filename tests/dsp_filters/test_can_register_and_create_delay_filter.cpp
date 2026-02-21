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

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, dsp_filters, can_register_and_create_delay_filter)
    {
    public:
        void Run() override
        {
            auto filter = amshared(DelayFilter);
            filter->Initialize(0.3f, 0.7f);

            Filter::Unregister(Filter::Find("Delay"));
            Filter::Register(filter);

            AM_EXPECT_NOT(Filter::Find("Delay") == nullptr);
            AM_EXPECT(Filter::Find("Delay")->GetName() == "Delay");

            {
                auto instance = Filter::Construct("Delay");
                AM_EXPECT_NOT(instance == nullptr);
            }

            Filter::Unregister(filter);
        }
    };

    AM_REGISTER_TEST(dsp_filters, can_register_and_create_delay_filter);
} // namespace SparkyStudios::Audio::Amplitude::Tests
