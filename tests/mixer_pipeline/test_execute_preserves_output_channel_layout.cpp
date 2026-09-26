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
     * @brief Executing a pipeline whose graph produces the channel count of the caller's
     * output buffer copies the result verbatim and keeps the caller's layout.
     *
     * The graph is a plain Input -> Output passthrough, so a mono input yields a mono
     * result, which matches the mono output buffer.
     */
    AM_TEST_CASE(EngineTestCase, mixer_pipeline, execute_preserves_output_channel_layout)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 256;
            constexpr AmUInt16 monoChannels = 1;

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

            AudioBuffer input(frameCount, monoChannels);
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = static_cast<AmReal32>(i) / static_cast<AmReal32>(frameCount);

            AudioBuffer output(frameCount, monoChannels);
            instance->Execute(input, output);

            AM_EXPECT_EQ(output.GetChannelCount(), monoChannels);
            AM_EXPECT_EQ(output.GetFrameCount(), frameCount);
            if (output.GetChannelCount() != monoChannels || output.GetFrameCount() != frameCount)
                return;

            for (AmUInt64 i = 0; i < frameCount; ++i)
                AM_EXPECT_EQ(output[0][i], input[0][i]);
        }
    };

    AM_REGISTER_TEST(mixer_pipeline, execute_preserves_output_channel_layout);
} // namespace SparkyStudios::Audio::Amplitude::Tests
