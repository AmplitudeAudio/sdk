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
#include <random>
#include <thread>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/Streaming/StreamRing.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, mixer_streaming, stream_ring_survives_concurrent_producer_and_consumer)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 kTotalFrames = 200000;
            constexpr AmUInt64 kMaxWriteChunk = 37;
            constexpr AmUInt64 kMaxReadChunk = 29;

            StreamRing ring(64, 2);

            // Frame n carries n on channel 0 and -n on channel 1.
            std::thread producer(
                [&ring]()
                {
                    std::minstd_rand rng(1234);
                    std::uniform_int_distribution<AmUInt64> chunkSize(1, kMaxWriteChunk);
                    AudioBuffer source(kMaxWriteChunk, 2);

                    AmUInt64 next = 0;
                    while (next < kTotalFrames)
                    {
                        const AmUInt64 frames = std::min(chunkSize(rng), kTotalFrames - next);
                        for (AmUInt64 i = 0; i < frames; ++i)
                        {
                            const AmReal32 value = static_cast<AmReal32>(next + i);
                            source[0][i] = value;
                            source[1][i] = -value;
                        }

                        AmUInt64 offset = 0;
                        while (offset < frames)
                        {
                            const AmUInt64 written = ring.Write(source, offset, frames - offset);
                            offset += written;
                            if (written == 0)
                                std::this_thread::yield();
                        }

                        next += frames;
                    }

                    ring.MarkEnd();
                });

            std::minstd_rand rng(5678);
            std::uniform_int_distribution<AmUInt64> chunkSize(1, kMaxReadChunk);
            AudioBuffer out(kMaxReadChunk, 2);

            AmUInt64 index = 0;
            AmUInt64 mismatches = 0;

            while (true)
            {
                const AmUInt64 end = ring.GetEndIndex();
                if (end != StreamRing::kNoEnd && index >= end)
                    break;

                const AmUInt64 got = ring.Read(index, out, 0, chunkSize(rng));
                if (got == 0)
                {
                    std::this_thread::yield();
                    continue;
                }

                for (AmUInt64 i = 0; i < got; ++i)
                {
                    const AmReal32 expected = static_cast<AmReal32>(index + i);
                    if (out[0][i] != expected || out[1][i] != -expected)
                        ++mismatches;
                }

                index += got;
                ring.Release(index);
            }

            producer.join();

            AM_EXPECT_EQ(kTotalFrames, index);
            AM_EXPECT_EQ(0u, mismatches);
        }
    };

    AM_REGISTER_TEST(mixer_streaming, stream_ring_survives_concurrent_producer_and_consumer);
} // namespace SparkyStudios::Audio::Amplitude::Tests
