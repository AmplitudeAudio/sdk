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

#include <Fidelity/Scenarios/Common.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <DSP/Gain.h>

#include <Fidelity/Analysis/Click.h>
#include <Fidelity/Analysis/Envelope.h>
#include <Fidelity/Analysis/Integrity.h>
#include <Fidelity/Analysis/Spectrum.h>
#include <Fidelity/AssetGenerator.h>
#include <Fidelity/Targets.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    std::uint64_t Seconds(double seconds, double rate)
    {
        return static_cast<std::uint64_t>(std::llround(seconds * rate));
    }

    std::uint64_t OnsetFrame(const Capture& capture, std::uint64_t from)
    {
        std::uint64_t onset = std::numeric_limits<std::uint64_t>::max();
        for (const auto& channel : capture.channels)
            for (std::uint64_t i = from; i < channel.size() && i < onset; ++i)
                if (channel[i] != 0.0f)
                    onset = i;

        return onset == std::numeric_limits<std::uint64_t>::max() ? from : onset;
    }

    std::uint64_t OutputFrames(const StimulusSpec& spec, std::uint32_t outputRate)
    {
        return static_cast<std::uint64_t>(
            std::ceil(static_cast<double>(StimulusFrameCount(spec)) * outputRate / (static_cast<double>(spec.sampleRate) * spec.pitch)));
    }

    double CenterPanGain()
    {
        return 1.0 / std::sqrt(2.0);
    }

    double OutputChannelGain(const StimulusSpec& spec)
    {
        return spec.channels >= 2 ? 1.0 : CenterPanGain();
    }

    RenderSettings IsolatedSettings(const GridPoint& point, std::uint64_t durationSamples)
    {
        RenderSettings settings;
        settings.configFile = ConfigName("isolated", point.blockSize, point.outputRate, ".config.amconfig");
        settings.blockSize = point.blockSize;
        settings.fps = point.fps;
        settings.jitter = point.jitter;
        settings.durationSamples = durationSamples;
        return settings;
    }

    ActionFactory PlayOnly(const std::string& soundName, std::shared_ptr<bool> played)
    {
        return [soundName, played]()
        {
            return std::vector<TimedAction>{
                { kLeadIn, "play",
                  [soundName, played]()
                  {
                      const Channel channel = amEngine->Play(amEngine->GetSoundHandle(soundName));
                      if (played != nullptr)
                          *played = channel.Valid();
                  } },
            };
        };
    }

    bool RequireSignal(Measurement& out, const Capture& capture, std::uint64_t begin, std::uint64_t end)
    {
        double peak = 0.0;
        if (!capture.channels.empty())
        {
            const std::vector<float>& channel = capture.channels[0];
            for (std::uint64_t i = begin; i < std::min<std::uint64_t>(end, channel.size()); ++i)
                peak = std::max(peak, static_cast<double>(std::abs(channel[i])));
        }

        if (peak >= AmplitudeFromDb(-60.0))
            return true;

        out.error = "no signal between samples " + std::to_string(begin) + " and " + std::to_string(end) + ": the sound did not play";
        return false;
    }

    void AddIntegrityMetrics(Measurement& out, const Capture& capture, std::uint64_t signalBegin, std::uint64_t signalEnd)
    {
        std::size_t nan = 0, inf = 0, denormals = 0, clipped = 0, dropouts = 0;
        for (const auto& channel : capture.channels)
        {
            IntegrityOptions options;
            options.signalBegin = static_cast<std::size_t>(signalBegin);
            options.signalEnd = static_cast<std::size_t>(signalEnd);
            const IntegrityResult r = AnalyzeIntegrity(channel, options);
            nan += r.nanCount;
            inf += r.infCount;
            denormals += r.denormalCount;
            clipped += r.clippedCount;
            dropouts += r.dropoutCount;
        }

        out.Add("integrity.nan", static_cast<double>(nan), "count", Better::Lower, Targets::kMaxNanCount);
        out.Add("integrity.inf", static_cast<double>(inf), "count", Better::Lower, Targets::kMaxInfCount);
        out.Add("integrity.denormals", static_cast<double>(denormals), "count", Better::Lower, Targets::kMaxDenormalCount);
        out.Add("integrity.clipped", static_cast<double>(clipped), "count", Better::Lower, Targets::kMaxClippedSamples);
        out.Add("integrity.dropouts", static_cast<double>(dropouts), "count", Better::Lower, Targets::kMaxDropouts);
    }

    void AddClickMetrics(Measurement& out, const Capture& capture, double highPassHz, std::uint64_t begin, std::uint64_t end)
    {
        ClickOptions options;
        options.highPassHz = highPassHz;

        std::vector<EventRecord> events;
        double worst = -400.0;
        double bandPeak = -400.0;
        for (std::size_t c = 0; c < capture.channels.size(); ++c)
        {
            const ClickResult r = AnalyzeClicks(ChannelSignal(capture, c), capture.sampleRate, options);
            bandPeak = std::max(bandPeak, r.maxPeakDbfs);
            for (const ClickEvent& e : r.events)
            {
                if (e.sample < begin || e.sample >= end)
                    continue;

                EventRecord record;
                record.analyzer = "click";
                record.channel = static_cast<std::uint32_t>(c);
                record.sample = e.sample;
                record.levelDbfs = e.peakDbfs;
                record.aboveFloorDb = e.aboveFloorDb;
                events.push_back(record);
                worst = std::max(worst, e.peakDbfs);
            }
        }

        AnnotateEvents(events, capture);
        out.Add("click.worstDbfs", worst, "dBFS", Better::Lower, Targets::kMaxClickDbfs);
        out.Add("click.events", static_cast<double>(events.size()), "count", Better::Lower);
        out.Add("click.bandPeakDbfs", bandPeak, "dBFS", Better::Lower);
        out.events.insert(out.events.end(), events.begin(), events.end());
    }

    std::vector<std::string> StimulusNames(std::initializer_list<StimulusKind> kinds)
    {
        std::vector<std::string> names;
        for (const StimulusSpec& spec : StimulusCatalog())
            if (std::find(kinds.begin(), kinds.end(), spec.kind) != kinds.end())
                names.push_back(spec.name);

        return names;
    }

    double AliasLevelDbc(std::span<const double> steady, double sampleRate, const StimulusSpec& spec, double frequencyHz)
    {
        const PowerSpectrum spectrum = AveragedPowerSpectrum(steady, sampleRate, 32768);
        return DbFromPower(BandPower(spectrum, sampleRate - frequencyHz, 8) / 0.5) - DbFromAmplitude(spec.amplitude * OutputChannelGain(spec));
    }

    double MedianFinite(std::vector<double> values)
    {
        values.erase(
            std::remove_if(
                values.begin(), values.end(),
                [](double v)
                {
                    return !std::isfinite(v);
                }),
            values.end());

        if (values.empty())
            return std::numeric_limits<double>::quiet_NaN();

        auto middle = values.begin() + static_cast<std::ptrdiff_t>(values.size() / 2);
        std::nth_element(values.begin(), middle, values.end());
        return *middle;
    }

    double FadeZero(std::span<const double> envelope, double level, double searchBegin, bool rising)
    {
        const double high = 0.8 * level;
        const double low = 0.2 * level;

        if (rising)
        {
            const double t20 = CrossingTime(envelope, low, searchBegin, true);
            if (t20 < 0.0)
                return -1.0;

            const double t80 = CrossingTime(envelope, high, t20, true);
            if (t80 <= t20)
                return -1.0;

            const double slope = (high - low) / (t80 - t20);
            return t20 - low / slope;
        }

        const double t80 = CrossingTime(envelope, high, searchBegin, false);
        if (t80 < 0.0)
            return -1.0;

        const double t20 = CrossingTime(envelope, low, t80, false);
        if (t20 <= t80)
            return -1.0;

        const double slope = (low - high) / (t20 - t80);
        return t20 - low / slope;
    }

    std::vector<double> AnalyticPanGains(double azimuth, const std::vector<double>& speakerAzimuths)
    {
        constexpr double kRadians = 3.14159265358979323846 / 180.0;
        const auto wrap360 = [](double degrees)
        {
            const double wrapped = std::fmod(degrees, 360.0);
            return wrapped < 0.0 ? wrapped + 360.0 : wrapped;
        };

        std::vector<double> gains(speakerAzimuths.size(), 0.0);
        std::vector<std::size_t> ring;
        for (std::size_t c = 0; c < speakerAzimuths.size(); ++c)
            if (!std::isnan(speakerAzimuths[c]))
                ring.push_back(c);

        std::sort(
            ring.begin(), ring.end(),
            [&](std::size_t a, std::size_t b)
            {
                return speakerAzimuths[a] < speakerAzimuths[b];
            });

        if (ring.size() == 1)
        {
            gains[ring[0]] = 1.0;
            return gains;
        }

        double a = std::remainder(azimuth, 360.0);
        const std::size_t pairs = ring.size() == 2 ? 1 : ring.size();
        if (ring.size() == 2)
        {
            if (a > 90.0)
                a = 180.0 - a;
            else if (a < -90.0)
                a = -180.0 - a;

            a = std::clamp(a, speakerAzimuths[ring[0]], speakerAzimuths[ring[1]]);
        }

        for (std::size_t j = 0; j < pairs; ++j)
        {
            const std::size_t first = ring[j];
            const std::size_t second = ring[(j + 1) % ring.size()];
            const double span = wrap360(speakerAzimuths[second] - speakerAzimuths[first]);
            const double t = wrap360(a - speakerAzimuths[first]);
            if (t > span)
                continue;

            const double g1 = std::sin((span - t) * kRadians);
            const double g2 = std::sin(t * kRadians);
            const double norm = std::hypot(g1, g2);
            gains[first] = g1 / norm;
            gains[second] = g2 / norm;
            break;
        }

        return gains;
    }

    /// Same frame as the engine tests: listener at the origin facing -Z, +Y up, azimuth positive to the left.
    AmVector3 LocationAt(double azimuth)
    {
        constexpr double kPi = 3.14159265358979323846;
        constexpr double kDistance = 10.0;
        const double radians = azimuth * kPi / 180.0;
        return { static_cast<AmReal32>(-std::sin(radians) * kDistance), 0.0f, static_cast<AmReal32>(-std::cos(radians) * kDistance) };
    }

    TimedAction SceneSetup()
    {
        return { 0, "scene", []()
                 {
                     const Listener listener = amEngine->AddListener(1);
                     listener.SetLocation({ 0.0f, 0.0f, 0.0f });
                     listener.SetOrientation(Orientation({ 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }));
                     amEngine->SetDefaultListener(1);
                 } };
    }

    /// Mean square of every channel over [begin, end).
    std::vector<double> ChannelPowers(const Capture& capture, std::uint64_t begin, std::uint64_t end)
    {
        std::vector<double> powers;
        for (std::size_t c = 0; c < capture.channels.size(); ++c)
        {
            const Signal x = ChannelSignal(capture, c);
            const double rms = Rms(std::span<const double>(x.data() + begin, end - begin));
            powers.push_back(rms * rms);
        }
        return powers;
    }

    /**
     * Integrity of a panned render. NaN, Inf, denormals and clipping are checked per channel, but a channel the pan
     * leaves silent is not a dropout: dropouts are searched on the sum of the channels' magnitudes, which is zero
     * only when the whole output is.
     */
    void AddPanIntegrityMetrics(Measurement& out, const Capture& capture, std::uint64_t begin, std::uint64_t end)
    {
        std::vector<float> total;
        for (const auto& channel : capture.channels)
        {
            if (total.empty())
                total.assign(channel.size(), 0.0f);

            for (std::size_t i = 0; i < channel.size(); ++i)
                total[i] += std::abs(channel[i]);
        }

        IntegrityOptions options;
        options.signalBegin = static_cast<std::size_t>(begin);
        options.signalEnd = static_cast<std::size_t>(end);
        const IntegrityResult sum = AnalyzeIntegrity(total, options);

        std::size_t nan = 0, inf = 0, denormals = 0, clipped = 0;
        for (const auto& channel : capture.channels)
        {
            const IntegrityResult r = AnalyzeIntegrity(channel, IntegrityOptions{});
            nan += r.nanCount;
            inf += r.infCount;
            denormals += r.denormalCount;
            clipped += r.clippedCount;
        }

        out.Add("integrity.nan", static_cast<double>(nan), "count", Better::Lower, Targets::kMaxNanCount);
        out.Add("integrity.inf", static_cast<double>(inf), "count", Better::Lower, Targets::kMaxInfCount);
        out.Add("integrity.denormals", static_cast<double>(denormals), "count", Better::Lower, Targets::kMaxDenormalCount);
        out.Add("integrity.clipped", static_cast<double>(clipped), "count", Better::Lower, Targets::kMaxClippedSamples);
        out.Add("integrity.dropouts", static_cast<double>(sum.dropoutCount), "count", Better::Lower, Targets::kMaxDropouts);
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
