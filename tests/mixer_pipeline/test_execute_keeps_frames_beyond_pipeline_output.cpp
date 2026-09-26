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
    /**
     * @brief When the pipeline produces fewer frames than the caller's output buffer holds,
     * the produced frames are copied, the remaining frames are left untouched, and the
     * output buffer keeps its own frame count.
     */
    AM_TEST_CASE(EngineTestCase, mixer_pipeline, execute_keeps_frames_beyond_pipeline_output)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 inputFrameCount = 256;
            constexpr AmUInt64 outputFrameCount = 512;
            constexpr AmUInt16 monoChannels = 1;
            constexpr AmReal32 sentinel = 0.25f;

            AmplimixLayerImpl layer;

            PipelineImpl pipeline;
            pipeline.LoadDefinitionFromFile(
                _fileSystem->OpenFile(
                    _fileSystem->Join({ AM_OS_STRING("pipelines"), AM_OS_STRING("tests.mono_passthrough.ampipeline") }),
                    eFileOpenMode_Read),
                nullptr);

            auto instance = pipeline.CreateInstance(&layer);
            AM_EXPECT_NOT(instance == nullptr);
            if (instance == nullptr)
                return;

            AudioBuffer input(inputFrameCount, monoChannels);
            for (AmUInt64 i = 0; i < inputFrameCount; ++i)
                input[0][i] = 0.5f + static_cast<AmReal32>(i) / static_cast<AmReal32>(2 * inputFrameCount);

            AudioBuffer output(outputFrameCount, monoChannels);
            for (AmUInt64 i = 0; i < outputFrameCount; ++i)
                output[0][i] = sentinel;

            instance->Execute(input, output);

            AM_EXPECT_EQ(output.GetChannelCount(), monoChannels);
            AM_EXPECT_EQ(output.GetFrameCount(), outputFrameCount);
            if (output.GetChannelCount() != monoChannels || output.GetFrameCount() != outputFrameCount)
                return;

            for (AmUInt64 i = 0; i < inputFrameCount; ++i)
                AM_EXPECT_EQ(output[0][i], input[0][i]);

            // Frames beyond what the pipeline produced are not written.
            for (AmUInt64 i = inputFrameCount; i < outputFrameCount; ++i)
                AM_EXPECT_EQ(output[0][i], sentinel);
        }
    };

    AM_REGISTER_TEST(mixer_pipeline, execute_keeps_frames_beyond_pipeline_output);
} // namespace SparkyStudios::Audio::Amplitude::Tests
