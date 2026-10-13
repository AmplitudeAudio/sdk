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

#include <string>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Fidelity/Analysis/Click.h>
#include <Fidelity/AssetGenerator.h>
#include <Fidelity/RenderSession.h>
#include <Fidelity/Signal.h>
#include <Fidelity/Stimuli.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    // A device callback larger than the mixer's nominal block (1024 frames here) is rendered in sub-blocks, each mixed at a
    // non-zero offset of the output. A misplaced sub-block shows up as a discontinuity in a steady tone.
    AM_TEST_CASE(PureUnitTestCase, fidelity_harness, render_split_into_sub_blocks)
    {
    public:
        void Run() override
        {
            const std::string sound = SoundName(*FindStimulus("sine_997_48000"), false);

            RenderSettings settings;
            settings.configFile = ConfigName("isolated", 1024, 48000, ".config.amconfig");
            settings.durationSamples = 40000;
            settings.blockSequence = { 3000, 1500, 2048, 777, 1024, 2600, 1025 };

            const std::vector<TimedAction> actions = { { 4000, "play",
                                                         [sound]()
                                                         {
                                                             AM_UNUSED(amEngine->Play(amEngine->GetSoundHandle(sound)));
                                                         } } };

            const RenderOutcome outcome = Render(kDefaultAssetsPath, settings, actions);
            AM_EXPECT(outcome.error.empty());
            if (!outcome.error.empty() || outcome.capture.channels.empty())
                return;

            const Signal tone = ToSignal(outcome.capture.channels[0]);
            AM_EXPECT(Peak(tone) > 0.1);

            ClickOptions options;
            options.guard = 8000; // past the onset
            const ClickResult clicks = AnalyzeClicks(tone, outcome.capture.sampleRate, options);
            AM_EXPECT(clicks.maxPeakDbfs < -80.0);
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(fidelity_harness, render_split_into_sub_blocks);
} // namespace SparkyStudios::Audio::Amplitude::Tests
