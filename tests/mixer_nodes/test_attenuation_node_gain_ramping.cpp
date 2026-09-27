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

#include <DSP/Gain.h>
#include <Mixer/Nodes/AttenuationNode.h>

#include "NodeTestCase.h"
#include "TestRegistry.h"
#include "TestUtils.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        constexpr AmUInt64 kFrames = 1024;
        constexpr AmUInt32 kSampleRate = 48000;
        constexpr AmReal32 kTolerance = 1e-3f;

        // MockAttenuation(100): gain = 1 - distance / 100.
        AmReal32 GainAt(AmReal32 distance)
        {
            return std::max(0.0f, 1.0f - distance / 100.0f);
        }

        AmReal32 MaxStep(const AudioBuffer& buffer)
        {
            AmReal32 maxStep = 0.0f;
            for (AmUInt64 i = 1; i < buffer.GetFrameCount(); ++i)
                maxStep = std::max(maxStep, std::abs(buffer[0][i] - buffer[0][i - 1]));
            return maxStep;
        }
    } // namespace

    class AttenuationRampTestCase : public NodeTestCase
    {
    protected:
        void SetUpScene()
        {
            GetMockLayer().SetSampleRate(kSampleRate);
            GetMockLayer().SetSpatialization(eSpatialization_Position);

            _listenerState.SetLocation({ 0.0f, 0.0f, 0.0f });
            _listenerState.SetOrientation(Orientation({ 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }));
            _listenerState.Update();
            GetMockLayer().SetListener(InitTestListener(&_listenerState));
            GetMockLayer().SetAttenuation(&_attenuation);

            _input = AudioBuffer(kFrames, 1);
            for (AmUInt64 i = 0; i < kFrames; ++i)
                _input[0][i] = 1.0f; // DC: output equals the applied gain
        }

        const AudioBuffer* ProcessAt(ProcessorNodeInstance* processor, AmReal32 distance)
        {
            GetMockLayer().SetLocation({ 0.0f, 0.0f, -distance });

            // The mixer resets every node's memoized state after each block (see
            // PipelineInstanceImpl::Reset()); mirror that here so the fixture matches production.
            processor->Reset();
            return processor->Process(&_input);
        }

        // _input must release its allocation before the memory manager is torn down: the test
        // case object itself outlives TearDown(), so its member destructors run after Deinitialize().
        void TearDown() override
        {
            {
                AudioBuffer discard(std::move(_input));
            }

            NodeTestCase::TearDown();
        }

        ListenerInternalState _listenerState;
        MockAttenuation _attenuation{ 100.0 };
        AudioBuffer _input;
    };

    AM_TEST_CASE(AttenuationRampTestCase, mixer_nodes, attenuation_node_distance_change_is_continuous)
    {
    public:
        void Run() override
        {
            SetUpScene();
            auto instance = CreateAndConfigureNode<AttenuationNode>(kFrames, 1);
            auto* processor = AsProcessor(instance);

            const AudioBuffer* near = ProcessAt(processor, 5.0f);
            AM_EXPECT_NOT(near == nullptr);
            const AmReal32 lastNear = (*near)[0][kFrames - 1];
            AM_EXPECT(std::abs(lastNear - GainAt(5.0f)) < kTolerance); // first block snaps

            const AudioBuffer* far = ProcessAt(processor, 50.0f);
            AM_EXPECT_NOT(far == nullptr);

            const AmReal32 delta = GainAt(5.0f) - GainAt(50.0f);
            const AmReal32 slope = delta / static_cast<AmReal32>(kFrames);

            // No step at the block boundary, and a linear ramp across the block.
            AM_EXPECT(std::abs((*far)[0][0] - lastNear) <= slope + kTolerance);
            AM_EXPECT(MaxStep(*far) <= slope + kTolerance);
            AM_EXPECT(std::abs((*far)[0][kFrames - 1] - (GainAt(50.0f) + slope)) < kTolerance);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, attenuation_node_distance_change_is_continuous);

    AM_TEST_CASE(AttenuationRampTestCase, mixer_nodes, attenuation_node_fades_out_before_culling)
    {
    public:
        void Run() override
        {
            SetUpScene();
            auto instance = CreateAndConfigureNode<AttenuationNode>(kFrames, 1);
            auto* processor = AsProcessor(instance);

            AM_EXPECT_NOT(ProcessAt(processor, 5.0f) == nullptr);

            // Beyond max distance: the first silent block still renders, ramping to 0.
            const AudioBuffer* fading = ProcessAt(processor, 150.0f);
            AM_EXPECT_NOT(fading == nullptr);
            AM_EXPECT(std::abs((*fading)[0][0] - GainAt(5.0f)) < kTolerance);
            AM_EXPECT((*fading)[0][kFrames - 1] < 0.01f);
            AM_EXPECT(MaxStep(*fading) <= GainAt(5.0f) / static_cast<AmReal32>(kFrames) + kTolerance);

            // Once the ramp has landed on 0, the node culls.
            AM_EXPECT(ProcessAt(processor, 150.0f) == nullptr);

            // Coming back fades in from 0 instead of jumping.
            const AudioBuffer* back = ProcessAt(processor, 5.0f);
            AM_EXPECT_NOT(back == nullptr);
            AM_EXPECT((*back)[0][0] < 0.01f);
            AM_EXPECT(MaxStep(*back) <= GainAt(5.0f) / static_cast<AmReal32>(kFrames) + kTolerance);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, attenuation_node_fades_out_before_culling);

    AM_TEST_CASE(AttenuationRampTestCase, mixer_nodes, attenuation_node_first_silent_block_then_fades_in)
    {
    public:
        void Run() override
        {
            SetUpScene();
            auto instance = CreateAndConfigureNode<AttenuationNode>(kFrames, 1);
            auto* processor = AsProcessor(instance);

            // Very first block is inaudible: culled immediately.
            AM_EXPECT(ProcessAt(processor, 150.0f) == nullptr);

            // Becoming audible fades in from 0.
            const AudioBuffer* in = ProcessAt(processor, 5.0f);
            AM_EXPECT_NOT(in == nullptr);
            AM_EXPECT((*in)[0][0] < 0.01f);
            AM_EXPECT(MaxStep(*in) <= GainAt(5.0f) / static_cast<AmReal32>(kFrames) + kTolerance);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, attenuation_node_first_silent_block_then_fades_in);

    AM_TEST_CASE(AttenuationRampTestCase, mixer_nodes, attenuation_node_separate_mode_snaps_per_instance)
    {
    public:
        void Run() override
        {
            SetUpScene();
            auto instance = CreateAndConfigureNode<AttenuationNode>(kFrames, 1);
            auto* processor = AsProcessor(instance);

            // A non-empty instance list puts the mock in separate-mode instancing, matching the
            // condition MixLayerSeparateMode() runs under. IsMultiPosition() stays false, so the
            // gain still comes from SetLocation()/GetLocation(), not from the instance list itself.
            GetMockLayer().AddInstance({ 0.0f, 0.0f, -5.0f }, 1.0f, 1.0f);

            AM_EXPECT_NOT(ProcessAt(processor, 5.0f) == nullptr);

            // Separate-mode instances share this node within a block: the next instance snaps
            // instead of ramping from the previous instance's gain.
            const AudioBuffer* other = ProcessAt(processor, 50.0f);
            AM_EXPECT_NOT(other == nullptr);
            for (AmUInt64 i = 0; i < kFrames; ++i)
                AM_EXPECT(std::abs((*other)[0][i] - GainAt(50.0f)) < kTolerance);

            GetMockLayer().ClearInstances();
        }
    };

    AM_REGISTER_TEST(mixer_nodes, attenuation_node_separate_mode_snaps_per_instance);
} // namespace SparkyStudios::Audio::Amplitude::Tests
