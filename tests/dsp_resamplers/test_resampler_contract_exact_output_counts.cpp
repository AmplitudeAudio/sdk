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
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_resamplers, resampler_contract_exact_output_counts)
    {
    public:
        void Run() override
        {
            constexpr AmUInt32 kRates[][2] = { { 44100, 48000 }, { 48000, 44100 }, { 22050, 48000 }, { 96000, 48000 }, { 48000, 48000 } };
            constexpr AmUInt64 kBlocks[] = { 1, 7, 256, 1024, 4096 };

            // The sinc presets read up to 40 zero crossings at the 4x stretch cap (161 frames) ahead of the next
            // output's centre, so the input buffer has to leave room for the read-ahead on top of the block itself.
            constexpr AmUInt64 kReadAheadHeadroom = 256;

            for (const char* name : kResamplerPresets)
            {
                for (const auto& rates : kRates)
                {
                    for (const AmUInt64 block : kBlocks)
                    {
                        auto instance = Resampler::Construct(name);
                        instance->Initialize(1, rates[0], rates[1]);

                        AudioBuffer input(block * 4 + kReadAheadHeadroom, 1);
                        AudioBuffer output(block, 1);

                        for (int call = 0; call < 8; ++call)
                        {
                            const AmUInt64 needed = instance->GetInputFramesNeeded(block);
                            AM_EXPECT(needed <= input.GetFrameCount());

                            for (AmUInt64 i = 0; i < needed; ++i)
                                input[0][i] = 0.25f;

                            AmUInt64 inFrames = needed;
                            AmUInt64 outFrames = block;
                            AM_EXPECT(instance->Process(input, inFrames, output, outFrames));
                            AM_EXPECT_EQ(block, outFrames);
                            AM_EXPECT(inFrames <= needed);
                        }
                    }
                }
            }
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, resampler_contract_exact_output_counts);
} // namespace SparkyStudios::Audio::Amplitude::Tests
