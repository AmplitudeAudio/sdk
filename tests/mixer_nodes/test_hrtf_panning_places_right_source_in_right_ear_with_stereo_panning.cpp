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
    AM_TEST_CASE(LightEngineTestCase, mixer_nodes, hrtf_panning_places_right_source_in_right_ear_with_stereo_panning)
    {
    public:
        void SetUp() override
        {
            LightEngineTestCase::SetUp();
            InitializeConfig(AM_OS_STRING("tests.stereo.config.amconfig"));
        }

        void Run() override
        {
            struct Case
            {
                AmVector3 forward;
                AmVector3 source;
            };

            // Stereo panning decodes the field on a virtual speaker pair instead of binauralizing it.
            AM_EXPECT_EQ(ePanningMode_Stereo, amEngine->GetPanningMode());

            // Listener at the origin, up (0,0,1). The source is placed on the listener's right: forward x up.
            const Case cases[] = {
                { { 0.0f, 1.0f, 0.0f }, { 10.0f, 0.0f, 0.0f } },
                { { 1.0f, 0.0f, 0.0f }, { 0.0f, -10.0f, 0.0f } },
                { { 0.0f, -1.0f, 0.0f }, { -10.0f, 0.0f, 0.0f } },
                { { -1.0f, 0.0f, 0.0f }, { 0.0f, 10.0f, 0.0f } },
            };

            for (const Case& c : cases)
            {
                AmReal32 leftRMS = 0.0f, rightRMS = 0.0f;
                Render(c.forward, c.source, leftRMS, rightRMS);
                // A first-order stereo decode separates a side source by 2:1 at most.
                AM_EXPECT(rightRMS > 1.5f * leftRMS);
            }
        }

    private:
        void Render(const AmVector3& forward, const AmVector3& source, AmReal32& leftRMS, AmReal32& rightRMS)
        {
            constexpr AmUInt64 frameCount = 512;
            constexpr AmUInt32 sampleRate = 48000;

            ListenerInternalState listenerState;
            listenerState.SetLocation({ 0.0f, 0.0f, 0.0f });
            listenerState.SetOrientation(Orientation(forward, { 0.0f, 0.0f, 1.0f }));
            listenerState.Update();
            Listener listener = InitTestListener(&listenerState);

            SpatialMockLayer mockLayer;
            mockLayer.SetSampleRate(sampleRate);
            mockLayer.SetSpatialization(eSpatialization_HRTF);
            mockLayer.SetListener(listener);
            mockLayer.SetLocation(source);

            SoundFormat format;
            format.SetAll(sampleRate, 1, 32, frameCount, 4, eAudioSampleFormat_Float32);
            mockLayer.SetSoundFormat(format);

            const AmUInt32 order = AM_MAX(static_cast<AmUInt32>(amEngine->GetPanningMode()), 1u);
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
            AM_EXPECT_NOT(field == nullptr);

            const AudioBuffer* rotated = dynamic_cast<ProcessorNodeInstance*>(rotator.get())->Process(field);
            AM_EXPECT_NOT(rotated == nullptr);

            const AudioBuffer* ears = dynamic_cast<ProcessorNodeInstance*>(decoder.get())->Process(rotated);
            AM_EXPECT_NOT(ears == nullptr);

            for (AmUInt64 i = 0; i < frameCount; ++i)
            {
                leftRMS += (*ears)[0][i] * (*ears)[0][i];
                rightRMS += (*ears)[1][i] * (*ears)[1][i];
            }

            leftRMS = std::sqrt(leftRMS / static_cast<AmReal32>(frameCount));
            rightRMS = std::sqrt(rightRMS / static_cast<AmReal32>(frameCount));
        }
    };

    AM_REGISTER_TEST(mixer_nodes, hrtf_panning_places_right_source_in_right_ear_with_stereo_panning);
} // namespace SparkyStudios::Audio::Amplitude::Tests
