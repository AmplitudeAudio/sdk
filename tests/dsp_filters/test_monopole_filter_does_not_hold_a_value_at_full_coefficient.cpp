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
    // A coefficient of 1 is a pole on the unit circle: without a bound, the recursion is y = previous, and the last
    // output is held forever as a constant value, whatever comes in. A full obstruction, or a directivity null, reaches
    // it. The filter must stay a (very heavy) low-pass whose state decays.
    AM_TEST_CASE(ComponentTestCase, dsp_filters, monopole_filter_does_not_hold_a_value_at_full_coefficient)
    {
    public:
        void Run() override
        {
            for (const AmUInt32 sampleRate : { 44100u, 48000u, 96000u })
            {
                MonoPoleFilter filter;
                AM_EXPECT_EQ(filter.Initialize(0.5f), eErrorCode_Success);

                auto instance = filter.CreateInstance();
                instance->SetParameter(MonoPoleFilter::ATTRIBUTE_WET, 1.0f);

                // Settle the state on a constant input: the output follows it.
                const AmUInt64 settleFrames = sampleRate / 10;
                AudioBuffer constant(settleFrames, 1);
                AudioBuffer settled(settleFrames, 1);
                for (AmUInt64 i = 0; i < settleFrames; ++i)
                    constant[0][i] = 0.5f;

                instance->Process(constant, settled, settleFrames, sampleRate);
                AM_EXPECT(std::abs(settled[0][settleFrames - 1] - 0.5f) < 1e-3f);

                // At the full coefficient, silence in must fade the held value out.
                instance->SetParameter(MonoPoleFilter::ATTRIBUTE_COEFFICIENT, 1.0f);

                const AmUInt64 tailFrames = sampleRate / 2;
                AudioBuffer silence(tailFrames, 1);
                AudioBuffer tail(tailFrames, 1);
                silence.Clear();

                instance->Process(silence, tail, tailFrames, sampleRate);

                AM_EXPECT(std::abs(tail[0][tailFrames - 1]) < 1e-3f);

                // It still is a low-pass, not a mute: a constant input comes back out at full coefficient.
                instance->Process(constant, settled, settleFrames, sampleRate);
                AM_EXPECT(std::abs(settled[0][settleFrames - 1] - 0.5f) < 1e-2f);
            }
        }
    };

    AM_REGISTER_TEST(dsp_filters, monopole_filter_does_not_hold_a_value_at_full_coefficient);
} // namespace SparkyStudios::Audio::Amplitude::Tests
