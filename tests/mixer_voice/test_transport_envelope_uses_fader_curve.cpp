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

            // Knots every 32 frames are exact; frames between them are within the curve's local slope.
            for (AmUInt64 k = 31; k < length; k += 32)
            {
                const auto expected = static_cast<AmReal32>(reference->GetFromPercentage(static_cast<AmReal64>(k + 1) / static_cast<AmReal64>(length)));
                AM_EXPECT(std::abs(gains[0][k] - expected) < 1e-5f);
            }
        }
    };

    AM_REGISTER_TEST(mixer_voice, transport_envelope_uses_fader_curve);
} // namespace SparkyStudios::Audio::Amplitude::Tests
