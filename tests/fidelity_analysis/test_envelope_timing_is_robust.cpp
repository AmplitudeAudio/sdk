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

#include <Fidelity/Analysis/Envelope.h>
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

        // 2 s tone with 20 ms fades placed at `at` in a 110000-sample capture, scaled by `gain`.
        Signal PlacedTone(double gain, std::size_t at = 4800)
        {
            Signal tone = MakeSine(96000, kRate, 997.0, 0.5 * gain);
            ApplyFades(tone, 960);
            Signal x(110000, 0.0);
            for (std::size_t i = 0; i < tone.size(); ++i)
                x[at + i] = tone[i];

            return x;
        }
    } // namespace

    // Engine captures are not clean: a sample zeroed every block, a plateau off the nominal level, hard cuts. Timing must
    // come from the measured plateau and survive single-sample dips.
    AM_TEST_CASE(PureUnitTestCase, fidelity_analysis, envelope_timing_is_robust)
    {
    public:
        void Run() override
        {
            // A zeroed sample every 1024 samples: the fade midpoints stay within 2 samples (a dip near the midpoint pulls
            // the smoothed crossing by up to ~1.6 samples; the raw envelope was off by 33 and the old search by 45000).
            {
                Signal x = PlacedTone(1.0);
                for (std::size_t i = 1023; i < x.size(); i += 1024)
                    x[i] = 0.0;

                const Signal envelope = TimingEnvelope(x);
                const double plateau = PlateauLevel(envelope, 20000, 90000);
                AM_EXPECT(std::abs(DbFromAmplitude(plateau / 0.5)) <= 0.1);

                const double rise = SustainedCrossing(envelope, 0.5 * plateau, 0.0, true, kTimingHold);
                const double fall = LastFallingCrossing(envelope, 0.5 * plateau);
                AM_EXPECT(std::abs(rise - 5280.0) <= 2.0);
                AM_EXPECT(std::abs(fall - 100319.0) <= 2.0);
                AM_EXPECT(std::abs(SustainedCrossing(envelope, 0.5 * plateau, 50000.0, false, kTimingHold) - 100319.0) <= 2.0);
            }

            // A plateau 2.5 dB under nominal: the measured plateau puts the crossings back on the fade midpoints.
            {
                const Signal envelope = TimingEnvelope(PlacedTone(AmplitudeFromDb(-2.5)));
                const double plateau = PlateauLevel(envelope, 20000, 90000);
                AM_EXPECT(std::abs(DbFromAmplitude(plateau / 0.5) - (-2.5)) <= 0.05);
                AM_EXPECT(std::abs(SustainedCrossing(envelope, 0.5 * plateau, 0.0, true, kTimingHold) - 5280.0) <= 1.0);
                AM_EXPECT(std::abs(LastFallingCrossing(envelope, 0.5 * plateau) - 100319.0) <= 1.0);
            }

            // A hard cut at a block boundary is found within 3 samples of the cut (the analytic-signal tail after the cut
            // pulls the smoothed crossing ~2 samples late), and reads as a smoothing-wide, near-instant fade.
            {
                Signal x = PlacedTone(1.0);
                for (std::size_t i = 54272; i < x.size(); ++i)
                    x[i] = 0.0;

                const Signal envelope = TimingEnvelope(x);
                const double plateau = PlateauLevel(envelope, 20000, 50000);
                const double cut = SustainedCrossing(envelope, 0.5 * plateau, 52800.0, false, kTimingHold);
                AM_EXPECT(std::abs(cut - 54272.0) <= 3.0);
                const double departure = SustainedCrossing(envelope, 0.99 * plateau, 52800.0, false, kTimingHold);
                AM_EXPECT(departure > 54272.0 - static_cast<double>(kTimingSmoothing) && departure < 54272.0);
                const FadeTiming fade = MeasureFade(envelope, kRate, plateau, 0.0, departure - 1.0);
                AM_EXPECT(fade.found && fade.durationMs <= 2.5);
            }
        }
    };

    AM_REGISTER_TEST(fidelity_analysis, envelope_timing_is_robust);
} // namespace SparkyStudios::Audio::Amplitude::Tests
