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

#include <Core/ListenerInternalState.h>
#include <Mixer/Pipeline.h>

#include "EngineTestCase.h"
#include "NodeTestCase.h"
#include "TestRegistry.h"
#include "TestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // The shipped default and surround graphs, configured on the game thread for every supported source and output
    // channel count, execute without touching the allocator.
    AM_TEST_CASE(EngineTestCase, mixer_pipeline, real_pipelines_execute_without_allocating)
    {
    public:
        void Run() override
        {
#if !defined(AM_NO_MEMORY_STATS)
            const auto allocations = []()
            {
                AmUInt64 total = 0;
                for (AmUInt32 k = 0; k < eMemoryPoolKind_COUNT; ++k)
                    total += amMemory->GetStats(static_cast<eMemoryPoolKind>(k)).allocCount.load();
                return total;
            };

            constexpr AmUInt64 frames = 512;

            // A bare layer has no sound and no listener, so most nodes would take their silent shortcut. This one has
            // a sound, a listener and a source 0.5 m away, which wakes the spatial and near-field paths.
            ListenerInternalState listenerState;
            listenerState.SetLocation({ 0.0f, 0.0f, 0.0f });
            listenerState.SetOrientation(Orientation({ 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }));
            listenerState.Update();

            MockSound sound(1.0f);

            SpatialMockLayer layer;
            layer.SetSampleRate(48000);
            layer.SetSpatialization(eSpatialization_Position);
            layer.SetListener(InitTestListener(&listenerState));
            layer.SetLocation({ 0.3f, 0.0f, -0.4f });
            layer.SetSound(&sound);

            for (const AmOsString name : { AM_OS_STRING("default.ampipeline"), AM_OS_STRING("surround.ampipeline") })
            {
                PipelineImpl pipeline;
                pipeline.LoadDefinitionFromFile(
                    _fileSystem->OpenFile(_fileSystem->Join({ AM_OS_STRING("pipelines"), name }), eFileOpenMode_Read), nullptr);

                for (const AmUInt16 inputChannels : { 1, 2, 6, 8 })
                {
                    for (const AmUInt16 outputChannels : { 2, 6 })
                    {
                        auto instance = pipeline.CreateInstance(&layer);
                        AM_EXPECT_NOT(instance == nullptr);
                        if (instance == nullptr)
                            return;

                        AM_EXPECT(instance->Configure(frames, inputChannels, frames, outputChannels));

                        AudioBuffer input(frames, inputChannels);
                        AudioBuffer output(frames, outputChannels);
                        instance->Execute(input, output);
                        instance->Reset();

                        const AmUInt64 before = allocations();
                        for (AmUInt32 i = 0; i < 100; ++i)
                        {
                            instance->Execute(input, output);
                            instance->Reset();
                        }
                        AM_EXPECT_EQ(before, allocations());
                    }
                }
            }
#else
            AM_EXPECT(false);
#endif
        }
    };

    AM_REGISTER_TEST(mixer_pipeline, real_pipelines_execute_without_allocating);
} // namespace SparkyStudios::Audio::Amplitude::Tests
