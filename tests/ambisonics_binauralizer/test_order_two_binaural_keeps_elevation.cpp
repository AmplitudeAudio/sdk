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

#include <algorithm>
#include <cmath>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Ambisonics/AmbisonicBinauralizer.h>
#include <Ambisonics/AmbisonicDecoder.h>
#include <Ambisonics/AmbisonicSource.h>
#include <Ambisonics/BFormat.h>

#include "DSPTestCase.h"
#include "PlatformTestCase.h"
#include "TestRegistry.h"

#include "BinauralReference.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, ambisonics_binauralizer, order_two_binaural_keeps_elevation)
    {
    public:
        void Run() override
        {
            // The order-2 virtual layout is the 20-point dodecahedron.
            AmbisonicDecoder decoder;
            AM_EXPECT(decoder.Configure(2, true, eSpeakersPreset_DodecahedronFaces));
            AM_EXPECT(decoder.GetSpeakerCount() == 20);

            constexpr AmReal32 expected[20][2] = { { -69.1f, 90.0f },  { -69.1f, -90.0f }, { -35.3f, 45.0f },  { -35.3f, 135.0f },
                                                   { -35.3f, -45.0f }, { -35.3f, -135.0f }, { -20.9f, 180.0f }, { -20.9f, 0.0f },
                                                   { 0.0f, 69.1f },    { 0.0f, 110.9f },   { 0.0f, -69.1f },   { 0.0f, -110.9f },
                                                   { 20.9f, 180.0f },  { 20.9f, 0.0f },    { 35.3f, 45.0f },   { 35.3f, 135.0f },
                                                   { 35.3f, -45.0f },  { 35.3f, -135.0f }, { 69.1f, 90.0f },   { 69.1f, -90.0f } };
            for (AmUInt32 i = 0; i < 20; ++i)
            {
                const SphericalPosition p = decoder.GetSpeakerPosition(i);
                const AmReal32 elevation = p.GetElevation() * AM_RadToDeg;
                const AmReal32 azimuth = p.GetAzimuth() * AM_RadToDeg;
                const AmReal32 dAz = std::abs(std::fmod(azimuth - expected[i][1] + 540.0f, 360.0f) - 180.0f);
                AM_EXPECT(std::abs(elevation - expected[i][0]) <= 0.01f);
                AM_EXPECT(dAz <= 0.01f);
            }

            // A source above and its mirror below render differently (a flat ring renders them alike).
            HRIRSphereImpl sphere;
            AM_EXPECT(LoadTestSphere(sphere) != nullptr);

            const AudioBuffer up = Render(sphere, 60.0f);
            const AudioBuffer down = Render(sphere, -60.0f);
            AmReal32 maxDiff = 0.0f;
            for (AmUInt16 ear = 0; ear < 2; ++ear)
                for (AmUInt32 i = 0; i < kFrames; ++i)
                    maxDiff = std::max(maxDiff, std::abs(up[ear][i] - down[ear][i]));
            AM_EXPECT(maxDiff > 1e-3f);
        }

    private:
        static constexpr AmUInt32 kFrames = 1024;

        static AudioBuffer Render(const HRIRSphereImpl& sphere, AmReal32 elevationDegrees)
        {
            AmbisonicBinauralizer binauralizer;
            binauralizer.Configure(2, true, kFrames, 48000, &sphere);

            AudioBuffer tone(kFrames, 1);
            AmUInt32 seed = 3u;
            for (AmUInt32 i = 0; i < kFrames; ++i)
            {
                seed = seed * 1664525u + 1013904223u;
                tone[0][i] = static_cast<AmReal32>(seed >> 8) / static_cast<AmReal32>(1u << 24) - 0.5f;
            }

            AmbisonicSource source;
            source.Configure(2, true);
            source.SetPosition(SphericalPosition::FromDegrees(-30.0f, elevationDegrees, 1.0f));

            BFormat field;
            field.Configure(2, true, kFrames);
            field.Reset();
            source.Process(tone[0], kFrames, &field);

            AudioBuffer out(kFrames, 2);
            binauralizer.Process(&field, kFrames, out);
            return out;
        }
    };

    AM_REGISTER_TEST(ambisonics_binauralizer, order_two_binaural_keeps_elevation);
} // namespace SparkyStudios::Audio::Amplitude::Tests
