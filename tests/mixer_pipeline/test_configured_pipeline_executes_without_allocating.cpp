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

#include <Mixer/Pipeline.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // Configured on the game thread, a pipeline executed with the same dimensions allocates nothing: no lazy
    // reconfiguration happens on the audio thread, fold-down buffers included.
    AM_TEST_CASE(EngineTestCase, mixer_pipeline, configured_pipeline_executes_without_allocating)
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
            AmplimixLayerImpl layer;

            for (const AmOsString name : { AM_OS_STRING("tests.mono_probe.ampipeline"), AM_OS_STRING("tests.custom_then_mono_probe.ampipeline") })
            {
                PipelineImpl pipeline;
                pipeline.LoadDefinitionFromFile(
                    _fileSystem->OpenFile(_fileSystem->Join({ AM_OS_STRING("pipelines"), name }), eFileOpenMode_Read), nullptr);

                auto instance = pipeline.CreateInstance(&layer);
                AM_EXPECT_NOT(instance == nullptr);
                if (instance == nullptr)
                    return;

                const AmUInt16 outputChannels = name == AM_OS_STRING("tests.mono_probe.ampipeline") ? 1 : 6;
                AM_EXPECT(instance->Configure(frames, 6, frames, outputChannels));

                AudioBuffer input(frames, 6);
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
#else
            AM_EXPECT(false);
#endif
        }
    };

    AM_REGISTER_TEST(mixer_pipeline, configured_pipeline_executes_without_allocating);
} // namespace SparkyStudios::Audio::Amplitude::Tests
