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

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, mixer_voice, transport_envelope_uses_fader_curve)
    {
    public:
        void Run() override
        {
            auto reference = Fader::Construct("SCurveSmooth");
            AM_EXPECT_NOT(reference == nullptr);
            reference->Set(1.0, 0.0);

            TransportEnvelope envelope;
            envelope.Initialize(Fader::Construct("SCurveSmooth"), 48000);
            envelope.SetGain(1.0f);
            const AmUInt64 length = envelope.FadeTo(0.0f, 100.0);
            AM_EXPECT_EQ(4800ULL, length);

            AudioBuffer gains(length, 1);
            envelope.Render(gains[0], 0, length);

            // Every frame, not just the knots, follows the curve: the knots are joined by cubic Hermite with the curve's
            // own slopes.
            for (AmUInt64 k = 0; k < length; ++k)
            {
                const auto expected = static_cast<AmReal32>(reference->GetFromPercentage(static_cast<AmReal64>(k + 1) / static_cast<AmReal64>(length)));
                AM_EXPECT(std::abs(gains[0][k] - expected) < 1e-5f);
            }

            // A short fade bends sharply within one knot span; the Hermite slopes keep the error small there too.
            TransportEnvelope shortEnvelope;
            shortEnvelope.Initialize(Fader::Construct("SCurveSmooth"), 48000);
            shortEnvelope.SetGain(1.0f);
            const AmUInt64 shortLength = shortEnvelope.FadeTo(0.0f, 10.0);
            AudioBuffer shortGains(shortLength, 1);
            shortEnvelope.Render(shortGains[0], 0, shortLength);

            for (AmUInt64 k = 0; k < shortLength; ++k)
            {
                const auto expected =
                    static_cast<AmReal32>(reference->GetFromPercentage(static_cast<AmReal64>(k + 1) / static_cast<AmReal64>(shortLength)));
                AM_EXPECT(std::abs(shortGains[0][k] - expected) < 5e-4f); // measured 1.7e-4; a chord between knots misses by about 3e-3
            }
        }
    };

    AM_REGISTER_TEST(mixer_voice, transport_envelope_uses_fader_curve);
} // namespace SparkyStudios::Audio::Amplitude::Tests
