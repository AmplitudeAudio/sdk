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

#include <Core/Engine.h>
#include <Mixer/Amplimix.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, core_engine, mixer_instance_pipeline_api_rejects_invalid_layer)
    {
    public:
        void Run() override
        {
            AmplimixImpl& mixer = amEngine->GetState()->mixer;

            // Absent from the test engine config, so the schema default applies.
            AM_EXPECT_EQ(mixer.GetMaxInstancePipelines(), AmplimixImpl::kDefaultMaxInstancePipelines);

            // No sound plays on this made-up channel/layer pair: every call is rejected on the game thread.
            constexpr AmUInt32 kUnknownChannel = 999999;
            constexpr AmUInt32 kUnknownLayer = 4095;
            AM_EXPECT_NOT(mixer.InstallInstancePipelineTable(kUnknownChannel, kUnknownLayer));
            AM_EXPECT_NOT(mixer.AttachInstancePipeline(kUnknownChannel, kUnknownLayer, 1));
            AM_EXPECT_NOT(mixer.DetachInstancePipeline(kUnknownChannel, kUnknownLayer, 1));

            // A zero cap disables per-instance pipelines entirely.
            const AmSize previous = mixer.GetMaxInstancePipelines();
            mixer.SetMaxInstancePipelines(0);
            AM_EXPECT_EQ(mixer.GetMaxInstancePipelines(), 0);
            AM_EXPECT_NOT(mixer.InstallInstancePipelineTable(kUnknownChannel, kUnknownLayer));
            mixer.SetMaxInstancePipelines(previous);
        }
    };

    AM_REGISTER_TEST(core_engine, mixer_instance_pipeline_api_rejects_invalid_layer);
} // namespace SparkyStudios::Audio::Amplitude::Tests
