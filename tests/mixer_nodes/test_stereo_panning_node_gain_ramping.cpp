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
#include <Mixer/Nodes/StereoPanningNode.h>

#include "NodeTestCase.h"
#include "TestRegistry.h"
#include "TestUtils.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        constexpr AmUInt64 kFrames = 256;
        constexpr AmReal32 kTolerance = 1e-3f;
    } // namespace

    class PanningRampTestCase : public NodeTestCase
    {
    protected:
        void SetUpScene()
        {
            GetMockLayer().SetSampleRate(48000);
            GetMockLayer().SetSpatialization(eSpatialization_Position);

            _listenerState.SetLocation({ 0.0f, 0.0f, 0.0f });
            _listenerState.SetOrientation(Orientation({ 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }));
            _listenerState.Update();
            GetMockLayer().SetListener(InitTestListener(&_listenerState));

            _input = AudioBuffer(kFrames, 1);
            for (AmUInt64 i = 0; i < kFrames; ++i)
                _input[0][i] = 1.0f;
        }

        AudioBuffer ProcessAt(ProcessorNodeInstance* processor, const AmVector3& location)
        {
            GetMockLayer().SetLocation(location);

            // The mixer resets every node's memoized state after each block (see
            // PipelineInstanceImpl::Reset()); mirror that here so the fixture matches production.
            processor->Reset();
            const AudioBuffer* output = processor->Process(&_input);
            AM_EXPECT_NOT(output == nullptr);

            AudioBuffer copy(kFrames, 2);
            AudioBuffer::Copy(*output, 0, copy, 0, kFrames);
            return copy;
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
        AudioBuffer _input;
    };

    AM_TEST_CASE(PanningRampTestCase, mixer_nodes, stereo_panning_node_pan_change_is_continuous)
    {
    public:
        void Run() override
        {
            SetUpScene();
            auto instance = CreateAndConfigureNode<StereoPanningNode>(kFrames, 1);
            auto* processor = AsProcessor(instance);

            const AudioBuffer left = ProcessAt(processor, { -10.0f, 0.0f, 0.0f });
            const AudioBuffer right = ProcessAt(processor, { 10.0f, 0.0f, 0.0f });

            // Ramp length = max(256, 240 frames @ 48 kHz) rounded to SIMD = 256 = the whole block.
            for (AmUInt16 c = 0; c < 2; ++c)
            {
                const AmReal32 slope = std::abs(right[c][kFrames - 1] - left[c][kFrames - 1]) / static_cast<AmReal32>(kFrames - 1);

                AM_EXPECT(std::abs(right[c][0] - left[c][kFrames - 1]) <= slope + kTolerance);
                for (AmUInt64 i = 1; i < kFrames; ++i)
                    AM_EXPECT(std::abs(right[c][i] - right[c][i - 1]) <= slope + kTolerance);
            }

            // Direction still correct at the end of the ramp.
            AM_EXPECT(right[1][kFrames - 1] > right[0][kFrames - 1]);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, stereo_panning_node_pan_change_is_continuous);

    AM_TEST_CASE(PanningRampTestCase, mixer_nodes, stereo_panning_node_separate_mode_keeps_ramp)
    {
    public:
        void Run() override
        {
            SetUpScene();
            auto instance = CreateAndConfigureNode<StereoPanningNode>(kFrames, 1);
            auto* processor = AsProcessor(instance);

            // Separate mode: each instance now has its own pipeline, so the pan keeps ramping.
            GetMockLayer().AddInstance({ -10.0f, 0.0f, 0.0f }, 1.0f, 1.0f);

            const AudioBuffer left = ProcessAt(processor, { -10.0f, 0.0f, 0.0f });
            const AudioBuffer right = ProcessAt(processor, { 10.0f, 0.0f, 0.0f });

            for (AmUInt16 c = 0; c < 2; ++c)
            {
                const AmReal32 slope = std::abs(right[c][kFrames - 1] - left[c][kFrames - 1]) / static_cast<AmReal32>(kFrames - 1);
                AM_EXPECT(std::abs(right[c][0] - left[c][kFrames - 1]) <= slope + kTolerance);
            }

            AM_EXPECT(right[1][kFrames - 1] > right[0][kFrames - 1]);

            GetMockLayer().ClearInstances();
        }
    };

    AM_REGISTER_TEST(mixer_nodes, stereo_panning_node_separate_mode_keeps_ramp);

    AM_TEST_CASE(PanningRampTestCase, mixer_nodes, stereo_panning_node_shared_instance_pipeline_snaps)
    {
    public:
        void Run() override
        {
            SetUpScene();
            auto instance = CreateAndConfigureNode<StereoPanningNode>(kFrames, 1);
            auto* processor = AsProcessor(instance);

            // Instances without their own pipeline share this node within a block: each one snaps to its own pan.
            GetMockLayer().SetSharingPipelineAcrossInstances(true);

            AM_UNUSED(ProcessAt(processor, { -10.0f, 0.0f, 0.0f }));
            const AudioBuffer right = ProcessAt(processor, { 10.0f, 0.0f, 0.0f });

            for (AmUInt16 c = 0; c < 2; ++c)
                for (AmUInt64 i = 1; i < kFrames; ++i)
                    AM_EXPECT(std::abs(right[c][i] - right[c][0]) < kTolerance);

            AM_EXPECT(right[1][0] > right[0][0]);

            GetMockLayer().SetSharingPipelineAcrossInstances(false);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, stereo_panning_node_shared_instance_pipeline_snaps);
} // namespace SparkyStudios::Audio::Amplitude::Tests
