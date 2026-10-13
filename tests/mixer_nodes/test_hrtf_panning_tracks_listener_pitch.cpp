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
#include <Ambisonics/AmbisonicSource.h>
#include <Core/ListenerInternalState.h>
#include <Mixer/Nodes/AmbisonicPanningNode.h>
#include <Mixer/Nodes/AmbisonicRotatorNode.h>
#include <Utils/Utils.h>

#include "LightEngineTestCase.h"
#include "NodeTestCase.h"
#include "TestRegistry.h"
#include "TestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(LightEngineTestCase, mixer_nodes, hrtf_panning_tracks_listener_pitch)
    {
    public:
        void SetUp() override
        {
            LightEngineTestCase::SetUp();
            InitializeConfig(AM_OS_STRING("tests.config.amconfig"));
        }

        void Run() override
        {
            const AmReal32 c45 = std::sqrt(0.5f);

            // Pitched up 45 degrees: the listener's forward, the world's up and the listener's right.
            CheckField({ 0.0f, c45, c45 }, { 0.0f, -c45, c45 }, { 0.0f, 10.0f * c45, 10.0f * c45 });
            CheckField({ 0.0f, c45, c45 }, { 0.0f, -c45, c45 }, { 0.0f, 0.0f, 10.0f });
            CheckField({ 0.0f, c45, c45 }, { 0.0f, -c45, c45 }, { 10.0f, 0.0f, 0.0f });

            // A Y-up listener (forward -Z, up +Y): ahead, right, overhead and an oblique direction.
            CheckField({ 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, -10.0f });
            CheckField({ 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 10.0f, 0.0f, 0.0f });
            CheckField({ 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 10.0f, 0.0f });
            CheckField({ 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { -7.0f, 5.0f, 3.0f });

            // Yaw only: the pitch step runs at zero angle and must leave every channel in place.
            CheckField({ -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 3.0f, 5.0f, 2.0f });

            // Yaw, pitch and roll together.
            const Orientation turned(0.7f, -0.4f, 0.3f);
            CheckField(turned.GetForward(), turned.GetUp(), { 3.0f, -6.0f, 4.0f });
            CheckField(turned.GetForward(), turned.GetUp(), { -2.0f, 1.0f, -8.0f });
        }

    private:
        // Encodes a world-fixed source through the panning and rotator nodes, and compares every ambisonic channel with
        // the source encoded directly at its direction relative to the listener.
        void CheckField(const AmVector3& forward, const AmVector3& up, const AmVector3& source)
        {
            constexpr AmUInt64 frameCount = 512;
            constexpr AmUInt32 sampleRate = 48000;

            ListenerInternalState listenerState;
            listenerState.SetLocation({ 0.0f, 0.0f, 0.0f });
            listenerState.SetOrientation(Orientation(forward, up));
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

            AudioBuffer input(frameCount, 1);
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = 1.0f;

            const AudioBuffer* field = dynamic_cast<ProcessorNodeInstance*>(panning.get())->Process(&input);
            AM_EXPECT_NOT(field == nullptr);

            const AudioBuffer* rotated = dynamic_cast<ProcessorNodeInstance*>(rotator.get())->Process(field);
            AM_EXPECT_NOT(rotated == nullptr);

            const AmVector3 unitForward = Normalize(forward);
            const AmVector3 right = Normalize(Cross(unitForward, up));
            const AmVector3 unitUp = Cross(right, unitForward);
            const AmVector3 relative{ Dot(source, right), Dot(source, unitForward), Dot(source, unitUp) };

            AmbisonicSource expected;
            expected.Configure(order, true);
            expected.SetPosition(SphericalPosition::ForHRTF(relative));

            // The panning gain is not under test: compare each channel relative to W, on the last frame.
            const AmUInt64 last = frameCount - 1;
            const AmReal32 w = (*rotated)[0][last];
            AM_EXPECT(w > 0.0f);

            for (AmUInt16 c = 1; c < ambiChannels; ++c)
            {
                const AmReal32 actual = (*rotated)[c][last] / w;
                const AmReal32 target = expected.GetCoefficient(c) / expected.GetCoefficient(0);
                AM_EXPECT(std::abs(actual - target) < 1e-3f);
            }
        }
    };

    AM_REGISTER_TEST(mixer_nodes, hrtf_panning_tracks_listener_pitch);
} // namespace SparkyStudios::Audio::Amplitude::Tests
