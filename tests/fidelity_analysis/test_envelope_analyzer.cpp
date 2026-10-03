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

#include <Fidelity/Analysis/Analytic.h>
#include <Fidelity/Analysis/Envelope.h>
#include <Fidelity/Signal.h>
#include <Fidelity/Targets.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    AM_TEST_CASE(PureUnitTestCase, fidelity_analysis, envelope_analyzer)
    {
    public:
        void Run() override
        {
            constexpr double fs = 48000.0;
            constexpr std::size_t n = 96000;
            const Signal carrier = MakeSine(n, fs, 997.0, 0.5);

            // A +0.1 dB staircase every 2048 samples, 20 steps.
            {
                Signal x = carrier;
                for (std::size_t i = 0; i < n; ++i)
                {
                    const std::size_t steps = i < 20000 ? 0 : std::min<std::size_t>(20, (i - 20000) / 2048 + 1);
                    x[i] *= AmplitudeFromDb(0.1 * static_cast<double>(steps));
                }

                EnvelopeOptions options;
                options.begin = 8000;
                options.end = n - 8000;
                const EnvelopeResult r = AnalyzeEnvelope(x, fs, options);
                AM_EXPECT(r.largestStepDb >= 0.09 && r.largestStepDb <= 0.13);
                AM_EXPECT(r.stepCount >= 19 && r.stepCount <= 21);
            }

            // A smooth 6 dB ramp matches its expected envelope.
            {
                Signal x = carrier;
                Signal expected(n);
                for (std::size_t i = 0; i < n; ++i)
                {
                    const double t = std::clamp((static_cast<double>(i) - 24000.0) / 48000.0, 0.0, 1.0);
                    const double gain = AmplitudeFromDb(-6.0 * t);
                    x[i] *= gain;
                    expected[i] = 0.5 * gain;
                }

                EnvelopeOptions options;
                options.begin = 8000;
                options.end = n - 8000;
                options.expected = expected;
                const EnvelopeResult r = AnalyzeEnvelope(x, fs, options);
                AM_EXPECT(r.maxErrorDb <= Floors::kRampErrorDb);
                AM_EXPECT(r.largestStepDb <= Floors::kEnvelopeStepDb);
                AM_EXPECT_EQ(r.stepCount, 0);
            }

            // Sinusoidal AM at the 1024-sample block rate: sidebands at 20 log10(m / 2), modulation noise at m / sqrt(2).
            {
                const double m = 2e-4;
                const double rate = fs / 1024.0;
                Signal x = carrier;
                for (std::size_t i = 0; i < n; ++i)
                    x[i] *= 1.0 + m * std::sin(2.0 * std::numbers::pi * rate * static_cast<double>(i) / fs);

                EnvelopeOptions options;
                options.begin = 8000;
                options.end = n - 8000;
                options.updateRatesHz = { rate };
                const EnvelopeResult r = AnalyzeEnvelope(x, fs, options);
                AM_EXPECT(std::abs(r.worstSidebandDbc - DbFromAmplitude(m / 2.0)) <= 1.0);
                AM_EXPECT(std::abs(r.modulationNoiseDbc - DbFromAmplitude(m / std::sqrt(2.0))) <= 1.0);
            }

            // A clean tone has no sidebands above the floor.
            {
                EnvelopeOptions options;
                options.begin = 8000;
                options.end = n - 8000;
                options.updateRatesHz = { fs / 1024.0, 60.0 };
                const EnvelopeResult r = AnalyzeEnvelope(ToSignal(ToFloats(carrier)), fs, options);
                AM_EXPECT(r.worstSidebandDbc <= Floors::kSidebandDbc);
            }

            // Fade timing: a 480-sample linear fade to silence spans 8 ms between its 10 % and 90 % points.
            {
                Signal x = carrier;
                for (std::size_t i = 48000; i < n; ++i)
                    x[i] *= std::max(0.0, 1.0 - static_cast<double>(i - 48000) / 480.0);

                const Signal envelope = Envelope(x);
                const FadeTiming fade = MeasureFade(envelope, fs, 0.5, 0.0, 40000.0);
                AM_EXPECT(fade.found);
                AM_EXPECT(std::abs(fade.durationMs - 8.0) <= 0.2);
                AM_EXPECT(std::abs(fade.start - 48048.0) <= 2.0);
            }

            // Crossings of a raised-cosine-faded tone land on the fade midpoints.
            {
                Signal tone = carrier;
                ApplyFades(tone, 960);
                Signal x(4800, 0.0);
                x.insert(x.end(), tone.begin(), tone.end());
                x.insert(x.end(), 4800, 0.0);

                const Signal envelope = Envelope(x);
                const double rise = CrossingTime(envelope, 0.25, 0.0, true);
                const double fall = CrossingTime(envelope, 0.25, 50000.0, false);
                AM_EXPECT(std::abs(rise - 5280.0) <= Floors::kLengthSamples);
                AM_EXPECT(std::abs(fall - (4800.0 + 96000.0 - 1.0 - 480.0)) <= Floors::kLengthSamples);
            }
        }
    };

    AM_REGISTER_TEST(fidelity_analysis, envelope_analyzer);
} // namespace SparkyStudios::Audio::Amplitude::Tests
