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
    AM_TEST_CASE(ComponentTestCase, mixer_voice, transport_envelope_declick_floor)
    {
    public:
        void Run() override
        {
            TransportEnvelope envelope;
            envelope.Initialize(nullptr, 48000);
            envelope.SetGain(1.0f);

            // Stop(0) becomes a 5 ms raised cosine: 240 frames at 48 kHz.
            AM_EXPECT_EQ(240ULL, envelope.FadeTo(0.0f, 0.0));

            AudioBuffer gains(300, 1);
            envelope.Render(gains[0], 0, 300);

            AM_EXPECT(std::abs(gains[0][119] - 0.5f) < 0.02f);
            AM_EXPECT(gains[0][0] > 0.99f);
            AM_EXPECT_EQ(0.0f, gains[0][239]);

            // Every frame sits on the analytic raised cosine, not on a chord between knots: frame k has the value at
            // (k + 1) / 240.
            for (AmUInt64 k = 0; k < 240; ++k)
            {
                const AmReal64 p = static_cast<AmReal64>(k + 1) / 240.0;
                const auto expected = static_cast<AmReal32>(1.0 - (0.5 - 0.5 * std::cos(3.14159265358979323846 * p)));
                AM_EXPECT(std::abs(gains[0][k] - expected) < 1e-6f);
            }

            // Monotonic, so no step anywhere.
            for (AmUInt64 k = 1; k < 240; ++k)
                AM_EXPECT(gains[0][k] <= gains[0][k - 1]);
        }
    };

    AM_REGISTER_TEST(mixer_voice, transport_envelope_declick_floor);
} // namespace SparkyStudios::Audio::Amplitude::Tests
