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

#include <DSP/Resamplers/BandlimitedKernel.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_resamplers, bandlimited_kernel_shape)
    {
    public:
        void Run() override
        {
            for (const BandlimitedKernelSpec& spec : { kSincKernel, kSincBestKernel })
            {
                const BandlimitedKernel kernel(spec);

                AM_EXPECT_EQ(1.0f, kernel.Evaluate(0.0));

                // Exactly 0 at every whole frame but the centre: a whole-frame phase passes the input through.
                for (AmUInt32 k = 1; k <= spec.zeroCrossings; ++k)
                    AM_EXPECT_EQ(0.0f, kernel.Evaluate(static_cast<AmReal64>(k)));

                AM_EXPECT_EQ(0.0f, kernel.Evaluate(static_cast<AmReal64>(spec.zeroCrossings) + 0.5));

                for (const AmReal64 d : { 0.1, 0.5, 1.37, 7.9 })
                    AM_EXPECT_EQ(kernel.Evaluate(d), kernel.Evaluate(-d));
            }
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, bandlimited_kernel_shape);
} // namespace SparkyStudios::Audio::Amplitude::Tests
