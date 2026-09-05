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

#include <Mixer/Nodes/AttenuationNode.h>

#include "NodeTestCase.h"
#include "TestRegistry.h"
#include "TestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        // Mock attenuation with air absorption enabled: absorption increases with distance,
        // and higher frequency bands decay faster (mirroring real air absorption). The band
        // index maps to the EQ bands in AttenuationNode: 0 = low shelf, 1 = mid peaking,
        // 2 = high shelf. Band-dependent values are required so that Normalize() yields
        // differing gains per block: this is what drives _needUpdateGains between the two
        // Process() calls and exercises the crossfade path under test.
        class MockAirAbsorptionAttenuation final : public MockAttenuation
        {
        public:
            explicit MockAirAbsorptionAttenuation(AmReal64 maxDistance = 100.0)
                : MockAttenuation(maxDistance)
            {}

            [[nodiscard]] bool IsAirAbsorptionEnabled() const override
            {
                return true;
            }

            [[nodiscard]] AmReal32 EvaluateAirAbsorption(const AmVector3& soundLocation, const AmVector3& listenerLocation, AmUInt32 band) const override
            {
                const AmReal32 distance = Length(Sub(soundLocation, listenerLocation));

                AmReal32 bandDistance;
                switch (band)
                {
                case 0: // low
                    bandDistance = 120.0f;
                    break;
                case 1: // mid
                    bandDistance = 100.0f;
                    break;
                default: // high
                    bandDistance = 70.0f;
                    break;
                }

                return std::max(0.2f, 1.0f - distance / bandDistance);
            }
        };
    } // namespace

    AM_TEST_CASE(NodeTestCase, mixer_nodes, attenuation_node_air_absorption_updates_between_blocks)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 1024;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            GetMockLayer().SetSampleRate(sampleRate);
            GetMockLayer().SetSpatialization(eSpatialization_Position);

            ListenerInternalState listenerState;
            listenerState.SetLocation({ 0.0f, 0.0f, 0.0f });
            listenerState.SetOrientation(Orientation({ 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }));
            listenerState.Update();
            Listener listener = InitTestListener(&listenerState);
            GetMockLayer().SetListener(listener);

            MockAirAbsorptionAttenuation mockAttenuation(100.0);
            GetMockLayer().SetAttenuation(&mockAttenuation);

            AudioBuffer input(frameCount, channelCount);
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = std::sin(2.0f * AM_PI32 * 440.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate));

            // Same node instance across both blocks: the gain change between them exercises
            // the air-absorption crossfade path with the hoisted buffers.
            auto instance = CreateAndConfigureNode<AttenuationNode>(frameCount, channelCount);
            auto* processor = AsProcessor(instance);
            AM_EXPECT_NOT(processor == nullptr);

            GetMockLayer().SetLocation({ 0.0f, 0.0f, -5.0f });
            const AudioBuffer* outNear = processor->Process(&input);
            AM_EXPECT_NOT(outNear == nullptr);

            // The node returns a pointer to its internal output buffer, which the next
            // Process call overwrites. Measure the near output before processing the far one.
            AmReal32 nearPeak = 0.0f;
            for (AmUInt64 i = 0; i < frameCount; ++i)
                nearPeak = std::max(nearPeak, std::abs((*outNear)[0][i]));

            GetMockLayer().SetLocation({ 0.0f, 0.0f, -50.0f });
            const AudioBuffer* outFar = processor->Process(&input);
            AM_EXPECT_NOT(outFar == nullptr);

            AmReal32 farPeak = 0.0f;
            for (AmUInt64 i = 0; i < frameCount; ++i)
                farPeak = std::max(farPeak, std::abs((*outFar)[0][i]));

            // Both outputs must be finite and non-silent. The EQ gains are in dB and
            // normalized to a small boost (max band = 1.0 dB), so the near output may
            // legitimately exceed the input peak; only the far/near ordering is asserted.
            AM_EXPECT(nearPeak > 0.0f);
            AM_EXPECT(farPeak > 0.0f && farPeak < nearPeak);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, attenuation_node_air_absorption_updates_between_blocks);
} // namespace SparkyStudios::Audio::Amplitude::Tests
