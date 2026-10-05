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

#include <Fidelity/AssetGenerator.h>
#include <Fidelity/RenderSession.h>
#include <Fidelity/Scenario.h>
#include <Fidelity/Scenarios/Common.h>
#include <Fidelity/Stimuli.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    AM_TEST_CASE(PureUnitTestCase, fidelity_harness, resampler_option_selects_a_preset)
    {
    public:
        void Run() override
        {
            // A 44.1 kHz tone on a 48 kHz output differs between the linear preset and the default.
            const StimulusSpec* spec = FindStimulus("sine_5000_44100");
            AM_EXPECT_NOT(spec == nullptr);
            if (spec == nullptr)
                return;

            GridPoint point;
            RenderSettings settings = IsolatedSettings(point, kLeadIn + 9600);

            const RenderOutcome reference = Render(kDefaultAssetsPath, settings, PlayOnly(SoundName(*spec, false))());
            settings.resampler = "sinc_best";
            const RenderOutcome same = Render(kDefaultAssetsPath, settings, PlayOnly(SoundName(*spec, false))());
            settings.resampler = "linear";
            const RenderOutcome linear = Render(kDefaultAssetsPath, settings, PlayOnly(SoundName(*spec, false))());
            settings.resampler = "missing";
            const RenderOutcome missing = Render(kDefaultAssetsPath, settings, PlayOnly(SoundName(*spec, false))());

            AM_EXPECT(reference.error.empty());
            AM_EXPECT(same.error.empty());
            AM_EXPECT(linear.error.empty());
            AM_EXPECT_NOT(missing.error.empty());
            if (!reference.error.empty() || !linear.error.empty() || !same.error.empty())
                return;

            const auto differenceFrom = [&reference](const RenderOutcome& other)
            {
                double difference = 0.0;
                for (std::size_t i = kLeadIn + 2400; i < kLeadIn + 9600; ++i)
                    difference = std::max(difference, std::abs(double(reference.capture.channels[0][i]) - other.capture.channels[0][i]));

                return difference;
            };

            // The preset the default already uses renders identically: the alias forwards, it does not always differ.
            AM_EXPECT(differenceFrom(same) == 0.0);
            AM_EXPECT(differenceFrom(linear) > 1e-4);
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(fidelity_harness, resampler_option_selects_a_preset);
} // namespace SparkyStudios::Audio::Amplitude::Tests