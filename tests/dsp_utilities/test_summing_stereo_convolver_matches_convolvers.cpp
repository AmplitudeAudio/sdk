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

#include <DSP/ConvolverKernel.h>
#include <DSP/SummingStereoConvolver.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_utilities, summing_stereo_convolver_matches_convolvers)
    {
    public:
        void Run() override
        {
            for (const AmSize inputs : { static_cast<AmSize>(1), static_cast<AmSize>(4), static_cast<AmSize>(16) })
                CheckInputs(inputs);

            CheckReset();
            CheckEmptyKernels();
        }

    private:
        static constexpr AmSize kBlock = 128;
        static constexpr AmSize kTotal = 2000;

        // Lengths differ per input and ear so kernels have 1 to 4 segments.
        static std::vector<AmAudioSample> MakeIR(AmSize index, AmSize ear)
        {
            const AmSize irLen = 60 + (index * 37 + ear * 101) % 420;
            std::vector<AmAudioSample> ir(irLen);
            for (AmSize i = 0; i < irLen; ++i)
                ir[i] = 0.2f * std::exp(-static_cast<AmReal32>(i) / 90.0f) *
                    std::sin(0.11f * static_cast<AmReal32>((index + 1) * (ear + 2) * (i + 1)));
            ir[irLen - 1] = 0.05f;
            return ir;
        }

        static std::vector<std::vector<AmAudioSample>> MakeInputs(AmSize count)
        {
            std::vector<std::vector<AmAudioSample>> signals(count, std::vector<AmAudioSample>(kTotal));
            AmUInt32 seed = 777u;
            for (auto& signal : signals)
                for (auto& sample : signal)
                {
                    seed = seed * 1664525u + 1013904223u;
                    sample = static_cast<AmReal32>(seed >> 8) / static_cast<AmReal32>(1u << 24) * 2.0f - 1.0f;
                }
            return signals;
        }

        // Feeds the summing convolver with mixed call lengths.
        static void RunSumming(
            SummingStereoConvolver& convolver,
            const std::vector<std::vector<AmAudioSample>>& signals,
            std::vector<AmAudioSample>& outL,
            std::vector<AmAudioSample>& outR)
        {
            constexpr AmSize calls[] = { 64, 128, 1, 200, 37, 300, 256 };
            std::vector<const AmAudioSample*> pointers(signals.size());

            AmSize done = 0;
            for (AmSize c = 0; done < kTotal; ++c)
            {
                const AmSize len = std::min(calls[c % std::size(calls)], kTotal - done);
                for (AmSize i = 0; i < signals.size(); ++i)
                    pointers[i] = signals[i].data() + done;

                convolver.Process(pointers.data(), outL.data() + done, outR.data() + done, len);
                done += len;
            }
        }

        static AmReal32 MaxDiff(const std::vector<AmAudioSample>& a, const std::vector<AmAudioSample>& b)
        {
            AmReal32 maxDiff = 0.0f;
            for (AmSize i = 0; i < a.size(); ++i)
                maxDiff = std::max(maxDiff, std::abs(a[i] - b[i]));
            return maxDiff;
        }

        void CheckInputs(AmSize count)
        {
            const auto signals = MakeInputs(count);

            std::vector<std::shared_ptr<const ConvolverKernel>> kernels[2];
            std::vector<AmAudioSample> expected[2] = { std::vector<AmAudioSample>(kTotal, 0.0f),
                                                       std::vector<AmAudioSample>(kTotal, 0.0f) };

            for (AmSize ear = 0; ear < 2; ++ear)
            {
                for (AmSize i = 0; i < count; ++i)
                {
                    const auto ir = MakeIR(i, ear);
                    kernels[ear].push_back(ConvolverKernel::Create(kBlock, ir.data(), ir.size()));

                    Convolver reference;
                    reference.Init(kBlock, ir.data(), ir.size());
                    std::vector<AmAudioSample> out(kTotal, 0.0f);
                    reference.Process(signals[i].data(), out.data(), kTotal);

                    for (AmSize n = 0; n < kTotal; ++n)
                        expected[ear][n] += out[n];
                }
            }

            SummingStereoConvolver convolver;
            AM_EXPECT(convolver.Init(kernels[0], kernels[1]));
            AM_EXPECT(convolver.GetInputCount() == count);

            std::vector<AmAudioSample> outL(kTotal, 0.0f);
            std::vector<AmAudioSample> outR(kTotal, 0.0f);
            RunSumming(convolver, signals, outL, outR);

            AM_EXPECT(MaxDiff(outL, expected[0]) <= 1e-5f);
            AM_EXPECT(MaxDiff(outR, expected[1]) <= 1e-5f);
        }

        void CheckReset()
        {
            const auto signals = MakeInputs(4);
            std::vector<std::shared_ptr<const ConvolverKernel>> kernels[2];
            for (AmSize ear = 0; ear < 2; ++ear)
                for (AmSize i = 0; i < 4; ++i)
                {
                    const auto ir = MakeIR(i, ear);
                    kernels[ear].push_back(ConvolverKernel::Create(kBlock, ir.data(), ir.size()));
                }

            SummingStereoConvolver fresh;
            AM_EXPECT(fresh.Init(kernels[0], kernels[1]));
            std::vector<AmAudioSample> freshL(kTotal, 0.0f);
            std::vector<AmAudioSample> freshR(kTotal, 0.0f);
            RunSumming(fresh, signals, freshL, freshR);

            SummingStereoConvolver reused;
            AM_EXPECT(reused.Init(kernels[0], kernels[1]));
            std::vector<AmAudioSample> scratchL(kTotal, 0.0f);
            std::vector<AmAudioSample> scratchR(kTotal, 0.0f);
            RunSumming(reused, signals, scratchL, scratchR);
            reused.Reset();

            std::vector<AmAudioSample> reusedL(kTotal, 0.0f);
            std::vector<AmAudioSample> reusedR(kTotal, 0.0f);
            RunSumming(reused, signals, reusedL, reusedR);

            AM_EXPECT(reusedL == freshL);
            AM_EXPECT(reusedR == freshR);
        }

        void CheckEmptyKernels()
        {
            const auto signals = MakeInputs(2);
            const std::vector<AmAudioSample> zeros(200, 0.0f);
            const auto empty = ConvolverKernel::Create(kBlock, zeros.data(), zeros.size());
            const auto irL = MakeIR(0, 0);
            const auto irR = MakeIR(0, 1);
            const auto kernelL = ConvolverKernel::Create(kBlock, irL.data(), irL.size());
            const auto kernelR = ConvolverKernel::Create(kBlock, irR.data(), irR.size());

            // An empty kernel contributes nothing: two inputs with the second empty equal one input.
            const std::vector<std::shared_ptr<const ConvolverKernel>> left2 = { kernelL, empty };
            const std::vector<std::shared_ptr<const ConvolverKernel>> right2 = { kernelR, empty };
            const std::vector<std::shared_ptr<const ConvolverKernel>> left1 = { kernelL };
            const std::vector<std::shared_ptr<const ConvolverKernel>> right1 = { kernelR };

            SummingStereoConvolver two;
            SummingStereoConvolver one;
            AM_EXPECT(two.Init(left2, right2));
            AM_EXPECT(one.Init(left1, right1));

            std::vector<AmAudioSample> twoL(kTotal, 0.0f), twoR(kTotal, 0.0f), oneL(kTotal, 0.0f), oneR(kTotal, 0.0f);
            RunSumming(two, signals, twoL, twoR);
            const std::vector<std::vector<AmAudioSample>> first = { signals[0] };
            RunSumming(one, first, oneL, oneR);

            AM_EXPECT(MaxDiff(twoL, oneL) <= 1e-6f);
            AM_EXPECT(MaxDiff(twoR, oneR) <= 1e-6f);

            // All kernels empty: Process leaves the outputs untouched.
            const std::vector<std::shared_ptr<const ConvolverKernel>> allEmpty = { empty, empty };
            SummingStereoConvolver silent;
            AM_EXPECT(silent.Init(allEmpty, allEmpty));
            std::vector<AmAudioSample> keptL(kTotal, 0.5f), keptR(kTotal, 0.5f);
            RunSumming(silent, signals, keptL, keptR);
            AM_EXPECT(keptL == std::vector<AmAudioSample>(kTotal, 0.5f));
            AM_EXPECT(keptR == std::vector<AmAudioSample>(kTotal, 0.5f));
        }
    };

    AM_REGISTER_TEST(dsp_utilities, summing_stereo_convolver_matches_convolvers);
} // namespace SparkyStudios::Audio::Amplitude::Tests
