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

#include <algorithm>
#include <cmath>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <DSP/Resamplers/BandlimitedKernel.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        // Continuous-time response of the symmetric kernel at f cycles per input frame, from its table.
        AmReal64 Response(const BandlimitedKernel& kernel, AmReal64 f)
        {
            const AmReal32* table = kernel.GetTable();
            const AmReal64 dx = 1.0 / static_cast<AmReal64>(kernel.GetPhases());
            const AmUInt64 size = static_cast<AmUInt64>(kernel.GetZeroCrossings()) * kernel.GetPhases();

            AmReal64 sum = table[0];
            for (AmUInt64 i = 1; i < size; ++i)
                sum += 2.0 * table[i] * std::cos(2.0 * AM_PI * f * static_cast<AmReal64>(i) * dx);

            return sum * dx;
        }
    } // namespace

    AM_TEST_CASE(DSPTestCase, dsp_resamplers, bandlimited_kernel_meets_its_stopband)
    {
    public:
        void Run() override
        {
            for (const BandlimitedKernelSpec& spec : { kSincKernel, kSincBestKernel })
            {
                const BandlimitedKernel kernel(spec);
                const AmReal64 dc = Response(kernel, 0.0);

                AmReal64 worstStop = -400.0;
                for (AmReal64 f = spec.stopbandEdge; f <= 1.0; f += 0.002)
                    worstStop = std::max(worstStop, 20.0 * std::log10(std::abs(Response(kernel, f) / dc) + 1e-30));

                AmReal64 worstPass = 0.0;
                for (AmReal64 f = 0.0; f <= spec.passbandEdge; f += 0.002)
                    worstPass = std::max(worstPass, std::abs(20.0 * std::log10(std::abs(Response(kernel, f) / dc))));

                AM_EXPECT(worstStop <= spec.stopbandDb);
                AM_EXPECT(worstPass <= 0.1);
            }
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, bandlimited_kernel_meets_its_stopband);
} // namespace SparkyStudios::Audio::Amplitude::Tests
