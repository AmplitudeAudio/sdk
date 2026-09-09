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

#include <Ambisonics/AmbisonicComponent.h>
#include <Core/ListenerInternalState.h>
#include <Mixer/Nodes/AmbisonicBinauralDecoderNode.h>
#include <Mixer/Nodes/AmbisonicPanningNode.h>
#include <Mixer/Nodes/AmbisonicRotatorNode.h>

#include "LightEngineTestCase.h"
#include "NodeTestCase.h"
#include "TestRegistry.h"
#include "TestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(LightEngineTestCase, mixer_nodes, hrtf_panning_keeps_source_ahead_for_identity_listener)
    {
    public:
        void SetUp() override
        {
            LightEngineTestCase::SetUp();
            InitializeConfig(AM_OS_STRING("tests.config.amconfig"));
        }

        void Run() override
        {
            constexpr AmUInt64 frameCount = 512;
            constexpr AmUInt32 sampleRate = 48000;

            // Identity listener: forward +Y, up +Z. Source straight ahead at (0,10,0).
            ListenerInternalState listenerState;
            listenerState.SetLocation({ 0.0f, 0.0f, 0.0f });
            listenerState.SetOrientation(Orientation({ 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }));
            listenerState.Update();
            Listener listener = InitTestListener(&listenerState);

            SpatialMockLayer mockLayer;
            mockLayer.SetSampleRate(sampleRate);
            mockLayer.SetSpatialization(eSpatialization_HRTF);
            mockLayer.SetListener(listener);
            mockLayer.SetLocation({ 0.0f, 10.0f, 0.0f });

            SoundFormat format;
            format.SetAll(sampleRate, 1, 32, frameCount, 4, eAudioSampleFormat_Float32);
            mockLayer.SetSoundFormat(format);

            const ePanningMode mode = amEngine->GetPanningMode();
            const AmUInt32 order = AM_MAX(static_cast<AmUInt32>(mode), 1u);
            const AmUInt16 ambiChannels = static_cast<AmUInt16>(OrderToComponents(order, true));

            AmbisonicPanningNode panningFactory;
            auto panning = panningFactory.CreateInstance();
            panning->Initialize(1, &mockLayer, nullptr, panningFactory.GetParameterCount());
            panning->Configure(frameCount, 1);

            AmbisonicRotatorNode rotatorFactory;
            auto rotator = rotatorFactory.CreateInstance();
            rotator->Initialize(2, &mockLayer, nullptr, rotatorFactory.GetParameterCount());
            rotator->Configure(frameCount, ambiChannels);

            AmbisonicBinauralDecoderNode decoderFactory;
            auto decoder = decoderFactory.CreateInstance();
            decoder->Initialize(3, &mockLayer, nullptr, decoderFactory.GetParameterCount());
            decoder->Configure(frameCount, ambiChannels);

            AudioBuffer input(frameCount, 1);
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = std::sin(2.0f * AM_PI32 * 440.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate));

            const AudioBuffer* field = dynamic_cast<ProcessorNodeInstance*>(panning.get())->Process(&input);
            const AudioBuffer* rotated = dynamic_cast<ProcessorNodeInstance*>(rotator.get())->Process(field);
            AM_EXPECT_NOT(rotated == nullptr);

            // Source ahead -> AmbiX +X (channel 3) dominates.
            AmReal32 rmsY = 0.0f, rmsX = 0.0f;
            for (AmUInt64 i = 0; i < frameCount; ++i)
            {
                rmsY += (*rotated)[eBFormatChannel_Y][i] * (*rotated)[eBFormatChannel_Y][i];
                rmsX += (*rotated)[eBFormatChannel_X][i] * (*rotated)[eBFormatChannel_X][i];
            }
            rmsY = std::sqrt(rmsY / static_cast<AmReal32>(frameCount));
            rmsX = std::sqrt(rmsX / static_cast<AmReal32>(frameCount));
            AM_EXPECT(rmsX > 5.0f * rmsY);

            // Ears are balanced for a centered frontal source.
            const AudioBuffer* ears = dynamic_cast<ProcessorNodeInstance*>(decoder.get())->Process(rotated);
            AM_EXPECT_NOT(ears == nullptr);

            AmReal32 leftRMS = 0.0f, rightRMS = 0.0f;
            for (AmUInt64 i = 0; i < frameCount; ++i)
            {
                leftRMS += (*ears)[0][i] * (*ears)[0][i];
                rightRMS += (*ears)[1][i] * (*ears)[1][i];
            }
            leftRMS = std::sqrt(leftRMS / static_cast<AmReal32>(frameCount));
            rightRMS = std::sqrt(rightRMS / static_cast<AmReal32>(frameCount));

            AM_EXPECT(leftRMS > 0.001f);
            AM_EXPECT(std::abs(leftRMS - rightRMS) / AM_MAX(leftRMS, rightRMS) < 0.3f);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, hrtf_panning_keeps_source_ahead_for_identity_listener);
} // namespace SparkyStudios::Audio::Amplitude::Tests
