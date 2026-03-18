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
#include <Mixer/Nodes/LimiterNode.h>

#include "NodeTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(NodeTestCase, mixer_nodes, limiter_node_limits_loud_signal)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 2048;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            GetMockLayer().SetSampleRate(sampleRate);

            auto instance = CreateAndConfigureNode<LimiterNode>(frameCount, channelCount);
            auto* processor = AsProcessor(instance);
            AM_EXPECT_NOT(processor == nullptr);

            instance->SetParameter(LimiterNodeInstance::ATTRIBUTE_THRESHOLD_DB, -6.0f);
            instance->SetParameter(LimiterNodeInstance::ATTRIBUTE_ATTACK_MS, 1.0f);
            instance->SetParameter(LimiterNodeInstance::ATTRIBUTE_RELEASE_MS, 50.0f);
            instance->Reset();

            AudioBuffer input(frameCount, channelCount);
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = std::sin(2.0f * AM_PI32 * 440.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate));

            const AudioBuffer* output = processor->Process(&input);
            AM_EXPECT_NOT(output == nullptr);

            AmReal32 outputPeak = 0.0f;
            for (AmUInt64 i = frameCount / 2; i < frameCount; ++i)
                outputPeak = std::max(outputPeak, std::abs((*output)[0][i]));

            AM_EXPECT(outputPeak < 0.7f);
            AM_EXPECT(outputPeak > 0.1f);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, limiter_node_limits_loud_signal);
} // namespace SparkyStudios::Audio::Amplitude::Tests
