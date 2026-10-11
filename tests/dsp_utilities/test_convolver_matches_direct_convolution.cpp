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

#include "DSPTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_utilities, convolver_matches_direct_convolution)
    {
    public:
        void Run() override
        {
            constexpr AmSize irLen = 300;
            constexpr AmSize blockSize = 128;
            constexpr AmSize total = 1500;

            std::vector<AmAudioSample> ir(irLen);
            for (AmSize i = 0; i < irLen; ++i)
                ir[i] = std::exp(-static_cast<AmReal32>(i) / 60.0f) * (static_cast<AmReal32>(i % 7) - 3.0f) / 3.0f;

            std::vector<AmAudioSample> input(total);
            AmUInt32 seed = 12345u;
            for (AmSize i = 0; i < total; ++i)
            {
                seed = seed * 1664525u + 1013904223u;
                input[i] = static_cast<AmReal32>(seed >> 8) / static_cast<AmReal32>(1u << 24) * 2.0f - 1.0f;
            }

            std::vector<AmAudioSample> expected(total, 0.0f);
            for (AmSize n = 0; n < total; ++n)
                for (AmSize k = 0; k < irLen && k <= n; ++k)
                    expected[n] += ir[k] * input[n - k];

            // Dirty the allocator first: a convolver processes noise, then re-initialises and must start from silence.
            Convolver convolver;
            AM_EXPECT(convolver.Init(blockSize, ir.data(), irLen));
            {
                std::vector<AmAudioSample> scratch(total);
                convolver.Process(input.data(), scratch.data(), total);
            }
            AM_EXPECT(convolver.Init(blockSize, ir.data(), irLen));

            // Mixed call lengths: partial, single-frame, exact and multi-block.
            constexpr AmSize calls[] = { 100, 37, 128, 1, 300, 256, 77 };
            std::vector<AmAudioSample> output(total, 0.0f);
            AmSize done = 0;
            for (AmSize c = 0; done < total; ++c)
            {
                const AmSize len = std::min(calls[c % std::size(calls)], total - done);
                convolver.Process(input.data() + done, output.data() + done, len);
                done += len;
            }

            AmReal32 maxDiff = 0.0f;
            for (AmSize i = 0; i < total; ++i)
                maxDiff = std::max(maxDiff, std::abs(output[i] - expected[i]));

            AM_EXPECT(maxDiff <= 1e-4f);
        }
    };

    AM_REGISTER_TEST(dsp_utilities, convolver_matches_direct_convolution);
} // namespace SparkyStudios::Audio::Amplitude::Tests
