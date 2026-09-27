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

#include <algorithm>
#include <cmath>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

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
        constexpr AmReal32 kAmplitude = 0.5f;

        // Air absorption with fixed band gains: low and mid untouched, high band at 0.25 (-12 dB).
        class FixedAirAbsorptionAttenuation final : public MockAttenuation
        {
        public:
            FixedAirAbsorptionAttenuation()
                : MockAttenuation(100.0)
            {}

            [[nodiscard]] bool IsAirAbsorptionEnabled() const override
            {
                return true;
            }

            [[nodiscard]] AmReal32 EvaluateAirAbsorption(const AmVector3&, const AmVector3&, AmUInt32 band) const override
            {
                return band == 2 ? 0.25f : 1.0f;
            }
        };
    } // namespace

    AM_TEST_CASE(NodeTestCase, mixer_nodes, attenuation_node_air_absorption_uses_db)
    {
    public:
        void Run() override
        {
            GetMockLayer().SetSampleRate(kSampleRate);
            GetMockLayer().SetSpatialization(eSpatialization_Position);
            GetMockLayer().SetLocation({ 0.0f, 0.0f, -1.0f });

            ListenerInternalState listenerState;
            listenerState.SetLocation({ 0.0f, 0.0f, 0.0f });
            listenerState.SetOrientation(Orientation({ 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }));
            listenerState.Update();
            GetMockLayer().SetListener(InitTestListener(&listenerState));

            FixedAirAbsorptionAttenuation attenuation;
            GetMockLayer().SetAttenuation(&attenuation);

            // The distance gain at 1 m is 0.99; air absorption must only shape the spectrum.
            const AmReal32 distanceGain = 0.99f;

            // A 12 kHz tone sits in the high shelf: -12 dB there must take it at least 6 dB down.
            const AmReal32 highPeak = SteadyStatePeak(12000.0f);
            AM_EXPECT(highPeak < 0.5f * distanceGain * kAmplitude);

            // A 200 Hz tone sits in the untouched low band.
            const AmReal32 lowPeak = SteadyStatePeak(200.0f);
            AM_EXPECT(lowPeak > 0.9f * distanceGain * kAmplitude);
        }

    private:
        AmReal32 SteadyStatePeak(AmReal32 frequency)
        {
            auto instance = CreateAndConfigureNode<AttenuationNode>(kFrames, 1);
            auto* processor = AsProcessor(instance);

            AudioBuffer input(kFrames, 1);
            AmReal32 peak = 0.0f;

            // Blocks 0-1 let the gain ramp and the EQ crossfade settle; measure the last one.
            for (AmUInt64 block = 0; block < 4; ++block)
            {
                for (AmUInt64 i = 0; i < kFrames; ++i)
                {
                    const AmReal32 t = static_cast<AmReal32>(block * kFrames + i) / static_cast<AmReal32>(kSampleRate);
                    input[0][i] = kAmplitude * std::sin(2.0f * AM_PI32 * frequency * t);
                }

                instance->Reset();
                const AudioBuffer* output = processor->Process(&input);
                AM_EXPECT_NOT(output == nullptr);

                if (block == 3)
                    for (AmUInt64 i = 0; i < kFrames; ++i)
                        peak = std::max(peak, std::abs((*output)[0][i]));
            }

            return peak;
        }
    };

    AM_REGISTER_TEST(mixer_nodes, attenuation_node_air_absorption_uses_db);
} // namespace SparkyStudios::Audio::Amplitude::Tests
