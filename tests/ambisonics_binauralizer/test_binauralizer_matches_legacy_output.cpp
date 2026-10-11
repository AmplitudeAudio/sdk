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

#include <Ambisonics/AmbisonicBinauralizer.h>
#include <Ambisonics/AmbisonicShelfFilter.h>
#include <Ambisonics/BFormat.h>

#include "DSPTestCase.h"
#include "PlatformTestCase.h"
#include "TestRegistry.h"

#include "BinauralReference.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, ambisonics_binauralizer, binauralizer_matches_legacy_output)
    {
    public:
        void Run() override
        {
            HRIRSphereImpl sphere;
            AM_EXPECT(LoadTestSphere(sphere) != nullptr);

            for (AmUInt32 order = 1; order <= 3; ++order)
                CheckOrder(order, sphere);
        }

    private:
        static constexpr AmUInt32 kFrames = 512;
        static constexpr AmUInt32 kSampleRate = 48000;
        static constexpr AmUInt32 kBlocks = 4;

        void CheckOrder(AmUInt32 order, const HRIRSphereImpl& sphere)
        {
            const AmUInt32 channels = OrderToComponents(order, true);
            const AmUInt32 hrirLength = sphere.GetIRLength();

            AmbisonicBinauralizer binauralizer;
            AM_EXPECT(binauralizer.Configure(order, true, kFrames, kSampleRate, &sphere));
            AM_EXPECT(binauralizer.GetFilters() != nullptr);

            // Legacy path: shelf filter, then one Convolver per channel and ear over the reference HRIRs.
            AmbisonicShelfFilter shelf;
            AM_EXPECT(shelf.Configure(order, true, kFrames, kSampleRate));

            AudioBuffer reference[2];
            BuildReferenceHRIRs(order, true, sphere, reference);

            std::vector<std::unique_ptr<Convolver>> convolvers[2];
            for (AmUInt32 ear = 0; ear < 2; ++ear)
                for (AmUInt32 c = 0; c < channels; ++c)
                {
                    convolvers[ear].push_back(std::make_unique<Convolver>());
                    convolvers[ear].back()->Init(hrirLength, reference[ear][c].begin(), hrirLength);
                }

            BFormat actualField;
            BFormat legacyField;
            actualField.Configure(order, true, kFrames);
            legacyField.Configure(order, true, kFrames);

            AudioBuffer signal(kFrames, 1);
            AudioBuffer actual(kFrames, 2);
            AudioBuffer legacy(kFrames, 2);
            AudioBuffer scratch(kFrames, 1);

            AmUInt32 seed = 99u + order;
            AmReal32 maxDiff = 0.0f;

            for (AmUInt32 block = 0; block < kBlocks; ++block)
            {
                for (AmUInt32 c = 0; c < channels; ++c)
                {
                    for (AmUInt32 i = 0; i < kFrames; ++i)
                    {
                        seed = seed * 1664525u + 1013904223u;
                        signal[0][i] = (static_cast<AmReal32>(seed >> 8) / static_cast<AmReal32>(1u << 24) * 2.0f - 1.0f) /
                            static_cast<AmReal32>(c + 1);
                    }

                    actualField.CopyStream(signal[0], c, kFrames);
                    legacyField.CopyStream(signal[0], c, kFrames);
                }

                actual.Clear();
                binauralizer.Process(&actualField, kFrames, actual);

                legacy.Clear();
                shelf.Process(&legacyField, kFrames);
                for (AmUInt32 c = 0; c < channels; ++c)
                {
                    for (AmUInt32 ear = 0; ear < 2; ++ear)
                    {
                        convolvers[ear][c]->Process(legacyField.GetBufferChannel(c).begin(), scratch[0].begin(), kFrames);
                        for (AmUInt32 i = 0; i < kFrames; ++i)
                            legacy[ear][i] += scratch[0][i];
                    }
                }

                for (AmUInt32 ear = 0; ear < 2; ++ear)
                    for (AmUInt32 i = 0; i < kFrames; ++i)
                        maxDiff = std::max(maxDiff, std::abs(actual[ear][i] - legacy[ear][i]));
            }

            AM_EXPECT(maxDiff <= 1e-5f);
        }
    };

    AM_REGISTER_TEST(ambisonics_binauralizer, binauralizer_matches_legacy_output);
} // namespace SparkyStudios::Audio::Amplitude::Tests
