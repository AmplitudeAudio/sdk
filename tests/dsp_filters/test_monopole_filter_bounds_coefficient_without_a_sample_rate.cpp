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
    // A caller that does not know the sample rate passes 0: the filter assumes a rate of its own, and the
    // coefficient is bounded all the same, so a full coefficient does not hold the last value forever.
    AM_TEST_CASE(ComponentTestCase, dsp_filters, monopole_filter_bounds_coefficient_without_a_sample_rate)
    {
    public:
        void Run() override
        {
            MonoPoleFilter filter;
            AM_EXPECT_EQ(filter.Initialize(1.0f), eErrorCode_Success);

            auto instance = filter.CreateInstance();
            instance->SetParameter(MonoPoleFilter::ATTRIBUTE_WET, 1.0f);

            constexpr AmUInt64 frames = 48000;
            AudioBuffer constant(frames, 1);
            AudioBuffer silence(frames, 1);
            AudioBuffer out(frames, 1);
            silence.Clear();
            for (AmUInt64 i = 0; i < frames; ++i)
                constant[0][i] = 0.5f;

            instance->Process(constant, out, frames, 0);
            AM_EXPECT(out[0][frames - 1] > 0.1f);

            instance->Process(silence, out, frames, 0);
            AM_EXPECT(std::abs(out[0][frames - 1]) < 1e-2f);
        }
    };

    AM_REGISTER_TEST(dsp_filters, monopole_filter_bounds_coefficient_without_a_sample_rate);
} // namespace SparkyStudios::Audio::Amplitude::Tests
