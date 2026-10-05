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

#include <Fidelity/Stimuli.h>

#include <cmath>
#include <numbers>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    namespace
    {
        constexpr double kHalfScale = 0.5; // -6.02 dBFS peak
        constexpr double kToneFadeSeconds = 0.02;
        constexpr double kSweepFadeSeconds = 0.01;

        StimulusSpec Sine(double frequency, std::uint32_t rate, std::uint16_t channels = 1)
        {
            StimulusSpec spec;
            spec.name =
                "sine_" + std::to_string(static_cast<int>(frequency)) + "_" + std::to_string(rate) + (channels == 2 ? "_stereo" : "");
            spec.kind = StimulusKind::Sine;
            spec.sampleRate = rate;
            spec.channels = channels;
            spec.durationSeconds = 2.0;
            spec.amplitude = kHalfScale;
            spec.frequencyHz = frequency;
            spec.fadeSeconds = kToneFadeSeconds;
            return spec;
        }

        StimulusSpec Sweep(std::uint32_t rate)
        {
            StimulusSpec spec;
            spec.name = "sweep_" + std::to_string(rate);
            spec.kind = StimulusKind::LogSweep;
            spec.sampleRate = rate;
            spec.durationSeconds = 4.0;
            spec.amplitude = kHalfScale;
            spec.frequencyHz = 10.0;
            spec.frequencyEndHz = 0.95 * static_cast<double>(rate) / 2.0;
            spec.fadeSeconds = kSweepFadeSeconds;
            return spec;
        }

        StimulusSpec Pink(std::uint32_t rate)
        {
            StimulusSpec spec;
            spec.name = "pink_" + std::to_string(rate);
            spec.kind = StimulusKind::PinkNoise;
            spec.sampleRate = rate;
            spec.durationSeconds = 2.0;
            spec.amplitude = 0.125; // -18 dBFS RMS keeps the peaks under full scale
            spec.fadeSeconds = kToneFadeSeconds;
            spec.seed = 1;
            return spec;
        }

        StimulusSpec Loop(std::uint32_t rate, double frequency)
        {
            StimulusSpec spec;
            spec.name = "loop_sine_" + std::to_string(rate);
            spec.kind = StimulusKind::LoopSine;
            spec.sampleRate = rate;
            spec.durationSeconds = 1.0;
            spec.amplitude = kHalfScale;
            spec.frequencyHz = frequency;
            return spec;
        }

        StimulusSpec Chirp(std::uint32_t rate)
        {
            StimulusSpec spec;
            spec.name = "chirp_" + std::to_string(rate);
            spec.kind = StimulusKind::LinearChirp;
            spec.sampleRate = rate;
            spec.durationSeconds = 4.0;
            spec.amplitude = kHalfScale;
            spec.frequencyHz = 200.0;
            spec.frequencyEndHz = 2000.0;
            spec.fadeSeconds = kSweepFadeSeconds;
            return spec;
        }

        StimulusSpec Pitched(StimulusSpec spec, double pitch)
        {
            // spec.pitch divides a frame count.
            if (!(pitch > 0.0))
                pitch = 1.0;

            spec.name += "_pitch" + std::to_string(static_cast<int>(pitch));
            spec.pitch = pitch;
            return spec;
        }

        std::vector<StimulusSpec> BuildCatalog()
        {
            std::vector<StimulusSpec> catalog;
            for (const std::uint32_t rate : { 22050u, 44100u, 48000u, 96000u })
                catalog.push_back(Sine(997.0, rate));

            catalog.push_back(Sine(997.0, 44100, 2));
            catalog.push_back(Sine(997.0, 48000, 2));

            for (const double frequency : { 100.0, 5000.0, 15000.0, 19000.0 })
                for (const std::uint32_t rate : { 44100u, 48000u })
                    catalog.push_back(Sine(frequency, rate));

            catalog.push_back(Sine(10000.0, 22050)); // images at 22050 - 10000 Hz when upsampled
            catalog.push_back(Sine(30000.0, 96000)); // above a 48 kHz output's Nyquist: folds back if not filtered
            catalog.push_back(Sine(5000.0, 16000)); // a common voice and dialogue rate
            catalog.push_back(Pitched(Sine(8000.0, 48000), 2.0)); // 16 kHz heard at 48 kHz -> 48 kHz: the identity-ratio path, no stretch
            catalog.push_back(Pitched(Sine(15000.0, 48000), 2.0)); // 30 kHz heard: must be filtered, not folded back

            for (const std::uint32_t rate : { 16000u, 22050u, 44100u, 48000u, 96000u })
                catalog.push_back(Sweep(rate));

            catalog.push_back(Pink(44100));
            catalog.push_back(Pink(48000));
            catalog.push_back(Loop(44100, 1050.0)); // 42 samples per cycle
            catalog.push_back(Loop(48000, 1000.0)); // 48 samples per cycle
            catalog.push_back(Chirp(44100));
            return catalog;
        }

        Signal PinkNoise(const StimulusSpec& spec, std::size_t n)
        {
            // Paul Kellet's refined pink filter on seeded white noise.
            Random random(spec.seed);
            double b0 = 0.0, b1 = 0.0, b2 = 0.0, b3 = 0.0, b4 = 0.0, b5 = 0.0, b6 = 0.0;
            Signal x(n);
            for (std::size_t i = 0; i < n; ++i)
            {
                const double white = random.Symmetric();
                b0 = 0.99886 * b0 + white * 0.0555179;
                b1 = 0.99332 * b1 + white * 0.0750759;
                b2 = 0.96900 * b2 + white * 0.1538520;
                b3 = 0.86650 * b3 + white * 0.3104856;
                b4 = 0.55000 * b4 + white * 0.5329522;
                b5 = -0.7616 * b5 - white * 0.0168980;
                x[i] = b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362;
                b6 = white * 0.115926;
            }

            double mean = 0.0;
            for (const double v : x)
                mean += v;
            mean /= static_cast<double>(n);

            for (double& v : x)
                v -= mean;

            const double scale = spec.amplitude / Rms(x);
            for (double& v : x)
                v *= scale;

            ApplyFades(x, FadeFrames(spec));
            return x;
        }
    } // namespace

    const std::vector<StimulusSpec>& StimulusCatalog()
    {
        static const std::vector<StimulusSpec> catalog = BuildCatalog();
        return catalog;
    }

    const StimulusSpec* FindStimulus(std::string_view name)
    {
        for (const StimulusSpec& spec : StimulusCatalog())
            if (spec.name == name)
                return &spec;

        return nullptr;
    }

    std::size_t StimulusFrameCount(const StimulusSpec& spec)
    {
        return static_cast<std::size_t>(std::llround(spec.durationSeconds * spec.sampleRate));
    }

    std::size_t FadeFrames(const StimulusSpec& spec)
    {
        return static_cast<std::size_t>(std::llround(spec.fadeSeconds * spec.sampleRate));
    }

    SweepModel SweepModelOf(const StimulusSpec& spec)
    {
        return SweepModel{ spec.frequencyHz, spec.frequencyEndHz, spec.durationSeconds, spec.amplitude, spec.fadeSeconds };
    }

    ChirpModel ChirpModelOf(const StimulusSpec& spec)
    {
        return ChirpModel{ spec.frequencyHz, (spec.frequencyEndHz - spec.frequencyHz) / spec.durationSeconds, spec.amplitude };
    }

    Signal RenderStimulusChannel(const StimulusSpec& spec)
    {
        const std::size_t n = StimulusFrameCount(spec);
        const double rate = spec.sampleRate;

        switch (spec.kind)
        {
        case StimulusKind::Sine:
            {
                Signal x = MakeSine(n, rate, spec.frequencyHz, spec.amplitude);
                ApplyFades(x, FadeFrames(spec));
                return x;
            }
        case StimulusKind::LoopSine:
            return MakeSine(n, rate, spec.frequencyHz, spec.amplitude);
        case StimulusKind::LogSweep:
            return RenderLogSweep(SweepModelOf(spec), rate, spec.frequencyEndHz);
        case StimulusKind::LinearChirp:
            {
                const ChirpModel model = ChirpModelOf(spec);
                Signal x(n);
                for (std::size_t i = 0; i < n; ++i)
                {
                    const double tau = static_cast<double>(i) / rate;
                    x[i] = spec.amplitude * std::sin(2.0 * std::numbers::pi * std::fmod(ChirpCycles(model, tau), 1.0));
                }

                ApplyFades(x, FadeFrames(spec));
                return x;
            }
        case StimulusKind::PinkNoise:
            return PinkNoise(spec, n);
        }

        return {};
    }

    std::vector<float> RenderStimulusInterleaved(const StimulusSpec& spec)
    {
        const Signal channel = RenderStimulusChannel(spec);
        std::vector<float> interleaved(channel.size() * spec.channels);
        for (std::size_t i = 0; i < channel.size(); ++i)
            for (std::size_t c = 0; c < spec.channels; ++c)
                interleaved[i * spec.channels + c] = static_cast<float>(channel[i]);

        return interleaved;
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
