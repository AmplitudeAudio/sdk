// Copyright (c) 2021-present Sparky Studios. All rights reserved.
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

#include <thread>
#include <vector>

#include "MemoryTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(MemoryTestCase, core_memory, memory_usage_counters)
    {
    public:
        void Run() override
        {
            MemoryManager::Initialize(nullptr);

#if !defined(AM_NO_MEMORY_STATS)
            constexpr auto pool = eMemoryPoolKind_Filtering;
            const auto& stats = amMemory->GetStats(pool);
            const AmSize baseline = stats.currentMemoryUsed.load();

            // Current usage rises on allocation and returns to the baseline on free
            AmVoidPtr a = amMemory->Malloc(pool, 4096, __FILE__, __LINE__);
            AM_EXPECT(a != nullptr);
            AM_EXPECT(stats.currentMemoryUsed.load() >= baseline + 4096);
            AM_EXPECT(amMemory->TotalReservedMemorySize(pool) == stats.currentMemoryUsed.load());

            AmVoidPtr b = amMemory->Malign(pool, 8192, 64, __FILE__, __LINE__);
            AM_EXPECT(b != nullptr);
            AM_EXPECT(stats.currentMemoryUsed.load() >= baseline + 4096 + 8192);

            // The peak is a high-water mark: it survives the frees
            const AmSize peak = stats.maxMemoryUsed.load();
            AM_EXPECT(peak >= stats.currentMemoryUsed.load());

            amMemory->Free(pool, a);
            amMemory->Free(pool, b);
            AM_EXPECT(stats.currentMemoryUsed.load() == baseline);
            AM_EXPECT(stats.maxMemoryUsed.load() >= peak);

            // Growing and shrinking a block adjusts the current usage without counting extra allocations
            AmVoidPtr r = amMemory->Malloc(pool, 128, __FILE__, __LINE__);
            const AmUInt64 allocs = stats.allocCount.load();
            r = amMemory->Realloc(pool, r, 1 << 20, __FILE__, __LINE__);
            AM_EXPECT(r != nullptr);
            AM_EXPECT(stats.allocCount.load() == allocs);
            AM_EXPECT(stats.currentMemoryUsed.load() >= baseline + (1 << 20));

            r = amMemory->Realloc(pool, r, 64, __FILE__, __LINE__);
            AM_EXPECT(r != nullptr);
            AM_EXPECT(stats.currentMemoryUsed.load() < baseline + (1 << 20));

            amMemory->Free(pool, r);
            AM_EXPECT(stats.currentMemoryUsed.load() == baseline);

            // A realloc to zero releases the block, and a failed/null free changes nothing
            AmVoidPtr z = amMemory->Malloc(pool, 256, __FILE__, __LINE__);
            AM_EXPECT(amMemory->Realloc(pool, z, 0, __FILE__, __LINE__) == nullptr);
            AM_EXPECT(stats.currentMemoryUsed.load() == baseline);

            const AmUInt64 frees = stats.freeCount.load();
            amMemory->Free(pool, nullptr);
            AM_EXPECT(stats.freeCount.load() == frees);
            AM_EXPECT(stats.currentMemoryUsed.load() == baseline);

            // Concurrent balanced allocations leave no residue in the counters
            constexpr int threadCount = 4;
            constexpr int iterations = 2000;
            std::vector<std::thread> threads;
            for (int t = 0; t < threadCount; ++t)
            {
                threads.emplace_back(
                    []
                    {
                        for (int i = 0; i < iterations; ++i)
                        {
                            AmVoidPtr p = amMemory->Malloc(pool, 64 + (i % 7) * 32, __FILE__, __LINE__);
                            p = amMemory->Realloc(pool, p, 512, __FILE__, __LINE__);
                            amMemory->Free(pool, p);
                        }
                    });
            }

            for (auto& thread : threads)
                thread.join();

            AM_EXPECT(stats.currentMemoryUsed.load() == baseline);
            AM_EXPECT(amMemory->TotalReservedMemorySize() == [&] {
                AmSize total = 0;
                for (AmUInt32 k = 0; k < eMemoryPoolKind_COUNT; ++k)
                    total += amMemory->GetStats(static_cast<eMemoryPoolKind>(k)).currentMemoryUsed.load();
                return total;
            }());
#endif

            MemoryManager::Deinitialize();
        }
    };

    AM_REGISTER_TEST(core_memory, memory_usage_counters);
} // namespace SparkyStudios::Audio::Amplitude::Tests
