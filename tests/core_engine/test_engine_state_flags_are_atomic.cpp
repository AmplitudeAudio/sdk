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

#include <algorithm>
#include <atomic>
#include <chrono>
#include <thread>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, core_engine, engine_state_flags_are_atomic)
    {
    public:
        void Run() override
        {
            amEngine->Pause(true);
            AM_EXPECT(amEngine->IsPaused());
            amEngine->Pause(false);
            AM_EXPECT_NOT(amEngine->IsPaused());

            amEngine->SetMasterGain(0.25f);
            AM_EXPECT(amEngine->GetMasterGain() == 0.25f);

            // Cross-thread smoke: a reader observes only sane values while the main thread mutates
            std::atomic<bool> done{ false };
            std::atomic<bool> started{ false };
            std::atomic<AmReal32> observedMin{ 1.0f };
            std::atomic<AmReal32> observedMax{ 0.0f };

            std::thread reader(
                [&done, &started, &observedMin, &observedMax]()
                {
                    started.store(true, std::memory_order_release);

                    while (!done.load(std::memory_order_relaxed))
                    {
                        const AmReal32 g = amEngine->GetMasterGain();
                        observedMin.store(std::min(observedMin.load(std::memory_order_relaxed), g), std::memory_order_relaxed);
                        observedMax.store(std::max(observedMax.load(std::memory_order_relaxed), g), std::memory_order_relaxed);
                    }
                });

            // Wait for the reader to start before mutating, so it cannot miss the whole sweep
            while (!started.load(std::memory_order_acquire))
                std::this_thread::yield();

            for (AmUInt32 i = 0; i <= 100; ++i)
            {
                amEngine->SetMasterGain(static_cast<AmReal32>(i) / 100.0f);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            done.store(true);
            reader.join();

            AM_EXPECT(observedMin.load() >= 0.0f);
            AM_EXPECT(observedMax.load() <= 1.0f);
            AM_EXPECT(observedMax.load() > 0.0f);

            amEngine->SetMasterGain(1.0f);
        }
    };

    AM_REGISTER_TEST(core_engine, engine_state_flags_are_atomic);
} // namespace SparkyStudios::Audio::Amplitude::Tests
