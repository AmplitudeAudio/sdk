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
    AM_TEST_CASE(DSPTestCase, dsp_filters, fft_filter_processing_is_chunk_size_invariant)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 totalFrames = 4096;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            AudioBuffer input(totalFrames, channelCount);
            GenerateMultiTone(input, sampleRate, { 100.0f, 1000.0f, 6000.0f }, 0.25f);

            // One-shot processing
            auto filterA = amshared(BassBoostFilter);
            filterA->Initialize(2.0f);
            auto instanceA = filterA->CreateInstance();

            AudioBuffer outputA(totalFrames, channelCount);
            instanceA->Process(input, outputA, totalFrames, sampleRate);

            // Same stream processed in odd-sized chunks
            auto filterB = amshared(BassBoostFilter);
            filterB->Initialize(2.0f);
            auto instanceB = filterB->CreateInstance();

            AudioBuffer outputB(totalFrames, channelCount);

            const AmUInt64 chunks[] = { 480, 512, 1000, 300, 1804 };
            AmUInt64 offset = 0;
            for (const AmUInt64 chunkFrames : chunks)
            {
                AudioBuffer inChunk(chunkFrames, channelCount);
                AudioBuffer outChunk(chunkFrames, channelCount);

                for (AmUInt64 i = 0; i < chunkFrames; ++i)
                    inChunk[0][i] = input[0][offset + i];

                instanceB->Process(inChunk, outChunk, chunkFrames, sampleRate);

                for (AmUInt64 i = 0; i < chunkFrames; ++i)
                    outputB[0][offset + i] = outChunk[0][i];

                offset += chunkFrames;
            }

            // A streaming filter must produce the same output regardless of chunking.
            for (AmUInt64 i = 0; i < totalFrames; ++i)
                AM_EXPECT(std::abs(outputA[0][i] - outputB[0][i]) < 1e-5f);
        }
    };

    AM_REGISTER_TEST(dsp_filters, fft_filter_processing_is_chunk_size_invariant);
} // namespace SparkyStudios::Audio::Amplitude::Tests
