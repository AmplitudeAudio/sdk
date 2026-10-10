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
    AM_TEST_CASE(EngineTestCase, mixer_pipeline, mixer_refuses_mismatched_providers)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frames = 128;
            AmplimixLayerImpl layer;

            PipelineImpl pipeline;
            pipeline.LoadDefinitionFromFile(
                _fileSystem->OpenFile(
                    _fileSystem->Join({ AM_OS_STRING("pipelines"), AM_OS_STRING("tests.mismatched_mixer.ampipeline") }),
                    eFileOpenMode_Read),
                nullptr);

            auto instance = pipeline.CreateInstance(&layer);
            AM_EXPECT_NOT(instance == nullptr);
            if (instance == nullptr)
                return;

            // The mixer sums the stereo input with the probe's mono fold-down: refused.
            AM_EXPECT_NOT(instance->Configure(frames, 2, frames, 2));

            // A refused pipeline renders silence and never crashes, and asking again gives the same answer.
            AudioBuffer input(frames, 2);
            for (AmUInt64 i = 0; i < frames; ++i)
                input[0][i] = input[1][i] = 1.0f;

            AudioBuffer output(frames, 2);
            output[0][0] = 3.0f;
            instance->Execute(input, output);
            AM_EXPECT_EQ(0.0f, output[0][0]);
            AM_EXPECT_NOT(instance->Configure(frames, 2, frames, 2));

            // A mono source has nothing to fold: both providers are mono, so it configures.
            AM_EXPECT(instance->Configure(frames, 1, frames, 1));
        }
    };

    AM_REGISTER_TEST(mixer_pipeline, mixer_refuses_mismatched_providers);
} // namespace SparkyStudios::Audio::Amplitude::Tests
