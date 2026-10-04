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
#include <numbers>

#include <Fidelity/Analysis/Pitch.h>
#include <Fidelity/Scenarios/Common.h>
#include <Fidelity/Scenarios/Timing.h>
#include <Fidelity/Signal.h>
#include <Fidelity/Stimuli.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    namespace
    {
        constexpr double kRate = 48000.0;
        constexpr std::uint64_t kPlay = 4800;

        Capture Stereo(const Signal& x)
        {
            Capture capture;
            capture.sampleRate = static_cast<std::uint32_t>(kRate);
            capture.channels = { ToFloats(x), ToFloats(x) };
            return capture;
        }

        // What the engine would output for `spec` played at kPlay: the asset (with its own fades) at `gain`, resampled
        // to 48 kHz, with `envelope(i)` applied (stop fades, cuts).
        template<typename Envelope>
        Signal Played(const StimulusSpec& spec, std::size_t length, double gain, Envelope envelope)
        {
            const double level = spec.amplitude * CenterPanGain() * gain;
            const std::size_t frames = static_cast<std::size_t>(OutputFrames(spec, static_cast<std::uint32_t>(kRate)));
            const std::size_t fade = static_cast<std::size_t>(std::llround(spec.fadeSeconds * kRate));
            Signal x(length, 0.0);
            for (std::size_t n = 0; n < frames && kPlay + n < length; ++n)
            {
                const double tone = std::sin(2.0 * std::numbers::pi * std::fmod(static_cast<double>(n) * spec.frequencyHz / kRate, 1.0));
                x[kPlay + n] = level * FadeGain(n, frames, fade) * tone * envelope(kPlay + n);
            }

            return x;
        }

        double Value(const Measurement& m, const char* name)
        {
            const Metric* metric = m.Find(name);
            return metric != nullptr ? metric->value : std::nan("");
        }
    } // namespace

    // The timing metrics of P2, P3 and P7 on captures shaped like the engine's real output.
    AM_TEST_CASE(PureUnitTestCase, fidelity_harness, timing_measurements)
    {
    public:
        void Run() override
        {
            const StimulusSpec& sine = *FindStimulus("sine_997_44100");
            const std::uint64_t stopAt = kPlay + 48000;

            // Stop() that cuts at the next block boundary instead of fading: a near-instant fade that misses the curve.
            {
                Measurement m;
                MeasureStopTiming(
                    Stereo(Played(
                        sine, 80000, 1.0,
                        [](std::size_t i)
                        {
                            return i < 54272 ? 1.0 : 0.0;
                        })),
                    sine, kPlay, stopAt, kMinFadeDuration, m);
                AM_EXPECT(m.error.empty());
                AM_EXPECT(std::abs(Value(m, "start.latencyMs")) <= 0.1);
                AM_EXPECT(Value(m, "stop.fadeMs") <= 2.5);
                AM_EXPECT(Value(m, "stop.fadeCurveErrorDb") >= 3.0);
                AM_EXPECT(std::abs(Value(m, "stop.latencyMs") - (54272.0 - 240.0 - static_cast<double>(stopAt)) / 48.0) <= 0.1);
            }

            // A real 10 ms linear fade starting 200 samples after the stop.
            {
                const auto fade = [](std::size_t i)
                {
                    return std::clamp(1.0 - (static_cast<double>(i) - 53000.0) / 480.0, 0.0, 1.0);
                };

                Measurement m;
                MeasureStopTiming(Stereo(Played(sine, 80000, 1.0, fade)), sine, kPlay, stopAt, kMinFadeDuration, m);
                AM_EXPECT(m.error.empty());
                AM_EXPECT(std::abs(Value(m, "stop.latencyMs") - 200.0 / 48.0) <= 0.1);
                AM_EXPECT(std::abs(Value(m, "stop.fadeMs") - 8.0) <= 0.3);
                AM_EXPECT(Value(m, "stop.fadeCurveErrorDb") <= 0.1);
                AM_EXPECT(Value(m, "stop.tailDbfs") <= -120.0);
            }

            // End of sound 2.5 dB under nominal, with a sample zeroed every 1024 samples.
            {
                Signal x = Played(
                    sine, 110000, AmplitudeFromDb(-2.5),
                    [](std::size_t)
                    {
                        return 1.0;
                    });
                for (std::size_t i = 1023; i < x.size(); i += 1024)
                    x[i] = 0.0;

                Measurement m;
                MeasureEndOfSound(Stereo(x), sine, kPlay, OutputFrames(sine, 48000), m);
                AM_EXPECT(m.error.empty());
                AM_EXPECT(Value(m, "length.errorSamples") <= 2.0);
                AM_EXPECT(Value(m, "tail.peakDbfs") <= -120.0);
            }

            // Pause and resume with 10 ms linear fades; the source continues exactly where the fade-out ended.
            {
                const StimulusSpec& chirp = *FindStimulus("chirp_44100");
                const ChirpModel model = ChirpModelOf(chirp);
                const double level = chirp.amplitude * CenterPanGain();
                const std::uint64_t pauseAt = kPlay + 48000;
                const std::uint64_t resumeAt = kPlay + 72000;
                const std::uint64_t pauseEnd = pauseAt + 300 + 480; // fade-out starts 300 samples after the pause call
                const std::uint64_t resumeStart = resumeAt + 150; // fade-in starts 150 samples after the resume call

                Signal x(kPlay + 120000, 0.0);
                for (std::uint64_t i = kPlay; i < x.size(); ++i)
                {
                    double gain = 1.0;
                    double sourceSamples = static_cast<double>(i - kPlay);
                    if (i >= pauseAt + 300 && i < resumeStart)
                        gain = std::clamp(1.0 - static_cast<double>(i - pauseAt - 300) / 480.0, 0.0, 1.0);
                    else if (i >= resumeStart)
                    {
                        gain = std::clamp(static_cast<double>(i - resumeStart) / 480.0, 0.0, 1.0);
                        sourceSamples = static_cast<double>(pauseEnd - kPlay) + static_cast<double>(i - resumeStart);
                    }

                    const double tau = sourceSamples / kRate;
                    x[i] = level * gain * std::sin(2.0 * std::numbers::pi * std::fmod(ChirpCycles(model, tau), 1.0));
                }

                Measurement m;
                AM_EXPECT(MeasurePauseResume(Stereo(x), chirp, pauseAt, resumeAt, m));
                AM_EXPECT(m.error.empty());
                AM_EXPECT(Value(m, "resume.positionErrorSamples") <= 1.0);
                AM_EXPECT(std::abs(Value(m, "pause.fadeMs") - 8.0) <= 0.3);
                AM_EXPECT(std::abs(Value(m, "resume.fadeMs") - 8.0) <= 0.3);
            }
        }
    };

    AM_REGISTER_TEST(fidelity_harness, timing_measurements);
} // namespace SparkyStudios::Audio::Amplitude::Tests
