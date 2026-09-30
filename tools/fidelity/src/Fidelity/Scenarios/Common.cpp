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
            std::ceil(static_cast<double>(StimulusFrameCount(spec)) * outputRate / static_cast<double>(spec.sampleRate)));
    }

    double CenterPanGain()
    {
        return SparkyStudios::Audio::Amplitude::Gain::CalculateStereoPannedGain(1.0f, 0.0f).x;
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

    double AliasLevelDbc(std::span<const double> steady, double sampleRate, const StimulusSpec& spec)
    {
        const PowerSpectrum spectrum = AveragedPowerSpectrum(steady, sampleRate, 32768);
        return DbFromPower(BandPower(spectrum, sampleRate - spec.frequencyHz, 8) / 0.5) - DbFromAmplitude(spec.amplitude * CenterPanGain());
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
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
