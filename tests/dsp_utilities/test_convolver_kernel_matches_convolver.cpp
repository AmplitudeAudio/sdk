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

#include "DSPTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_utilities, convolver_kernel_matches_convolver)
    {
    public:
        void Run() override
        {
            AM_EXPECT(ConvolverKernel::Create(0, nullptr, 0) == nullptr);

            // Block sizes: a power of two and one that rounds up. IR lengths: below, equal to and above a block.
            for (const AmSize blockSize : { static_cast<AmSize>(256), static_cast<AmSize>(200) })
                for (const AmSize irLen : { static_cast<AmSize>(100), static_cast<AmSize>(256), static_cast<AmSize>(700) })
                    CheckAgainstConvolver(blockSize, irLen);

            // Trailing values under 1e-6 are trimmed.
            {
                std::vector<AmAudioSample> ir = MakeIR(300);
                const auto plain = ConvolverKernel::Create(256, ir.data(), ir.size());
                ir.resize(600, 1e-7f);
                const auto padded = ConvolverKernel::Create(256, ir.data(), ir.size());
                AM_EXPECT(plain->GetSegmentCount() == padded->GetSegmentCount());
            }

            // An all-zero IR gives an empty kernel.
            {
                const std::vector<AmAudioSample> zeros(256, 0.0f);
                const auto kernel = ConvolverKernel::Create(256, zeros.data(), zeros.size());
                AM_EXPECT(kernel != nullptr);
                AM_EXPECT(kernel->GetSegmentCount() == 0);
                AM_EXPECT(kernel->GetBlockSize() == 256);
            }
        }

    private:
        // The FFT backend is not bit-reproducible across buffers, so spectra are compared within float tolerance.
        static bool Near(AmReal32 a, AmReal32 b)
        {
            return std::abs(a - b) <= 1e-6f * std::max(1.0f, std::abs(b));
        }

        static std::vector<AmAudioSample> MakeIR(AmSize irLen)
        {
            std::vector<AmAudioSample> ir(irLen);
            for (AmSize i = 0; i < irLen; ++i)
                ir[i] = std::exp(-static_cast<AmReal32>(i) / 80.0f) * std::cos(0.3f * static_cast<AmReal32>(i));
            ir[irLen - 1] = 0.5f;
            return ir;
        }

        void CheckAgainstConvolver(AmSize blockSize, AmSize irLen)
        {
            const std::vector<AmAudioSample> ir = MakeIR(irLen);

            Convolver convolver;
            AM_EXPECT(convolver.Init(blockSize, ir.data(), irLen));

            const auto kernel = ConvolverKernel::Create(blockSize, ir.data(), irLen);
            AM_EXPECT(kernel != nullptr);
            AM_EXPECT(kernel->GetSegmentCount() == convolver.GetSegmentCount());
            AM_EXPECT(kernel->GetSegmentSize() == convolver.GetSegmentSize());
            AM_EXPECT(kernel->GetBlockSize() * 2 == kernel->GetSegmentSize());
            AM_EXPECT(kernel->GetComplexSize() == FFT::GetOutputSize(kernel->GetSegmentSize()));

            // Each segment is the forward FFT of its zero-padded IR block.
            FFT fft;
            fft.Initialize(kernel->GetSegmentSize());
            AmAlignedReal32Buffer padded;
            padded.Resize(static_cast<AmUInt32>(kernel->GetSegmentSize()));
            SplitComplex expected(kernel->GetComplexSize());

            bool same = true;
            for (AmSize s = 0; s < kernel->GetSegmentCount(); ++s)
            {
                padded.Clear();
                const AmSize begin = s * kernel->GetBlockSize();
                const AmSize count = std::min(kernel->GetBlockSize(), irLen - begin);
                std::memcpy(padded.GetBuffer(), ir.data() + begin, count * sizeof(AmAudioSample));
                fft.Forward(padded.GetBuffer(), expected);

                const SplitComplex& actual = kernel->GetSegment(s);
                for (AmSize i = 0; i < kernel->GetComplexSize(); ++i)
                    same = same && Near(actual.re()[i], expected.re()[i]) && Near(actual.im()[i], expected.im()[i]);
            }

            AM_EXPECT(same);
        }
    };

    AM_REGISTER_TEST(dsp_utilities, convolver_kernel_matches_convolver);
} // namespace SparkyStudios::Audio::Amplitude::Tests
