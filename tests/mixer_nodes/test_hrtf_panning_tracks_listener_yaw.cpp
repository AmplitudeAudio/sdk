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
    AM_TEST_CASE(LightEngineTestCase, mixer_nodes, hrtf_panning_tracks_listener_yaw)
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

            // Listener at the origin, yawed +90 degrees about Z: forward (-1,0,0), up (0,0,1).
            // A world-fixed source at (0,10,0) is then directly at the listener's RIGHT.
            ListenerInternalState listenerState;
            listenerState.SetLocation({ 0.0f, 0.0f, 0.0f });
            listenerState.SetOrientation(Orientation({ -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }));
            listenerState.Update();
            Listener listener = InitTestListener(&listenerState);

            SpatialMockLayer mockLayer;
            mockLayer.SetSampleRate(sampleRate);
            mockLayer.SetSpatialization(eSpatialization_HRTF);
            mockLayer.SetListener(listener);
            mockLayer.SetLocation({ 0.0f, 10.0f, 0.0f }); // source position (world)

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

            auto* panningProcessor = dynamic_cast<ProcessorNodeInstance*>(panning.get());
            auto* rotatorProcessor = dynamic_cast<ProcessorNodeInstance*>(rotator.get());
            auto* decoderProcessor = dynamic_cast<ProcessorNodeInstance*>(decoder.get());

            const AudioBuffer* field = panningProcessor->Process(&input);
            AM_EXPECT_NOT(field == nullptr);

            const AudioBuffer* rotated = rotatorProcessor->Process(field);
            AM_EXPECT_NOT(rotated == nullptr);

            // Field-level assertion: the source sits at the listener's right, i.e. ears-centered +Y.
            // First-order channels exist for any order >= 1: W=0, Y=1, Z=2, X=3.
            AmReal32 rmsY = 0.0f, rmsX = 0.0f, dotY = 0.0f;
            for (AmUInt64 i = 0; i < frameCount; ++i)
            {
                rmsY += (*rotated)[eBFormatChannel_Y][i] * (*rotated)[eBFormatChannel_Y][i];
                rmsX += (*rotated)[eBFormatChannel_X][i] * (*rotated)[eBFormatChannel_X][i];
                dotY += (*rotated)[eBFormatChannel_Y][i] * input[0][i];
            }
            rmsY = std::sqrt(rmsY / static_cast<AmReal32>(frameCount));
            rmsX = std::sqrt(rmsX / static_cast<AmReal32>(frameCount));

            AM_EXPECT(rmsY > 5.0f * rmsX);
            AM_EXPECT(dotY > 0.0f); // ears-centered field: the listener's right encodes as +Y (ForHRTF +90 deg azimuth)

            // End-to-end assertion through the binaural decoder: right ear dominates.
            const AudioBuffer* ears = decoderProcessor->Process(rotated);
            AM_EXPECT_NOT(ears == nullptr);
            AM_EXPECT(ears->GetChannelCount() == kAmStereoChannelCount);

            AmReal32 leftRMS = 0.0f, rightRMS = 0.0f;
            for (AmUInt64 i = 0; i < frameCount; ++i)
            {
                leftRMS += (*ears)[0][i] * (*ears)[0][i];
                rightRMS += (*ears)[1][i] * (*ears)[1][i];
            }
            leftRMS = std::sqrt(leftRMS / static_cast<AmReal32>(frameCount));
            rightRMS = std::sqrt(rightRMS / static_cast<AmReal32>(frameCount));

            AM_EXPECT(rightRMS > 2.0f * leftRMS);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, hrtf_panning_tracks_listener_yaw);
} // namespace SparkyStudios::Audio::Amplitude::Tests
