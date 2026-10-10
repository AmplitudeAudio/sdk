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

#include <DSP/Filters/MonoPoleFilter.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, dsp_filters, monopole_filter_keeps_state_per_channel)
    {
    public:
        void Run() override
        {
            MonoPoleFilter filter;
            filter.Initialize(0.5f);
            auto instance = filter.CreateInstance();

            // An impulse on the left only: the right channel must stay silent.
            AudioBuffer in(64, 2);
            in[0][0] = 1.0f;
            AudioBuffer out(64, 2);
            instance->Process(in, out, 64, 48000);

            AM_EXPECT(out[0][1] != 0.0f);
            for (AmUInt64 i = 0; i < 64; ++i)
                AM_EXPECT_EQ(0.0f, out[1][i]);
        }
    };

    AM_REGISTER_TEST(dsp_filters, monopole_filter_keeps_state_per_channel);
} // namespace SparkyStudios::Audio::Amplitude::Tests
