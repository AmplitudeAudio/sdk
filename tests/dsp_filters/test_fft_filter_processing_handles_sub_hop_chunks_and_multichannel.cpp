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

#include <DSP/Filters/BassBoostFilter.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_filters, fft_filter_processing_handles_sub_hop_chunks_and_multichannel)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 totalFrames = 4096;
            constexpr AmUInt16 channelCount = 2;
            constexpr AmUInt32 sampleRate = 48000;

            AudioBuffer input(totalFrames, channelCount);
            GenerateMultiTone(input, sampleRate, { 100.0f, 1000.0f, 6000.0f }, 0.25f);

            // One-shot processing, stereo.
            auto filterA = amshared(BassBoostFilter);
            filterA->Initialize(2.0f);
            auto instanceA = filterA->CreateInstance();

            AudioBuffer outputA(totalFrames, channelCount);
            instanceA->Process(input, outputA, totalFrames, sampleRate);

            // The same stream processed in chunks. Chunkings below the
            // 128-sample hop (including consecutive sub-hop calls, which
            // stress the carry/priming path) must not change the output.
            const std::vector<std::vector<AmUInt64>> experiments = {
                { 480, 512, 1000, 300, 1804 }, // supra-hop baseline, as in the existing test
                { 64, 65, 128, 33, 300, 100, 512, 480, 512, 1000, 902 }, // mixed, with chunks < 128
                { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
                  128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 }, // hop-aligned
                { 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 }, // pure sub-hop
            };

            for (const auto& chunks : experiments)
            {
                auto filterB = amshared(BassBoostFilter);
                filterB->Initialize(2.0f);
                auto instanceB = filterB->CreateInstance();

                AudioBuffer outputB(totalFrames, channelCount);

                AmUInt64 offset = 0;
                for (const AmUInt64 chunkFrames : chunks)
                {
                    AudioBuffer inChunk(chunkFrames, channelCount);
                    AudioBuffer outChunk(chunkFrames, channelCount);

                    for (AmUInt16 c = 0; c < channelCount; ++c)
                        for (AmUInt64 i = 0; i < chunkFrames; ++i)
                            inChunk[c][i] = input[c][offset + i];

                    instanceB->Process(inChunk, outChunk, chunkFrames, sampleRate);

                    for (AmUInt16 c = 0; c < channelCount; ++c)
                        for (AmUInt64 i = 0; i < chunkFrames; ++i)
                            outputB[c][offset + i] = outChunk[c][i];

                    offset += chunkFrames;
                }

                // A streaming filter must produce the same output regardless of
                // chunking. Both paths include the same 256-sample priming
                // latency, so outputs compare sample index to sample index.
                for (AmUInt16 c = 0; c < channelCount; ++c)
                    for (AmUInt64 i = 0; i < totalFrames; ++i)
                        AM_EXPECT(std::abs(outputA[c][i] - outputB[c][i]) < 1e-5f);
            }
        }
    };

    AM_REGISTER_TEST(dsp_filters, fft_filter_processing_handles_sub_hop_chunks_and_multichannel);
} // namespace SparkyStudios::Audio::Amplitude::Tests
