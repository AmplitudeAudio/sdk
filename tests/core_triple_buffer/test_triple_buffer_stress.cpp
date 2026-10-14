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

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Utils/TripleBuffer.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // A publisher rebuilds a vector of one repeated sequence number at full speed, with sizes that make it reallocate.
    // The reader holds each snapshot across several yields and checks that it never changes under it, and that the
    // sequence it sees never goes backwards. Against the old double buffer this reads freed storage.
    AM_TEST_CASE(ComponentTestCase, core_triple_buffer, reader_never_sees_a_torn_or_rewritten_snapshot)
    {
    public:
        void Run() override
        {
            struct Payload
            {
                std::vector<AmUInt32> values;
            };

            TripleBuffer<Payload> buffer;
            std::atomic<bool> stop{ false };

            // Seeded so the first Read() never returns an empty buffer, which would make the read count depend on the
            // publisher thread being scheduled.
            buffer.Write().values.assign(8, 0);
            buffer.Publish();

            std::thread publisher(
                [&]()
                {
                    AmUInt32 sequence = 0;
                    while (!stop.load(std::memory_order_relaxed))
                    {
                        ++sequence;

                        buffer.Write().values.assign(1 + (sequence * 7) % 97, sequence);
                        buffer.Publish();
                    }
                });

            bool consistent = true;
            bool monotonic = true;
            AmUInt64 reads = 0;
            AmUInt32 previous = 0;

            const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(500);
            while (std::chrono::steady_clock::now() < deadline)
            {
                const auto& held = buffer.Read();
                const AmUInt32 sequence = held.values.front();

                // Hold the snapshot while the publisher keeps going.
                for (int round = 0; round < 8; ++round)
                {
                    for (const AmUInt32 value : held.values)
                        if (value != sequence)
                            consistent = false;

                    std::this_thread::yield();
                }

                if (sequence < previous)
                    monotonic = false;

                previous = sequence;
                ++reads;
            }

            stop.store(true, std::memory_order_relaxed);
            publisher.join();

            AM_EXPECT(reads > 0);
            AM_EXPECT(consistent);
            AM_EXPECT(monotonic);
        }
    };

    AM_REGISTER_TEST(core_triple_buffer, reader_never_sees_a_torn_or_rewritten_snapshot);
} // namespace SparkyStudios::Audio::Amplitude::Tests