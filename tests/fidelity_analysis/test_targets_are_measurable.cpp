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

#include <Fidelity/Targets.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    // A target stricter than the finest value its analyzer resolves cannot be verified.
    AM_TEST_CASE(PureUnitTestCase, fidelity_analysis, targets_are_measurable)
    {
    public:
        void Run() override
        {
            AM_EXPECT(Targets::kMaxClickDbfs > Floors::kClickDbfs);
            AM_EXPECT(Targets::kMaxThdnDb > Floors::kSpectrumDb);
            AM_EXPECT(Targets::kMaxSpurDbc > Floors::kSpectrumDb);
            AM_EXPECT(Targets::kMaxPassbandRippleDb > Floors::kRippleDb);
            AM_EXPECT(Targets::kMaxEnvelopeStepDb > Floors::kEnvelopeStepDb);
            AM_EXPECT(Targets::kMaxSidebandDbc > Floors::kSidebandDbc);
            AM_EXPECT(Targets::kMaxRampErrorDb > Floors::kRampErrorDb);
            AM_EXPECT(Targets::kMaxPitchStepCents > Floors::kPitchStepCents);
            AM_EXPECT(Targets::kMaxPitchRmsDeviationCents > Floors::kPitchRmsCents);
            AM_EXPECT(Targets::kMaxNullResidualDbfs > Floors::kNullDbfs);
            AM_EXPECT(Targets::kMaxResumePositionErrorSamples > Floors::kPositionSamples);
            AM_EXPECT(Targets::kMaxLengthErrorSamples > Floors::kLengthSamples);
        }
    };

    AM_REGISTER_TEST(fidelity_analysis, targets_are_measurable);
} // namespace SparkyStudios::Audio::Amplitude::Tests
