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

#include <Ambisonics/BinauralDecodingFilters.h>
#include <DSP/ConvolverKernel.h>

#include "DSPTestCase.h"
#include "PlatformTestCase.h"
#include "TestRegistry.h"

#include "BinauralReference.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, ambisonics_binauralizer, binaural_filters_match_legacy_accumulation)
    {
    public:
        void Run() override
        {
            HRIRSphereImpl unloaded;
            AM_EXPECT(BinauralDecodingFilters::Build(1, true, unloaded) == nullptr);

            HRIRSphereImpl sphere;
            AM_EXPECT(LoadTestSphere(sphere) != nullptr);

            for (AmUInt32 order = 1; order <= 3; ++order)
                CheckOrder(order, sphere);
        }

    private:
        // The FFT backend is not bit-reproducible across buffers, so spectra are compared within float tolerance.
        static bool Near(AmReal32 a, AmReal32 b)
        {
            return std::abs(a - b) <= 1e-6f * std::max(1.0f, std::abs(b));
        }

        void CheckOrder(AmUInt32 order, const HRIRSphereImpl& sphere)
        {
            const auto filters = BinauralDecodingFilters::Build(order, true, sphere);
            AM_EXPECT(filters != nullptr);
            AM_EXPECT(filters->GetOrder() == order);
            AM_EXPECT(filters->Is3D());
            AM_EXPECT(filters->GetChannelCount() == OrderToComponents(order, true));
            AM_EXPECT(filters->GetKernels(0).size() == filters->GetChannelCount());
            AM_EXPECT(filters->GetKernels(1).size() == filters->GetChannelCount());

            AudioBuffer reference[2];
            BuildReferenceHRIRs(order, true, sphere, reference);

            const AmUInt32 hrirLength = sphere.GetIRLength();

            AM_EXPECT(filters->GetHRIRLength() == hrirLength);

            bool timeSame = true;
            bool same = true;
            for (AmUInt32 ear = 0; ear < 2; ++ear)
            {
                for (AmUInt32 c = 0; c < filters->GetChannelCount(); ++c)
                {
                    const auto expected = ConvolverKernel::Create(hrirLength, reference[ear][c].begin(), hrirLength);
                    const auto& actual = filters->GetKernel(ear, c);

                    // The time-domain HRIRs are the real bit-identity check.
                    for (AmUInt32 i = 0; i < hrirLength; ++i)
                        timeSame = timeSame && filters->GetHRIR(ear, c)[i] == reference[ear][c][i];

                    same = same && actual->GetSegmentCount() == expected->GetSegmentCount();
                    for (AmSize s = 0; same && s < expected->GetSegmentCount(); ++s)
                        for (AmSize i = 0; i < expected->GetComplexSize(); ++i)
                            same = same && Near(actual->GetSegment(s).re()[i], expected->GetSegment(s).re()[i]) &&
                                Near(actual->GetSegment(s).im()[i], expected->GetSegment(s).im()[i]);
                }
            }

            AM_EXPECT(timeSame);
            AM_EXPECT(same);
        }
    };

    AM_REGISTER_TEST(ambisonics_binauralizer, binaural_filters_match_legacy_accumulation);
} // namespace SparkyStudios::Audio::Amplitude::Tests
