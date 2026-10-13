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

#include <chrono>
#include <cstdlib>
#include <functional>
#include <future>
#include <iterator>
#include <memory>
#include <thread>
#include <unordered_set>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Sound/Schedulers/RandomScheduler.h>
#include <Sound/Schedulers/SequenceScheduler.h>

#include "collection_definition_generated.h"
#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        constexpr AmSoundID kSounds[] = { 9991, 9992, 9993 };

        // Runs the callable on a helper thread. A hang is reported as a timeout and the thread is abandoned, so a
        // regression fails the test instead of blocking the whole suite. On a regression the detached worker keeps
        // spinning until the process exits: this is accepted, the test fails instead of hanging.
        bool RunBounded(std::function<void()> work)
        {
            auto promise = std::make_shared<std::promise<void>>();
            std::future<void> done = promise->get_future();

            std::thread(
                [promise, work = std::move(work)]
                {
                    work();
                    promise->set_value();
                })
                .detach();

            return done.wait_for(std::chrono::seconds(2)) == std::future_status::ready;
        }

        // Owns the flatbuffer a scheduler reads from, so the buffer outlives every Select call.
        struct SchedulerFixture
        {
            flatbuffers::FlatBufferBuilder builder;
            std::unique_ptr<RandomScheduler> scheduler;

            explicit SchedulerFixture(const RandomSoundSchedulerConfig* config, const size_t soundCount = std::size(kSounds))
            {
                std::vector<flatbuffers::Offset<void>> entries;
                std::vector<uint8_t> types;
                for (size_t i = 0; i < soundCount; ++i)
                {
                    const AmSoundID id = kSounds[i];
                    entries.push_back(CreateRandomSchedulerCollectionEntry(builder, id, 0, 0, 1.0f).Union());
                    types.push_back(CollectionEntry_Random);
                }

                const auto name = builder.CreateString("random_scheduler_test");
                const auto typesVec = builder.CreateVector(types);
                const auto entriesVec = builder.CreateVector(entries);

                CollectionDefinitionBuilder collection(builder);
                collection.add_id(424242);
                collection.add_name(name);
                collection.add_sounds_type(typesVec);
                collection.add_sounds(entriesVec);
                builder.Finish(collection.Finish());

                scheduler = std::make_unique<RandomScheduler>(config);
                scheduler->Init(flatbuffers::GetRoot<CollectionDefinition>(builder.GetBufferPointer()));
            }
        };
    } // namespace

    AM_TEST_CASE(EngineTestCase, core_sound_scheduler, random_scheduler_always_terminates)
    {
    public:
        void Run() override
        {
            // Every sound skipped: no candidate, must return nullptr instead of looping forever.
            {
                auto fixture = std::make_shared<SchedulerFixture>(nullptr);
                auto result = std::make_shared<Sound*>(reinterpret_cast<Sound*>(1));
                AM_EXPECT(RunBounded(
                    [fixture, result]
                    {
                        *result = fixture->scheduler->Select({ kSounds[0], kSounds[1], kSounds[2] });
                    }));
                AM_EXPECT_EQ(*result, nullptr);
            }

            // Null config: treated as avoid_repeat = false, always yields a sound.
            {
                auto fixture = std::make_shared<SchedulerFixture>(nullptr);
                auto picked = std::make_shared<int>(0);
                AM_EXPECT(RunBounded(
                    [fixture, picked]
                    {
                        for (int i = 0; i < 50; ++i)
                            *picked += fixture->scheduler->Select({}) != nullptr ? 1 : 0;
                    }));
                AM_EXPECT_EQ(*picked, 50);
            }

            // avoid_repeat with a repeat count >= the number of sounds: always yields a sound.
            {
                // Shared with the helper thread so the config outlives a hung Select.
                auto configBuilder = std::make_shared<flatbuffers::FlatBufferBuilder>();
                configBuilder->Finish(CreateRandomSoundSchedulerConfig(*configBuilder, true, 10));
                const auto* config = flatbuffers::GetRoot<RandomSoundSchedulerConfig>(configBuilder->GetBufferPointer());

                auto fixture = std::make_shared<SchedulerFixture>(config);
                auto picked = std::make_shared<int>(0);
                AM_EXPECT(RunBounded(
                    [fixture, picked, configBuilder]
                    {
                        for (int i = 0; i < 50; ++i)
                            *picked += fixture->scheduler->Select({}) != nullptr ? 1 : 0;
                    }));
                AM_EXPECT_EQ(*picked, 50);
            }

            // Relaxed avoid-repeat (repeat count >= sound count) never repeats the sound just played.
            {
                auto configBuilder = std::make_shared<flatbuffers::FlatBufferBuilder>();
                configBuilder->Finish(CreateRandomSoundSchedulerConfig(*configBuilder, true, 3));
                const auto* config = flatbuffers::GetRoot<RandomSoundSchedulerConfig>(configBuilder->GetBufferPointer());

                auto fixture = std::make_shared<SchedulerFixture>(config);
                auto result = std::make_shared<std::pair<int, int>>(0, 0); // { sounds returned, immediate repeats }
                AM_EXPECT(RunBounded(
                    [fixture, result, configBuilder]
                    {
                        std::srand(12345);
                        const Sound* previous = nullptr;
                        for (int i = 0; i < 300; ++i)
                        {
                            const Sound* sound = fixture->scheduler->Select({});
                            result->first += sound != nullptr ? 1 : 0;
                            result->second += sound != nullptr && sound == previous ? 1 : 0;
                            previous = sound;
                        }
                    }));
                AM_EXPECT_EQ(result->first, 300);
                AM_EXPECT_EQ(result->second, 0);
            }

            // A single sound with avoid_repeat still plays every call: an immediate repeat is the only choice.
            {
                auto configBuilder = std::make_shared<flatbuffers::FlatBufferBuilder>();
                configBuilder->Finish(CreateRandomSoundSchedulerConfig(*configBuilder, true, 3));
                const auto* config = flatbuffers::GetRoot<RandomSoundSchedulerConfig>(configBuilder->GetBufferPointer());

                auto fixture = std::make_shared<SchedulerFixture>(config, 1);
                auto picked = std::make_shared<int>(0);
                AM_EXPECT(RunBounded(
                    [fixture, picked, configBuilder]
                    {
                        for (int i = 0; i < 20; ++i)
                            *picked += fixture->scheduler->Select({}) != nullptr ? 1 : 0;
                    }));
                AM_EXPECT_EQ(*picked, 20);
            }
        }
    };

    AM_REGISTER_TEST(core_sound_scheduler, random_scheduler_always_terminates);
} // namespace SparkyStudios::Audio::Amplitude::Tests
