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

#include <Utils/Audio/FFT/AudioFFT.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_utilities, fft_plans_are_shared_per_size)
    {
    public:
        void Run() override
        {
#if defined(AM_FFT_FFTW3)
            AM_EXPECT(GetFFTPlanUseCount(4096) == 0);
#else
            constexpr AmSize size = 4096;
            AM_EXPECT(GetFFTPlanUseCount(size) == 0);

            AudioBuffer signal(size, 1);
            GenerateSineWave(signal, size);

            {
                FFT a;
                a.Initialize(size);
                AM_EXPECT(GetFFTPlanUseCount(size) == 1);

                {
                    FFT b;
                    b.Initialize(size);
                    AM_EXPECT(GetFFTPlanUseCount(size) == 2);

                    SplitComplex fromA;
                    SplitComplex fromB;
                    a.Forward(signal[0].begin(), fromA);
                    b.Forward(signal[0].begin(), fromB);

                    bool same = true;
                    for (AmSize i = 0; i < fromA.GetSize(); ++i)
                        same = same && fromA.re()[i] == fromB.re()[i] && fromA.im()[i] == fromB.im()[i];
                    AM_EXPECT(same);

                    // Re-initialising to another size releases the shared plan.
                    b.Initialize(size / 2);
                    AM_EXPECT(GetFFTPlanUseCount(size) == 1);
                    AM_EXPECT(GetFFTPlanUseCount(size / 2) == 1);
                }

                AM_EXPECT(GetFFTPlanUseCount(size / 2) == 0);
            }

            AM_EXPECT(GetFFTPlanUseCount(size) == 0);

            // A released plan is rebuilt and still round-trips.
            FFT c;
            c.Initialize(size);
            AM_EXPECT(GetFFTPlanUseCount(size) == 1);

            SplitComplex spectrum;
            AudioBuffer back(size, 1);
            c.Forward(signal[0].begin(), spectrum);
            c.Backward(back[0].begin(), spectrum);

            AmReal32 maxDiff = 0.0f;
            for (AmSize i = 0; i < size; ++i)
                maxDiff = std::max(maxDiff, std::abs(back[0][i] - signal[0][i]));
            AM_EXPECT(maxDiff < 1e-4f);
#endif
        }
    };

    AM_REGISTER_TEST(dsp_utilities, fft_plans_are_shared_per_size);
} // namespace SparkyStudios::Audio::Amplitude::Tests
