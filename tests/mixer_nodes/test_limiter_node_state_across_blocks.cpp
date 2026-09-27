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

#include <cmath>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/Nodes/LimiterNode.h>

#include "NodeTestCase.h"
#include "TestRegistry.h"
#include "TestUtils.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        constexpr AmUInt64 kFrames = 512;
        constexpr AmUInt32 kSampleRate = 48000;

        void Fill(AudioBuffer& buffer, AmReal32 value)
        {
            for (AmUInt64 i = 0; i < buffer.GetFrameCount(); ++i)
                buffer[0][i] = value;
        }
    } // namespace

    AM_TEST_CASE(NodeTestCase, mixer_nodes, limiter_node_envelope_survives_reset)
    {
    public:
        void Run() override
        {
            GetMockLayer().SetSampleRate(kSampleRate);

            auto instance = CreateAndConfigureNode<LimiterNode>(kFrames, 1);
            auto* processor = AsProcessor(instance);
            AM_EXPECT_NOT(processor == nullptr);

            instance->SetParameter(LimiterNodeInstance::ATTRIBUTE_THRESHOLD_DB, -6.0f);
            instance->SetParameter(LimiterNodeInstance::ATTRIBUTE_ATTACK_MS, 1.0f);
            instance->SetParameter(LimiterNodeInstance::ATTRIBUTE_RELEASE_MS, 500.0f);

            AudioBuffer loud(kFrames, 1);
            Fill(loud, 1.0f);
            AudioBuffer quiet(kFrames, 1);
            Fill(quiet, 0.4f);

            // Block 1: a loud DC signal drives the envelope up and the gain down to ~threshold.
            instance->Reset();
            const AudioBuffer* first = processor->Process(&loud);
            AM_EXPECT_NOT(first == nullptr);
            const AmReal32 lastGain = (*first)[0][kFrames - 1] / 1.0f;
            AM_EXPECT(lastGain < 0.6f);

            // Block 2: the mixer calls Reset() before every block. With a 500 ms release the envelope is
            // still high, so the quiet signal keeps being limited with the same gain — no jump.
            instance->Reset();
            const AudioBuffer* second = processor->Process(&quiet);
            AM_EXPECT_NOT(second == nullptr);
            const AmReal32 firstGain = (*second)[0][0] / 0.4f;
            AM_EXPECT(std::abs(firstGain - lastGain) < 0.02f);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, limiter_node_envelope_survives_reset);

    AM_TEST_CASE(NodeTestCase, mixer_nodes, limiter_node_attack_change_takes_effect)
    {
    public:
        void Run() override
        {
            GetMockLayer().SetSampleRate(kSampleRate);

            AudioBuffer silence(kFrames, 1);
            Fill(silence, 0.0f);
            AudioBuffer loud(kFrames, 1);
            Fill(loud, 1.0f);

            // Reference: fast attack from the start.
            auto reference = CreateAndConfigureNode<LimiterNode>(kFrames, 1);
            reference->SetParameter(LimiterNodeInstance::ATTRIBUTE_THRESHOLD_DB, -6.0f);
            reference->SetParameter(LimiterNodeInstance::ATTRIBUTE_ATTACK_MS, 1.0f);
            reference->SetParameter(LimiterNodeInstance::ATTRIBUTE_RELEASE_MS, 50.0f);
            reference->Reset();
            AM_EXPECT_NOT(AsProcessor(reference)->Process(&silence) == nullptr);

            // Under test: slow attack, one block processed (coefficients computed), then switched to fast.
            auto changed = CreateAndConfigureNode<LimiterNode>(kFrames, 1);
            changed->SetParameter(LimiterNodeInstance::ATTRIBUTE_THRESHOLD_DB, -6.0f);
            changed->SetParameter(LimiterNodeInstance::ATTRIBUTE_ATTACK_MS, 1000.0f);
            changed->SetParameter(LimiterNodeInstance::ATTRIBUTE_RELEASE_MS, 50.0f);
            changed->Reset();
            AM_EXPECT_NOT(AsProcessor(changed)->Process(&silence) == nullptr);
            changed->SetParameter(LimiterNodeInstance::ATTRIBUTE_ATTACK_MS, 1.0f);

            reference->Reset();
            const AudioBuffer* expected = AsProcessor(reference)->Process(&loud);
            AmReal32 expectedLast = (*expected)[0][kFrames - 1];

            changed->Reset();
            const AudioBuffer* actual = AsProcessor(changed)->Process(&loud);
            AM_EXPECT(std::abs((*actual)[0][kFrames - 1] - expectedLast) < 1e-4f);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, limiter_node_attack_change_takes_effect);
} // namespace SparkyStudios::Audio::Amplitude::Tests
