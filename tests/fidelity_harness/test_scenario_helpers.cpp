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

#include <Fidelity/Analysis/Analytic.h>
#include <Fidelity/Scenarios/Common.h>
#include <Fidelity/Signal.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    AM_TEST_CASE(PureUnitTestCase, fidelity_harness, scenario_helpers)
    {
    public:
        void Run() override
        {
            // A silent capture is an error, not a perfect measurement.
            Capture silent;
            silent.sampleRate = 48000;
            silent.channels = { std::vector<float>(48000, 0.0f), std::vector<float>(48000, 0.0f) };
            Measurement quiet;
            AM_EXPECT(!RequireSignal(quiet, silent, 4800, 40000));
            AM_EXPECT(quiet.error.find("no signal") != std::string::npos);

            Capture tone = silent;
            tone.channels[0] = ToFloats(MakeSine(48000, 48000.0, 997.0, 0.5));
            Measurement loud;
            AM_EXPECT(RequireSignal(loud, tone, 4800, 40000));
            AM_EXPECT(loud.error.empty());

            // A clean tone has no clicks; an injected step is one event tied to the timeline.
            Capture stepped = tone;
            stepped.markers = { { 0, MarkerKind::Block, {} }, { 20000, MarkerKind::Action, "poke" }, { 20480, MarkerKind::Block, {} } };
            stepped.channels[0][24000] += 0.01f;
            Measurement clicks;
            AddClickMetrics(clicks, stepped, 4000.0, 4800, 44000);
            AM_EXPECT(clicks.Find("click.worstDbfs") != nullptr && clicks.Find("click.worstDbfs")->value > -60.0);
            AM_EXPECT_EQ(clicks.events.size(), 1);
            AM_EXPECT(!clicks.events.empty() && clicks.events[0].nearestAction == "poke");

            Measurement clean;
            AddClickMetrics(clean, tone, 4000.0, 4800, 44000);
            AM_EXPECT(clean.Find("click.worstDbfs") != nullptr && clean.Find("click.worstDbfs")->value == -400.0);

            Measurement integrity;
            AddIntegrityMetrics(integrity, tone, 4800, 40000);
            AM_EXPECT(integrity.Find("integrity.nan") != nullptr && integrity.Find("integrity.nan")->value == 0.0);
            AM_EXPECT(integrity.Find("integrity.dropouts") != nullptr);

            // Fade edges: a 480-sample linear fade-out starting at 30000 reaches zero at 30480.
            Signal faded = MakeSine(48000, 48000.0, 997.0, 0.5);
            for (std::size_t i = 30000; i < faded.size(); ++i)
                faded[i] *= std::max(0.0, 1.0 - static_cast<double>(i - 30000) / 480.0);
            const double zero = FadeZero(Envelope(faded), 0.5, 25000.0, false);
            AM_EXPECT(std::abs(zero - 30480.0) <= 2.0);

            AM_EXPECT(std::isnan(MedianFinite({})));
            AM_EXPECT(MedianFinite({ 3.0, std::nan(""), 1.0, 2.0 }) == 2.0);
            AM_EXPECT_EQ(Seconds(0.5, 48000.0), 24000);
            AM_EXPECT(CenterPanGain() > 0.25 && CenterPanGain() <= 1.0);
        }
    };

    AM_REGISTER_TEST(fidelity_harness, scenario_helpers);
} // namespace SparkyStudios::Audio::Amplitude::Tests
