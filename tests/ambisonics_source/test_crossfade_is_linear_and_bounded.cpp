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

#include <Ambisonics/AmbisonicSource.h>
#include <Ambisonics/BFormat.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, ambisonics_source, crossfade_is_linear_and_bounded)
    {
    public:
        void Run() override
        {
            constexpr AmUInt32 kSamples = 64;
            constexpr AmUInt32 kOrder = 1;
            constexpr AmUInt16 kChannels = 4; // 3D first order: W, Y, Z, X

            AudioBuffer input(kSamples, 1);
            for (AmUInt32 i = 0; i < kSamples; ++i)
                input[0][i] = 1.0f;

            AmbisonicSource source;
            AM_EXPECT(source.Configure(kOrder, true));

            // Capture the "old" coefficients (no interpolation).
            source.SetPosition(SphericalPosition::FromDegrees(0.0f, 0.0f, 1.0f), 0.0f);
            BFormat fieldA;
            fieldA.Configure(kOrder, true, kSamples);
            source.Process(input[0], kSamples, &fieldA);

            // Move with a half-block crossfade (32 samples).
            source.SetPosition(SphericalPosition::FromDegrees(90.0f, 0.0f, 1.0f), 0.5f);
            BFormat fieldB;
            fieldB.Configure(kOrder, true, kSamples);
            source.Process(input[0], kSamples, &fieldB);

            // Capture the "new" coefficients (interpolation was consumed by the previous Process).
            BFormat fieldC;
            fieldC.Configure(kOrder, true, kSamples);
            source.Process(input[0], kSamples, &fieldC);

            for (AmUInt16 c = 0; c < kChannels; ++c)
            {
                const AmReal32 oldCoeff = fieldA.GetBufferChannel(c)[0];
                const AmReal32 newCoeff = fieldC.GetBufferChannel(c)[0];
                const AmReal32 lo = AM_MIN(oldCoeff, newCoeff) - 1e-4f;
                const AmReal32 hi = AM_MAX(oldCoeff, newCoeff) + 1e-4f;

                // The crossfade starts exactly at the old coefficient.
                AM_EXPECT(std::abs(fieldB.GetBufferChannel(c)[0] - oldCoeff) < 1e-4f);

                // The crossfade is bounded by [old, new] and monotonic.
                AmReal32 prev = fieldB.GetBufferChannel(c)[0];
                for (AmUInt32 i = 1; i < 32; ++i)
                {
                    const AmReal32 v = fieldB.GetBufferChannel(c)[i];
                    AM_EXPECT(v >= lo && v <= hi);

                    if (newCoeff >= oldCoeff)
                        AM_EXPECT(v >= prev - 1e-4f);
                    else
                        AM_EXPECT(v <= prev + 1e-4f);

                    prev = v;
                }

                // The tail uses exactly the new coefficient.
                for (AmUInt32 i = 32; i < kSamples; ++i)
                    AM_EXPECT(std::abs(fieldB.GetBufferChannel(c)[i] - newCoeff) < 1e-4f);
            }

            // duration == 0 must snap without crashing.
            source.SetPosition(SphericalPosition::FromDegrees(45.0f, 0.0f, 1.0f), 0.0f);
            BFormat fieldD;
            fieldD.Configure(kOrder, true, kSamples);
            source.Process(input[0], kSamples, &fieldD);

            BFormat fieldD2;
            fieldD2.Configure(kOrder, true, kSamples);
            source.Process(input[0], kSamples, &fieldD2);
            for (AmUInt16 c = 0; c < kChannels; ++c)
                AM_EXPECT(std::abs(fieldD.GetBufferChannel(c)[0] - fieldD2.GetBufferChannel(c)[0]) < 1e-4f);

            // duration > 1 must clamp to the block and stay bounded.
            source.SetPosition(SphericalPosition::FromDegrees(0.0f, 0.0f, 1.0f), 2.0f);
            BFormat fieldE;
            fieldE.Configure(kOrder, true, kSamples);
            source.Process(input[0], kSamples, &fieldE);

            source.SetPosition(SphericalPosition::FromDegrees(0.0f, 0.0f, 1.0f), 0.0f);
            BFormat fieldE2;
            fieldE2.Configure(kOrder, true, kSamples);
            source.Process(input[0], kSamples, &fieldE2);
            for (AmUInt16 c = 0; c < kChannels; ++c)
            {
                const AmReal32 target = fieldE2.GetBufferChannel(c)[0];
                for (AmUInt32 i = 0; i < kSamples; ++i)
                {
                    const AmReal32 v = fieldE.GetBufferChannel(c)[i];
                    AM_EXPECT(v >= AM_MIN(fieldD2.GetBufferChannel(c)[0], target) - 1e-4f);
                    AM_EXPECT(v <= AM_MAX(fieldD2.GetBufferChannel(c)[0], target) + 1e-4f);
                }
            }
        }
    };

    AM_REGISTER_TEST(ambisonics_source, crossfade_is_linear_and_bounded);
} // namespace SparkyStudios::Audio::Amplitude::Tests
