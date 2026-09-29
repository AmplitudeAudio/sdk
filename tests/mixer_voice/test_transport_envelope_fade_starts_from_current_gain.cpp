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

#include <Mixer/Voice/TransportEnvelope.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, mixer_voice, transport_envelope_fade_starts_from_current_gain)
    {
    public:
        void Run() override
        {
            TransportEnvelope envelope;
            envelope.Initialize(nullptr, 48000);
            envelope.SetGain(1.0f);
            envelope.FadeTo(0.0f, 10.0);

            AudioBuffer gains(480, 1);
            envelope.Render(gains[0], 0, 240);
            const AmReal32 reached = gains[0][239];
            AM_EXPECT(std::abs(reached - 0.5f) < 0.01f);

            envelope.FadeTo(1.0f, 10.0);
            envelope.Render(gains[0], 240, 240);

            // No jump at the turn-around.
            AM_EXPECT(std::abs(gains[0][240] - reached) < 0.01f);
            AM_EXPECT(gains[0][479] > reached);
        }
    };

    AM_REGISTER_TEST(mixer_voice, transport_envelope_fade_starts_from_current_gain);
} // namespace SparkyStudios::Audio::Amplitude::Tests
