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

#include <numbers>
#include <set>

#include <Fidelity/Analysis/FrequencyResponse.h>
#include <Fidelity/Analysis/Pitch.h>
#include <Fidelity/Stimuli.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    AM_TEST_CASE(PureUnitTestCase, fidelity_analysis, stimulus_catalog)
    {
    public:
        void Run() override
        {
            std::set<std::string> names;
            for (const StimulusSpec& spec : StimulusCatalog())
            {
                AM_EXPECT(names.insert(spec.name).second);
                AM_EXPECT(FindStimulus(spec.name) == &spec);

                if (spec.kind == StimulusKind::LoopSine)
                {
                    // Whole cycles, so the loop point is seamless.
                    const double cycles = static_cast<double>(StimulusFrameCount(spec)) * spec.frequencyHz / spec.sampleRate;
                    AM_EXPECT(std::abs(cycles - std::round(cycles)) < 1e-9);
                }
            }

            AM_EXPECT(FindStimulus("does_not_exist") == nullptr);

            const StimulusSpec* sine = FindStimulus("sine_997_48000");
            AM_EXPECT(sine != nullptr);
            if (sine != nullptr)
            {
                const Signal x = RenderStimulusChannel(*sine);
                AM_EXPECT_EQ(x.size(), 96000);
                AM_EXPECT(x[0] == 0.0);
                const double expected = 0.5 * std::sin(2.0 * std::numbers::pi * std::fmod(48001.0 * 997.0 / 48000.0, 1.0));
                AM_EXPECT(std::abs(x[48001] - expected) < 1e-12);
            }

            const StimulusSpec* stereo = FindStimulus("sine_997_48000_stereo");
            AM_EXPECT(stereo != nullptr && stereo->channels == 2);
            if (stereo != nullptr)
            {
                const std::vector<float> interleaved = RenderStimulusInterleaved(*stereo);
                AM_EXPECT_EQ(interleaved.size(), 2 * StimulusFrameCount(*stereo));
                AM_EXPECT(interleaved[2 * 1000] == interleaved[2 * 1000 + 1]);
            }

            const StimulusSpec* pink = FindStimulus("pink_48000");
            AM_EXPECT(pink != nullptr);
            if (pink != nullptr)
            {
                const Signal x = RenderStimulusChannel(*pink);
                const std::span<const double> middle(x.data() + 4800, x.size() - 9600);
                AM_EXPECT(std::abs(Rms(middle) - pink->amplitude) <= 0.01 * pink->amplitude + 0.005);
                AM_EXPECT(Peak(x) < 1.0);
            }

            const StimulusSpec* chirp = FindStimulus("chirp_44100");
            AM_EXPECT(chirp != nullptr);
            if (chirp != nullptr)
            {
                const Signal x = RenderStimulusChannel(*chirp);
                const double tau = 20000.0 / chirp->sampleRate;
                const double expected =
                    chirp->amplitude * std::sin(2.0 * std::numbers::pi * std::fmod(ChirpCycles(ChirpModelOf(*chirp), tau), 1.0));
                AM_EXPECT(std::abs(x[20000] - expected) < 1e-9);
            }

            const StimulusSpec* sweep = FindStimulus("sweep_44100");
            AM_EXPECT(sweep != nullptr);
            if (sweep != nullptr)
            {
                const Signal x = RenderStimulusChannel(*sweep);
                const Signal reference = RenderLogSweep(SweepModelOf(*sweep), sweep->sampleRate, sweep->frequencyEndHz);
                AM_EXPECT(x == reference);
            }
        }
    };

    AM_REGISTER_TEST(fidelity_analysis, stimulus_catalog);
} // namespace SparkyStudios::Audio::Amplitude::Tests
