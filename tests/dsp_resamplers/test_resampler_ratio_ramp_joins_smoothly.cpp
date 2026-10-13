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

#include <cmath>
#include <complex>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        constexpr double kPi = 3.14159265358979323846;
        constexpr double kTwoPi = 2.0 * kPi;

        using Complex = std::complex<AmReal64>;

        /// The source tone of the glide, and the rate it is written at.
        constexpr double kSourceHz = 3000.0;

        std::vector<AmReal32> Sine(AmUInt64 frames, AmReal64 hz, AmReal64 rate)
        {
            std::vector<AmReal32> x(frames);
            for (AmUInt64 i = 0; i < frames; ++i)
                x[i] = static_cast<AmReal32>(0.5 * std::sin(2.0 * kPi * hz * static_cast<AmReal64>(i) / rate));
            return x;
        }

        /// Renders @p frames output frames from @p sourceOffset and returns the source frames they consumed.
        AmUInt64 Pull(
            ResamplerInstance& resampler,
            const std::vector<AmReal32>& source,
            AmUInt64 sourceOffset,
            AmUInt64 frames,
            std::vector<AmReal32>& out)
        {
            const AmUInt64 available = static_cast<AmUInt64>(source.size()) - sourceOffset;
            AudioBuffer in(available, 1);
            for (AmUInt64 i = 0; i < available; ++i)
                in[0][i] = source[sourceOffset + i];

            AudioBuffer o(frames, 1);
            AmUInt64 inFrames = resampler.GetInputFramesNeeded(frames);
            AmUInt64 outFrames = frames;
            if (!resampler.Process(in, inFrames, o, outFrames))
                return 0;

            for (AmUInt64 i = 0; i < outFrames; ++i)
                out.push_back(o[0][i]);
            return inFrames;
        }

        /// Periodic 7-term Blackman-Harris: the bins this test reads are only 32 steps from the carrier, which is far
        /// too close for a rectangular window's sidelobes to be ignored at the levels being compared.
        std::vector<AmReal64> Window(AmUInt64 frames)
        {
            constexpr AmReal64 kCoefficients[] = { 0.27105140069342, 0.43329793923448, 0.21812299954311, 0.06592544638803,
                                                   0.01081174209837, 0.00077658482522, 0.00001388721735 };

            std::vector<AmReal64> window(frames);
            for (AmUInt64 i = 0; i < frames; ++i)
            {
                const auto t = kTwoPi * static_cast<AmReal64>(i) / static_cast<AmReal64>(frames);
                window[i] = kCoefficients[0] - kCoefficients[1] * std::cos(t) + kCoefficients[2] * std::cos(2.0 * t) -
                    kCoefficients[3] * std::cos(3.0 * t) + kCoefficients[4] * std::cos(4.0 * t) - kCoefficients[5] * std::cos(5.0 * t) +
                    kCoefficients[6] * std::cos(6.0 * t);
            }

            return window;
        }

        /**
         * @brief Energy at the block-rate sidebands of a glide, against the carrier it rides on.
         *
         * The tone is demodulated by its expected phase first: a spectrum of the glide itself would smear the carrier
         * over the whole sweep. What is left is the render's phase error.
         *
         * @param[in]  x           Rendered samples.
         * @param[in]  phase       The expected phase of x, in radians.
         * @param[in]  rate        The output sample rate.
         * @param[in]  orders      How many multiples of the block rate the sideband total sums over.
         * @param[out] carrierDbfs The demodulated carrier against a full-scale sine, in dBFS.
         *
         * @return The loudest sideband against the carrier, in dBc.
         */
        AmReal64 Sidebands(
            const std::vector<AmReal32>& x, const std::vector<AmReal64>& phase, AmReal64 rate, AmUInt64 orders, AmReal64& carrierDbfs)
        {
            carrierDbfs = -400.0;

            const AmUInt64 frames = phase.size();
            const AmReal64 blockRate = rate / 1024.0;
            const auto binOf = [&](AmReal64 hz)
            {
                return static_cast<AmInt64>(std::llround(hz * static_cast<AmReal64>(frames) / rate));
            };

            const std::vector<AmReal64> window = Window(frames);

            // Demodulated once: every bin below is read from this baseband, where the carrier sits at 0 Hz.
            std::vector<Complex> baseband(frames);
            Complex carrier{ 0.0, 0.0 };
            AmReal64 peak = 0.0;
            for (AmUInt64 n = 0; n < frames; ++n)
            {
                const auto v = static_cast<AmReal64>(x[n]) * window[n];
                baseband[n] = Complex(v * std::cos(phase[n]), -v * std::sin(phase[n]));
                carrier += baseband[n];
                peak = AM_MAX(peak, std::abs(static_cast<AmReal64>(x[n])));
            }

            if (!(peak > 0.0) || !(std::norm(carrier) > 0.0))
                return 0.0;

            // The window's mean, its constant term: a tone of amplitude A sums to A/2 times this times the frame count.
            constexpr AmReal64 kCoherentGain = 0.27105140069342;
            const AmReal64 reference = static_cast<AmReal64>(frames) * kCoherentGain / 2.0;
            carrierDbfs = 20.0 * std::log10(std::sqrt(std::norm(carrier)) / reference);

            // The carrier's own main lobe runs a few bins either side of the centre and is not a sideband.
            const AmInt64 first = AM_MAX(static_cast<AmInt64>(1), binOf(0.25 * blockRate));
            const AmInt64 last = binOf(static_cast<AmReal64>(orders) * blockRate);
            AmReal64 worst = 0.0;

            for (AmInt64 k = first; k <= last; ++k)
            {
                for (const int sign : { 1, -1 })
                {
                    // One rotation per bin, carried along the window instead of a trigonometric call per sample.
                    const Complex step =
                        std::polar(1.0, kTwoPi * static_cast<AmReal64>(sign) * static_cast<AmReal64>(k) / static_cast<AmReal64>(frames));

                    Complex rotation = 1.0;
                    Complex bin{ 0.0, 0.0 };
                    for (AmUInt64 n = 0; n < frames; ++n)
                    {
                        bin += baseband[n] * rotation;
                        rotation *= step;
                    }

                    worst = AM_MAX(worst, std::norm(bin));
                }
            }

            return 10.0 * std::log10(worst / std::norm(carrier));
        }
    } // namespace

    AM_TEST_CASE(DSPTestCase, dsp_resamplers, resampler_ratio_ramp_joins_smoothly)
    {
    public:
        void Run() override
        {
            // Two ramps published back to back: the second starts where the first ended. They span different amounts,
            // so a shared end value cannot be the only thing keeping the total right.
            constexpr AmReal64 kFirst = 0.5;
            constexpr AmReal64 kMiddle = 1.25;
            constexpr AmReal64 kLast = 3.0;
            constexpr AmUInt64 kFrames = 1024;
            constexpr AmUInt64 kProbe = 128;
            constexpr AmReal64 kRate = 48000.0;
            constexpr auto kProbes = static_cast<AmSize>(kFrames / kProbe);

            const auto meanFrames = [](AmReal64 start, AmReal64 end)
            {
                return static_cast<AmReal64>(static_cast<AmReal64>(kFrames) * 0.5 * (start + end));
            };

            for (const char* name : kResamplerPresets)
            {
                const std::vector<AmReal32> source = Sine(kFrames * 8 + 8192, 3000.0, kRate);

                auto joined = Resampler::Construct(name);
                joined->Initialize(1, 48000, 48000);
                std::vector<AmReal32> across;
                AmUInt64 read = 0;

                // Pulled in probes, so a ramp also spans several Process() calls.
                joined->SetRatioRamp(kFirst, kMiddle, kFrames);
                for (AmUInt64 i = 0; i < kFrames; i += kProbe)
                    read += Pull(*joined, source, read, kProbe, across);
                const AmUInt64 firstConsumed = read;

                joined->SetRatioRamp(kMiddle, kLast, kFrames);

                // A ramp in progress reports its mean.
                if (const auto* running = dynamic_cast<const BandlimitedResamplerInstance*>(joined.get()); running != nullptr)
                    AM_EXPECT(std::abs(running->GetRatio() - 0.5 * (kMiddle + kLast)) < 1e-12);

                for (AmUInt64 i = 0; i < kFrames; i += kProbe)
                    read += Pull(*joined, source, read, kProbe, across);
                const AmUInt64 secondConsumed = read - firstConsumed;

                AM_EXPECT_EQ(across.size(), static_cast<AmSize>(2 * kFrames));

                // Each ramp advances the stream by its own mean.
                AM_EXPECT(std::abs(static_cast<double>(firstConsumed) - static_cast<double>(meanFrames(kFirst, kMiddle))) <= 1.0);
                AM_EXPECT(std::abs(static_cast<double>(secondConsumed) - static_cast<double>(meanFrames(kMiddle, kLast))) <= 1.0);

                // A spent ramp holds its end.
                const auto* instance = dynamic_cast<const BandlimitedResamplerInstance*>(joined.get());
                AM_EXPECT(instance != nullptr);
                if (instance != nullptr)
                    AM_EXPECT(std::abs(instance->GetRatio() - kLast) < 1e-12);

                // The shape inside each block: a ramp that bends the read position bends it the same way every block,
                // which phase-modulates the tone at the block rate. A steady glide is demodulated and the energy at the
                // first three multiples of the block rate read against its carrier.
                constexpr AmReal64 kGlideFrom = 0.9;
                constexpr AmReal64 kGlideTo = 1.1;
                constexpr AmUInt64 kBlocks = 96;
                constexpr AmUInt64 kMeasured = 32;

                // A smoothstep bows the read position by delta * N / 32: 0.067 of a frame here, about -44 dBc at 3 kHz.
                // The kernels' own block-rate content stays well under the gate: -84 dBc for linear, -138 for the sincs.
                constexpr AmReal64 kMaxSidebandDbc = -60.0;

                // A render out of step with the reference demodulates to about -43 dBFS over this window, a dead one
                // to -400.
                constexpr AmReal64 kMinCarrierDbfs = -25.0;
                const auto kDelta = static_cast<AmReal64>(kGlideTo - kGlideFrom) / static_cast<AmReal64>(kBlocks);

                auto glider = Resampler::Construct(name);
                glider->Initialize(1, 48000, 48000);

                const std::vector<AmReal32> glideSource = Sine(kFrames * kBlocks + 16384, kSourceHz, kRate);

                std::vector<AmReal32> glide;
                glide.reserve(kFrames * kBlocks);
                AmUInt64 glideRead = 0;
                for (AmUInt64 block = 0; block < kBlocks; ++block)
                {
                    const auto from = kGlideFrom + kDelta * static_cast<AmReal64>(block);
                    glider->SetRatioRamp(from, from + kDelta, kFrames);
                    glideRead += Pull(*glider, glideSource, glideRead, kFrames, glide);
                }

                AM_EXPECT_EQ(glide.size(), static_cast<AmSize>(kFrames * kBlocks));

                // The last kMeasured blocks, so the analysis window holds a whole number of them.
                const std::vector<AmReal32> window(glide.end() - static_cast<AmInt64>(kMeasured * kFrames), glide.end());

                // The expected phase, at a read position that runs straight through every block. The rendered position
                // of frame j sums the steps before it, so the reference steps at j, not j + 0.5: half a frame of delta
                // per frame would read as sidebands of its own.
                std::vector<AmReal64> phase(kMeasured * kFrames);
                AmReal64 readPosition = 0.0;
                for (AmUInt64 block = kBlocks - kMeasured; block < kBlocks; ++block)
                {
                    const auto start = kGlideFrom + kDelta * static_cast<AmReal64>(block);
                    for (AmUInt64 j = 0; j < kFrames; ++j)
                    {
                        readPosition += start + kDelta * static_cast<AmReal64>(j) / static_cast<AmReal64>(kFrames);
                        phase[(block - (kBlocks - kMeasured)) * kFrames + j] = kTwoPi * kSourceHz * readPosition / kRate;
                    }
                }

                AmReal64 carrierDbfs = 0.0;
                const AmReal64 sideband = Sidebands(window, phase, kRate, 3, carrierDbfs);

                // Sidebands only mean something once the carrier is coherent: a render that stalled demodulates to
                // nothing and would report none.
                AM_EXPECT(carrierDbfs > kMinCarrierDbfs);
                AM_EXPECT(sideband <= kMaxSidebandDbc);
            }
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, resampler_ratio_ramp_joins_smoothly);
} // namespace SparkyStudios::Audio::Amplitude::Tests
