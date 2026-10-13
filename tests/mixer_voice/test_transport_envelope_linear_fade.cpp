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

            // The two ends are rounded over 0.5 ms (24 frames) each, so the ramp has no slope corner; everything between
            // stays the straight ramp.
            constexpr AmReal64 length = 480.0;
            constexpr AmReal64 corner = 24.0;
            const auto ease = [](AmReal64 d)
            {
                const AmReal64 u = d / corner;
                return corner * u * u * (2.0 - u);
            };

            for (AmUInt64 k = 0; k < 480; ++k)
            {
                const auto x = static_cast<AmReal64>(k + 1);
                AmReal64 warped = x;
                if (x < corner)
                    warped = ease(x);
                else if (length - x < corner)
                    warped = length - ease(length - x);

                AM_EXPECT(std::abs(gains[0][k] - static_cast<AmReal32>(1.0 - warped / length)) < 1e-5f);
            }

            // No corner: the first and last steps are tiny (a plain ramp steps 1 / 480 = 2.1e-3 from the first frame), and
            // the step never changes abruptly.
            AM_EXPECT(1.0f - gains[0][0] < 5e-4f);
            for (AmUInt64 k = 2; k < 480; ++k)
                AM_EXPECT(std::abs((gains[0][k] - gains[0][k - 1]) - (gains[0][k - 1] - gains[0][k - 2])) < 1e-3f);
            for (AmUInt64 k = 479; k < 1000; ++k)
                AM_EXPECT_EQ(0.0f, gains[0][k]);

            AM_EXPECT_NOT(envelope.IsFading());
            AM_EXPECT_EQ(0.0f, envelope.GetGain());
        }
    };

    AM_REGISTER_TEST(mixer_voice, transport_envelope_linear_fade);
} // namespace SparkyStudios::Audio::Amplitude::Tests
