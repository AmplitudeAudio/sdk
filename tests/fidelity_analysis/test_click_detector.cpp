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

#include <Fidelity/Analysis/Click.h>
#include <Fidelity/Signal.h>
#include <Fidelity/Targets.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    namespace
    {
        constexpr double kRate = 48000.0;
        constexpr std::size_t kLead = 4800;

        // 2 s tone with 20 ms raised-cosine fades, 0.1 s of silence on each side, rounded through float like a capture.
        Signal FadedTone(double frequency)
        {
            Signal tone = MakeSine(96000, kRate, frequency, 0.5);
            ApplyFades(tone, 960);

            Signal x(kLead, 0.0);
            x.insert(x.end(), tone.begin(), tone.end());
            x.insert(x.end(), kLead, 0.0);

            return ToSignal(ToFloats(x));
        }
    } // namespace

    AM_TEST_CASE(PureUnitTestCase, fidelity_analysis, click_detector)
    {
    public:
        void Run() override
        {
            // Clean tones: no event, band peak under the floor.
            for (const double frequency : { 100.0, 997.0, 3000.0 })
            {
                ClickOptions options;
                options.highPassHz = std::max(4000.0, 2.0 * frequency);
                const ClickResult clean = AnalyzeClicks(FadedTone(frequency), kRate, options);
                AM_EXPECT(clean.events.empty());
                AM_EXPECT(clean.maxPeakDbfs <= Floors::kClickDbfs);
            }

            // A -80 dBFS single-sample step is found at the right sample and level.
            {
                Signal x = FadedTone(997.0);
                const std::size_t at = 50000;
                for (std::size_t i = at; i < x.size(); ++i)
                    x[i] += 1e-4;

                const ClickResult r = AnalyzeClicks(x, kRate, ClickOptions{});
                AM_EXPECT_EQ(r.events.size(), 1);
                if (r.events.size() == 1)
                {
                    AM_EXPECT(r.events[0].sample + 1 >= at && r.events[0].sample <= at + 1);
                    AM_EXPECT(r.events[0].peakDbfs >= -90.0 && r.events[0].peakDbfs <= -85.0);
                }
            }

            // Two steps far apart are two events.
            {
                Signal x = FadedTone(997.0);
                x[30000] += 1e-3;
                x[70000] += 1e-3;
                AM_EXPECT_EQ(AnalyzeClicks(x, kRate, ClickOptions{}).events.size(), 2);
            }

            // A dropped sample and a repeated sample are located.
            {
                Signal dropped = FadedTone(997.0);
                dropped.erase(dropped.begin() + 40000);
                const ClickResult r = AnalyzeClicks(dropped, kRate, ClickOptions{});
                AM_EXPECT_EQ(r.events.size(), 1);
                AM_EXPECT(!r.events.empty() && r.events[0].sample + 1 >= 40000 && r.events[0].sample <= 40001);
                AM_EXPECT(!r.events.empty() && r.events[0].peakDbfs > -60.0);

                Signal repeated = FadedTone(997.0);
                repeated.insert(repeated.begin() + 40000, repeated[40000]);
                const ClickResult s = AnalyzeClicks(repeated, kRate, ClickOptions{});
                AM_EXPECT_EQ(s.events.size(), 1);
                AM_EXPECT(!s.events.empty() && s.events[0].sample + 1 >= 40000 && s.events[0].sample <= 40001);
            }
        }
    };

    AM_REGISTER_TEST(fidelity_analysis, click_detector);
} // namespace SparkyStudios::Audio::Amplitude::Tests
