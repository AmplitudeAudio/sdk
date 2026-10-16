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

#include <Mixer/Nodes/SpeakerPanningNode.h>

#include "NodeTestCase.h"
#include "TestRegistry.h"
#include "TestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(NodeTestCase, mixer_nodes, speaker_panning_node_pans_right_for_any_heading)
    {
    public:
        void Run() override
        {
            struct Case
            {
                AmVector3 forward;
                AmVector3 source;
            };

            // Listener at the origin, up (0,0,1). The source is placed on the listener's right: forward x up.
            const Case cases[] = {
                { { 0.0f, 1.0f, 0.0f }, { 10.0f, 0.0f, 0.0f } },
                { { 1.0f, 0.0f, 0.0f }, { 0.0f, -10.0f, 0.0f } },
                { { 0.0f, -1.0f, 0.0f }, { -10.0f, 0.0f, 0.0f } },
                { { -1.0f, 0.0f, 0.0f }, { 0.0f, 10.0f, 0.0f } },
            };

            for (const Case& c : cases)
            {
                AmReal32 leftEnergy = 0.0f, rightEnergy = 0.0f;
                Render(c.forward, c.source, leftEnergy, rightEnergy);
                AM_EXPECT(rightEnergy > 2.0f * leftEnergy);
            }
        }

    private:
        void Render(const AmVector3& forward, const AmVector3& source, AmReal32& leftEnergy, AmReal32& rightEnergy)
        {
            constexpr AmUInt64 frameCount = 256;
            constexpr AmUInt16 monoChannels = 1;

            ListenerInternalState listenerState;
            listenerState.SetLocation({ 0.0f, 0.0f, 0.0f });
            listenerState.SetOrientation(Orientation(forward, { 0.0f, 0.0f, 1.0f }));
            listenerState.Update();
            Listener listener = InitTestListener(&listenerState);
            GetMockLayer().SetListener(listener);

            GetMockLayer().SetSpatialization(eSpatialization_Position);
            GetMockLayer().SetLocation(source);

            auto instance = CreateAndConfigureNode<SpeakerPanningNode>(frameCount, monoChannels);
            auto* processor = AsProcessor(instance);
            AM_EXPECT_NOT(processor == nullptr);

            AudioBuffer input(frameCount, monoChannels);
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = 1.0f;

            const AudioBuffer* output = processor->Process(&input);
            AM_EXPECT_NOT(output == nullptr);

            for (AmUInt64 i = 0; i < frameCount; ++i)
            {
                leftEnergy += (*output)[0][i] * (*output)[0][i];
                rightEnergy += (*output)[1][i] * (*output)[1][i];
            }
        }
    };

    AM_REGISTER_TEST(mixer_nodes, speaker_panning_node_pans_right_for_any_heading);
} // namespace SparkyStudios::Audio::Amplitude::Tests
