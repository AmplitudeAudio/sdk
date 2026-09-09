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

#include <Ambisonics/AmbisonicOrientationProcessor.h>
#include <Ambisonics/BFormat.h>
#include <Math/LinearAlgebra.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, ambisonics_orientation_processor, order2_beta_legendre_weight)
    {
    public:
        void Run() override
        {
            constexpr AmUInt32 kSamples = 8;

            BFormat field;
            AM_EXPECT(field.Configure(2, true, kSamples));
            field.Reset();

            // Energy in the zonal 2nd-order harmonic only.
            for (AmUInt32 i = 0; i < kSamples; ++i)
                field.GetBufferChannel(eBFormatChannel_R)[i] = 1.0f;

            AmbisonicOrientationProcessor processor;
            AM_EXPECT(processor.Configure(2, true));

            // 90-degree rotation about +Y -> ZYZ (alpha=0, beta=pi/2, gamma=0).
            processor.SetOrientation(Orientation(FromAxisAngle(kVector3UnitY, AM_PI32 / 2.0f)));
            processor.Refresh(); // idempotent; harmless if SetOrientation already refreshed
            processor.Process(&field, kSamples);

            // Zonal harmonics rotate with the Legendre weight: R' = R * P2(cos beta)
            //   = R * (0.75*cos(2*beta) + 0.25) = -0.5 for beta = pi/2.
            // (The buggy 0.75*cos(beta) + 0.25 would give +0.25.)
            for (AmUInt32 i = 0; i < kSamples; ++i)
                AM_EXPECT(std::abs(field.GetBufferChannel(eBFormatChannel_R)[i] - (-0.5f)) < 1e-4f);
        }
    };

    AM_REGISTER_TEST(ambisonics_orientation_processor, order2_beta_legendre_weight);
} // namespace SparkyStudios::Audio::Amplitude::Tests
