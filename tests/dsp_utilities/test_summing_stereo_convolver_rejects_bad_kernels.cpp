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
    AM_TEST_CASE(DSPTestCase, dsp_utilities, summing_stereo_convolver_rejects_bad_kernels)
    {
    public:
        void Run() override
        {
            const std::vector<AmAudioSample> ir(300, 0.25f);
            const auto k128 = ConvolverKernel::Create(128, ir.data(), ir.size());
            const auto k256 = ConvolverKernel::Create(256, ir.data(), ir.size());

            SummingStereoConvolver convolver;

            // Mismatched block sizes.
            const std::vector<std::shared_ptr<const ConvolverKernel>> mixed = { k128, k256 };
            const std::vector<std::shared_ptr<const ConvolverKernel>> same = { k128, k128 };
            AM_EXPECT_NOT(convolver.Init(mixed, same));

            // Left and right counts differ.
            const std::vector<std::shared_ptr<const ConvolverKernel>> one = { k128 };
            AM_EXPECT_NOT(convolver.Init(same, one));

            // No input.
            const std::vector<std::shared_ptr<const ConvolverKernel>> none;
            AM_EXPECT_NOT(convolver.Init(none, none));

            // A null kernel.
            const std::vector<std::shared_ptr<const ConvolverKernel>> withNull = { k128, nullptr };
            AM_EXPECT_NOT(convolver.Init(withNull, same));

            // More than 16 inputs.
            const std::vector<std::shared_ptr<const ConvolverKernel>> many(SummingStereoConvolver::kMaxInputs + 1, k128);
            AM_EXPECT_NOT(convolver.Init(many, many));

            // A failed Init leaves nothing behind: Process adds nothing.
            std::vector<AmAudioSample> outL(64, 0.5f), outR(64, 0.5f);
            const std::vector<AmAudioSample> input(64, 1.0f);
            const AmAudioSample* inputs[] = { input.data(), input.data() };
            convolver.Process(inputs, outL.data(), outR.data(), 64);
            AM_EXPECT(outL == std::vector<AmAudioSample>(64, 0.5f));
            AM_EXPECT(outR == std::vector<AmAudioSample>(64, 0.5f));

            // The maximum is accepted.
            const std::vector<std::shared_ptr<const ConvolverKernel>> sixteen(SummingStereoConvolver::kMaxInputs, k128);
            AM_EXPECT(convolver.Init(sixteen, sixteen));
        }
    };

    AM_REGISTER_TEST(dsp_utilities, summing_stereo_convolver_rejects_bad_kernels);
} // namespace SparkyStudios::Audio::Amplitude::Tests
