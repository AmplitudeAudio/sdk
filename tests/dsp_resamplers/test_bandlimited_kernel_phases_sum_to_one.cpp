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

#include <cmath>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <DSP/Resamplers/BandlimitedKernel.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_resamplers, bandlimited_kernel_phases_sum_to_one)
    {
    public:
        void Run() override
        {
            for (const BandlimitedKernelSpec& spec : { kSincKernel, kSincBestKernel })
            {
                const BandlimitedKernel kernel(spec);

                // A windowed sinc passes DC to within its stopband level; the tolerance follows the preset.
                const AmReal64 tolerance = 2.0 * std::pow(10.0, spec.stopbandDb / 20.0);

                for (const AmReal64 frac : { 0.0, 0.123, 0.25, 0.5, 0.875 })
                {
                    AmReal64 sum = 0.0;
                    for (AmInt32 k = -static_cast<AmInt32>(spec.zeroCrossings); k <= static_cast<AmInt32>(spec.zeroCrossings); ++k)
                        sum += kernel.Evaluate(static_cast<AmReal64>(k) - frac);

                    AM_EXPECT(std::abs(sum - 1.0) <= tolerance);
                }
            }
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, bandlimited_kernel_phases_sum_to_one);
} // namespace SparkyStudios::Audio::Amplitude::Tests
