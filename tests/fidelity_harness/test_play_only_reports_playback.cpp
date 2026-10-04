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

#include <memory>

#include <Fidelity/AssetGenerator.h>
#include <Fidelity/Scenarios/Common.h>
#include <Fidelity/Stimuli.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    // A scenario that expects silence (a tone above the output band) must still know whether the sound played.
    AM_TEST_CASE(PureUnitTestCase, fidelity_harness, play_only_reports_playback)
    {
    public:
        void Run() override
        {
            const RenderSettings settings = IsolatedSettings(GridPoint{}, 9600);

            auto played = std::make_shared<bool>(false);
            const RenderOutcome real =
                Render(kDefaultAssetsPath, settings, PlayOnly(SoundName(*FindStimulus("sine_997_48000"), false), played)());
            AM_EXPECT(real.error.empty());
            AM_EXPECT(*played);

            auto missing = std::make_shared<bool>(true);
            const RenderOutcome none = Render(kDefaultAssetsPath, settings, PlayOnly("fidelity.does_not_exist", missing)());
            AM_EXPECT(none.error.empty());
            AM_EXPECT(!*missing);
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(fidelity_harness, play_only_reports_playback);
} // namespace SparkyStudios::Audio::Amplitude::Tests
