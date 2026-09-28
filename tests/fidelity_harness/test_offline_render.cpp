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

#include <Fidelity/Analysis/Null.h>
#include <Fidelity/AssetGenerator.h>
#include <Fidelity/RenderSession.h>
#include <Fidelity/Signal.h>
#include <Fidelity/Stimuli.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    namespace
    {
        std::vector<TimedAction> PlayTone(std::uint64_t at)
        {
            const std::string sound = SoundName(*FindStimulus("sine_997_44100"), false);
            return { { at, "play", [sound]()
                       {
                           AM_UNUSED(amEngine->Play(amEngine->GetSoundHandle(sound)));
                       } } };
        }
    } // namespace

    AM_TEST_CASE(PureUnitTestCase, fidelity_harness, offline_render)
    {
    public:
        void Run() override
        {
            RenderSettings settings;
            settings.configFile = ConfigName("isolated", 1024, 48000, ".config.amconfig");
            settings.durationSamples = 24000;

            const RenderOutcome first = Render(kDefaultAssetsPath, settings, PlayTone(4000));
            const RenderOutcome second = Render(kDefaultAssetsPath, settings, PlayTone(4000));
            AM_EXPECT(first.error.empty());
            AM_EXPECT(second.error.empty());
            if (!first.error.empty() || !second.error.empty() || first.capture.channels.size() != 2)
                return;

            AM_EXPECT_EQ(first.capture.sampleRate, 48000);
            AM_EXPECT_EQ(first.capture.channels.size(), 2);
            AM_EXPECT_EQ(first.capture.channels[0].size(), 24000);

            // Bit-identical and audible.
            std::size_t differences = 0;
            for (std::size_t c = 0; c < first.capture.channels.size(); ++c)
                differences += CountDifferences(first.capture.channels[c], second.capture.channels[c]);
            AM_EXPECT_EQ(differences, 0);
            AM_EXPECT(Peak(ToSignal(first.capture.channels[0])) > 0.1);

            // The action ran on the frame at sample 4000 (the 6th frame at 800-sample spacing).
            bool actionAt4000 = false;
            for (const Marker& marker : first.capture.markers)
                if (marker.kind == MarkerKind::Action && marker.label == "play")
                    actionAt4000 = marker.sample == 4000;
            AM_EXPECT(actionAt4000);

            // A block size that does not match the config's buffer_size is an error, not a padded render.
            RenderSettings mismatched = settings;
            mismatched.blockSize = 256;
            AM_EXPECT(!Render(kDefaultAssetsPath, mismatched, {}).error.empty());

            // An unknown config is an error.
            RenderSettings missing = settings;
            missing.configFile = "fidelity.does_not_exist.amconfig";
            AM_EXPECT(!Render(kDefaultAssetsPath, missing, {}).error.empty());
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(fidelity_harness, offline_render);
} // namespace SparkyStudios::Audio::Amplitude::Tests
