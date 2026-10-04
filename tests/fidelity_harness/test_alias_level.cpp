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

#include <Fidelity/Scenarios/Common.h>
#include <Fidelity/Signal.h>
#include <Fidelity/Stimuli.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    // The alias level is relative to the tone as it would reach the output (after the centre pan), not to the raw asset.
    AM_TEST_CASE(PureUnitTestCase, fidelity_harness, alias_level)
    {
    public:
        void Run() override
        {
            const StimulusSpec* spec = FindStimulus("sine_30000_96000");
            AM_EXPECT(spec != nullptr);
            if (spec == nullptr)
                return;

            // A 30 kHz tone folded to 18 kHz at 48 kHz, 20 dB under the panned tone level.
            const double level = spec->amplitude * CenterPanGain() * AmplitudeFromDb(-20.0);
            const Signal folded = MakeSine(96000, 48000.0, 48000.0 - spec->frequencyHz, level);
            AM_EXPECT(std::abs(AliasLevelDbc(folded, 48000.0, *spec) - (-20.0)) <= 0.1);
        }
    };

    AM_REGISTER_TEST(fidelity_harness, alias_level);
} // namespace SparkyStudios::Audio::Amplitude::Tests
