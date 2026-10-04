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

#include <Fidelity/Scenarios/Timing.h>

#include <algorithm>
#include <cmath>
#include <vector>

#include <Fidelity/Analysis/Envelope.h>
#include <Fidelity/Analysis/Pitch.h>
#include <Fidelity/Scenarios/Common.h>
#include <Fidelity/Targets.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    namespace
    {
        // Source position minus output time (in output samples) over [from, to): constant while the sound plays at speed 1.
        double SourceOffset(const Signal& tau, double fs, std::uint64_t from, std::uint64_t to)
        {
            std::vector<double> values;
            for (std::uint64_t i = from; i < std::min<std::uint64_t>(to, tau.size()); ++i)
                values.push_back(tau[i] * fs - static_cast<double>(i));

            return MedianFinite(std::move(values));
        }

        double TailPeakDbfs(const Signal& x, double from)
        {
            const auto begin = static_cast<std::size_t>(std::max(0.0, from));
            if (begin >= x.size())
                return 0.0;

            return DbFromAmplitude(Peak(std::span<const double>(x.data() + begin, x.size() - begin)));
        }
    } // namespace

    void MeasureStopTiming(
        const Capture& capture, const StimulusSpec& spec, std::uint64_t playAt, std::uint64_t stopAt, double fadeMs, Measurement& out)
    {
        const double fs = capture.sampleRate;
        const double ratio = fs / spec.sampleRate;
        const Signal x = ChannelSignal(capture, 0);
        const Signal envelope = TimingEnvelope(x);

        const double plateau = PlateauLevel(envelope, playAt + Seconds(spec.fadeSeconds + 0.1, fs), stopAt - Seconds(0.02, fs));
        if (plateau <= 0.0)
        {
            out.error = "no plateau before the stop";
            return;
        }

        // The asset fades in on its own: its 50 % point is half its fade after the engine starts it.
        const double onset = SustainedCrossing(envelope, 0.5 * plateau, static_cast<double>(playAt), true, kTimingHold);
        if (onset >= 0.0)
        {
            const double assetHalfFade = 0.5 * static_cast<double>(FadeFrames(spec)) * ratio;
            out.Add("start.latencyMs", (onset - assetHalfFade - static_cast<double>(playAt)) / fs * 1000.0, "ms", Better::Lower);
        }

        const double half = SustainedCrossing(envelope, 0.5 * plateau, static_cast<double>(stopAt), false, kTimingHold);
        if (half < 0.0)
        {
            out.error = "the stop was not found";
            return;
        }

        // A linear fade is symmetric about its 50 % point; a cut is its own 50 % point.
        const double fadeSamples = fadeMs / 1000.0 * fs;
        const double fadeStart = half - 0.5 * fadeSamples;
        out.Add("stop.latencyMs", (fadeStart - static_cast<double>(stopAt)) / fs * 1000.0, "ms", Better::Lower);

        const double departure = SustainedCrossing(envelope, 0.99 * plateau, static_cast<double>(stopAt), false, kTimingHold);
        const FadeTiming timing = MeasureFade(envelope, fs, plateau, 0.0, departure >= 0.0 ? departure - 1.0 : static_cast<double>(stopAt));
        if (timing.found)
            out.Add("stop.fadeMs", timing.durationMs, "ms", Better::Lower);

        if (fadeSamples > 0.0)
        {
            // The expected linear fade goes through the same smoothing as the measured envelope, then both are compared
            // down to -20 dB.
            Signal expected(envelope.size(), 0.0);
            for (std::size_t i = 0; i < expected.size(); ++i)
                expected[i] = plateau * std::clamp(1.0 - (static_cast<double>(i) - fadeStart) / fadeSamples, 0.0, 1.0);

            const Signal smoothed = MovingAverage(expected, kTimingSmoothing);
            const auto first = static_cast<std::size_t>(std::max(0.0, fadeStart - static_cast<double>(kTimingSmoothing)));
            double worst = 0.0;
            for (std::size_t i = first; i < smoothed.size() && i < envelope.size(); ++i)
            {
                if (smoothed[i] < 0.1 * plateau)
                    break;

                worst = std::max(worst, std::abs(DbFromAmplitude(envelope[i] / smoothed[i])));
            }

            out.Add("stop.fadeCurveErrorDb", worst, "dB", Better::Lower, Targets::kMaxRampErrorDb);
        }

        out.Add(
            "stop.tailDbfs", TailPeakDbfs(x, static_cast<double>(stopAt + Seconds(0.1, fs))), "dBFS", Better::Lower, Targets::kMaxTailDbfs);
    }

    bool MeasurePauseResume(
        const Capture& capture, const StimulusSpec& spec, std::uint64_t pauseAt, std::uint64_t resumeAt, Measurement& out)
    {
        const double fs = capture.sampleRate;
        const Signal x = ChannelSignal(capture, 0);
        const Signal envelope = TimingEnvelope(x);
        const double plateau = PlateauLevel(envelope, pauseAt - Seconds(0.5, fs), pauseAt - Seconds(0.02, fs));
        const Signal tau = EstimateChirpSourceTime(x, fs, ChirpModelOf(spec), 0.9 * plateau);

        const double before = SourceOffset(tau, fs, pauseAt - Seconds(0.3, fs), pauseAt - Seconds(0.05, fs));
        const double after = SourceOffset(tau, fs, resumeAt + Seconds(0.1, fs), resumeAt + Seconds(0.4, fs));
        const double stopEdge = FadeZero(envelope, plateau, static_cast<double>(pauseAt), false);
        const double startEdge = FadeZero(envelope, plateau, static_cast<double>(resumeAt), true);

        if (plateau <= 0.0 || !std::isfinite(before) || !std::isfinite(after) || stopEdge < 0.0 || startEdge < 0.0)
        {
            out.error = "cannot locate the pause and resume edges";
            return false;
        }

        out.Add(
            "resume.positionErrorSamples", std::abs((after + startEdge) - (before + stopEdge)), "samples", Better::Lower,
            Targets::kMaxResumePositionErrorSamples);

        const FadeTiming pauseFade = MeasureFade(envelope, fs, plateau, 0.0, static_cast<double>(pauseAt));
        const FadeTiming resumeFade = MeasureFade(envelope, fs, 0.0, plateau, static_cast<double>(resumeAt));
        if (pauseFade.found)
            out.Add("pause.fadeMs", pauseFade.durationMs, "ms", Better::Lower);
        if (resumeFade.found)
            out.Add("resume.fadeMs", resumeFade.durationMs, "ms", Better::Lower);

        return true;
    }

    bool MeasureSeek(const Capture& capture, const StimulusSpec& spec, std::uint64_t seekAt, double seekSeconds, Measurement& out)
    {
        const double fs = capture.sampleRate;
        const Signal x = ChannelSignal(capture, 0);
        const double plateau = PlateauLevel(TimingEnvelope(x), seekAt - Seconds(0.5, fs), seekAt - Seconds(0.02, fs));
        const Signal tau = EstimateChirpSourceTime(x, fs, ChirpModelOf(spec), 0.9 * plateau);

        const double after = SourceOffset(tau, fs, seekAt + Seconds(0.2, fs), seekAt + Seconds(0.6, fs));
        if (plateau <= 0.0 || !std::isfinite(after))
        {
            out.error = "cannot track the chirp after the seek";
            return false;
        }

        // The output sample where the source was at the seek target.
        const double effect = seekSeconds * fs - after;
        out.Add("seek.latencyMs", (effect - static_cast<double>(seekAt)) / fs * 1000.0, "ms", Better::Lower);
        return true;
    }

    bool MeasureEndOfSound(
        const Capture& capture, const StimulusSpec& spec, std::uint64_t playAt, std::uint64_t outFrames, Measurement& out)
    {
        const double fs = capture.sampleRate;
        const double ratio = fs / spec.sampleRate;
        const Signal x = ChannelSignal(capture, 0);
        const Signal envelope = TimingEnvelope(x);
        const double plateau = PlateauLevel(envelope, playAt + outFrames / 4, playAt + 3 * outFrames / 4);

        // The end is searched backward from the end of the capture, so a dip in the middle cannot end the sound early.
        const double start = SustainedCrossing(envelope, 0.5 * plateau, static_cast<double>(playAt), true, kTimingHold);
        const double end = LastFallingCrossing(envelope, 0.5 * plateau);
        if (plateau <= 0.0 || start < 0.0 || end <= start)
        {
            out.error = "cannot locate the start and end of the sound";
            return false;
        }

        const double fade = static_cast<double>(FadeFrames(spec)) * ratio;
        AddIntegrityMetrics(
            out, capture, static_cast<std::uint64_t>(start + fade), static_cast<std::uint64_t>(std::max(start, end - fade)));

        const double expected = static_cast<double>(StimulusFrameCount(spec) - 1 - FadeFrames(spec)) * ratio;
        out.Add("length.errorSamples", std::abs((end - start) - expected), "samples", Better::Lower, Targets::kMaxLengthErrorSamples);
        out.Add("tail.peakDbfs", TailPeakDbfs(x, end + fade + 256.0), "dBFS", Better::Lower, Targets::kMaxTailDbfs);
        return true;
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
