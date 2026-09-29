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
    AM_TEST_CASE(ComponentTestCase, mixer_voice, transport_envelope_linear_fade)
    {
    public:
        void Run() override
        {
            TransportEnvelope envelope;
            envelope.Initialize(nullptr, 48000);
            envelope.SetGain(1.0f);
            AM_EXPECT_EQ(480ULL, envelope.FadeTo(0.0f, 10.0));

            AudioBuffer gains(1000, 1);
            envelope.Render(gains[0], 0, 1000);

            for (AmUInt64 k = 0; k < 480; ++k)
                AM_EXPECT(std::abs(gains[0][k] - (1.0f - static_cast<AmReal32>(k + 1) / 480.0f)) < 1e-5f);
            for (AmUInt64 k = 479; k < 1000; ++k)
                AM_EXPECT_EQ(0.0f, gains[0][k]);

            AM_EXPECT_NOT(envelope.IsFading());
            AM_EXPECT_EQ(0.0f, envelope.GetGain());
        }
    };

    AM_REGISTER_TEST(mixer_voice, transport_envelope_linear_fade);
} // namespace SparkyStudios::Audio::Amplitude::Tests
