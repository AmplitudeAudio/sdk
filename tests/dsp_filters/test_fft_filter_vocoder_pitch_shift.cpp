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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <DSP/Filters/FFTFilter.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace
{
    // Routes the spectrum through the phase-vocoder analysis/synthesis helpers,
    // multiplying every true frequency by a constant shift factor. Unlike a
    // no-modification round trip (which is identity for ANY vocoder constants,
    // because analysis subtracts `expect` and synthesis adds it back), this
    // exercises the intermediate true-frequency values, so it fails when the
    // hop/window constants in the vocoder helpers are wrong.
    //
    // The expected values below are grounded in the canonical Bernsee phase
    // vocoder (smbPitchShift.cpp: expct = 2*pi*H/N, deviation scale N/(2*pi*H),
    // true frequency = bin center + deviation * freqPerBin) and were calibrated
    // with a faithful Python simulation of this engine (window 256, hop 128,
    // triangular COLA window); the simulation reproduces the C++ measurements
    // exactly for both the correct and the pre-df30ca0 constants.
    class VocoderPitchShiftInstance : public FFTFilterInstance
    {
    public:
        explicit VocoderPitchShiftInstance(FFTFilter* parent)
            : FFTFilterInstance(parent)
        {}

        // Per analysis hop: true frequency of the dominant bin before scaling,
        // and the same value after the shift factor was applied.
        std::vector<AmReal32> DominantTrueFrequency;
        std::vector<AmReal32> ShiftedTrueFrequency;

        // Per analysis hop: true frequency of a non-dominant bin whose phase
        // deviation is negative (bin 3 for the 440 Hz input), before scaling.
        // This bin is sensitive to the phase-wrap mode in MagPhase2MagFreq:
        // the simulation measures 439.86 Hz with the reference truncation
        // wrap, and 814.86 Hz (+2 bins) with a floor-based wrap.
        std::vector<AmReal32> NegativeDeviationBinFrequency;

    protected:
        void ProcessFFTChannel(SplitComplex& fft, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate) override
        {
            Comp2MagPhase(fft, static_cast<AmUInt32>(frames));
            MagPhase2MagFreq(fft, static_cast<AmUInt32>(frames), sampleRate, channel);

            // After Comp2MagPhase, re[] holds bin magnitudes and im[] the true
            // frequencies computed by MagPhase2MagFreq.
            AmUInt32 dominantBin = 0;
            AmReal32 dominantMag = -1.0f;
            for (AmUInt32 s = 0; s < static_cast<AmUInt32>(frames); ++s)
            {
                if (fft.re()[s] > dominantMag)
                {
                    dominantMag = fft.re()[s];
                    dominantBin = s;
                }
            }
            DominantTrueFrequency.push_back(fft.im()[dominantBin]);
            NegativeDeviationBinFrequency.push_back(fft.im()[kNegativeDeviationBin]);

            for (AmUInt32 s = 0; s < static_cast<AmUInt32>(frames); ++s)
                fft.im()[s] *= kShiftFactor;

            ShiftedTrueFrequency.push_back(fft.im()[dominantBin]);

            MagFreq2MagPhase(fft, static_cast<AmUInt32>(frames), sampleRate, channel);
            MagPhase2Comp(fft, static_cast<AmUInt32>(frames));
        }

    private:
        static constexpr AmReal32 kShiftFactor = 1.5f;
        static constexpr AmUInt32 kNegativeDeviationBin = 3;
    };

    // Estimates the dominant frequency of a mono buffer segment with an FFT
    // peak search. Zero-crossing counting is unreliable on phase-vocoder
    // output (period jitter creates spurious crossings), so the segment is
    // Hann-windowed and transformed; the strongest bin wins.
    AmReal32 EstimateDominantFrequency(const AudioBuffer& buffer, AmUInt64 start, AmUInt64 windowSize, AmUInt32 sampleRate)
    {
        FFT fft;
        fft.Initialize(windowSize);

        AudioBuffer window(windowSize, 1);
        for (AmUInt64 i = 0; i < windowSize; ++i)
        {
            const AmReal32 hann = 0.5f - 0.5f * std::cos(2.0f * AM_PI32 * static_cast<AmReal32>(i) / static_cast<AmReal32>(windowSize));
            window[0][i] = buffer[0][start + i] * hann;
        }

        SplitComplex spectrum;
        fft.Forward(window[0].begin(), spectrum);

        AmSize peakBin = 0;
        AmReal32 peakMag = 0.0f;
        for (AmSize s = 1; s < spectrum.GetSize(); ++s)
        {
            const AmReal32 mag = std::sqrt(spectrum.re()[s] * spectrum.re()[s] + spectrum.im()[s] * spectrum.im()[s]);
            if (mag > peakMag)
            {
                peakMag = mag;
                peakBin = s;
            }
        }

        return static_cast<AmReal32>(peakBin) * static_cast<AmReal32>(sampleRate) / static_cast<AmReal32>(windowSize);
    }
} // namespace

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_filters, fft_filter_vocoder_pitch_shift)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 4096;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;
            constexpr AmReal32 inputFrequency = 440.0f;
            constexpr AmReal32 shiftedFrequency = inputFrequency * 1.5f; // 660 Hz
            constexpr AmUInt64 analysisWindow = 2048;

            FFTFilter filter("VocoderPitchShift");
            VocoderPitchShiftInstance instance(&filter);
            FilterInstance& processor = instance;

            AudioBuffer input(frameCount, channelCount);
            AudioBuffer output(frameCount, channelCount);

            GenerateSineWaveAtFrequency(input, sampleRate, inputFrequency, 0.5f);

            processor.Process(input, output, frameCount, sampleRate);

            // Sanity: the estimator itself resolves the 440 Hz input closely
            // (bin resolution at this window size is ~23.4 Hz).
            const AmReal32 measuredInputFrequency = EstimateDominantFrequency(input, 512, analysisWindow, sampleRate);
            AM_EXPECT(std::abs(measuredInputFrequency - inputFrequency) <= 25.0f);

            // Helper contract, analysis side: for the bin carrying the sine's
            // energy, MagPhase2MagFreq must report the partial's true frequency
            // (Bernsee: bin center + wrapped phase deviation scaled by
            // N/(2*pi*H) bins). The simulation measures 440.1 Hz with a +/-9 Hz
            // hop-to-hop spread; allow +/-25 Hz for float32 noise. The
            // pre-df30ca0 constants report ~9077 Hz here (RED).
            // Skip the first hops so only steady-state frames past the
            // 256-sample priming latency are considered.
            constexpr AmSize firstSteadyHop = 4;
            AM_EXPECT(instance.DominantTrueFrequency.size() > firstSteadyHop);
            for (AmSize h = firstSteadyHop; h < instance.DominantTrueFrequency.size(); ++h)
            {
                AM_EXPECT(std::abs(instance.DominantTrueFrequency[h] - inputFrequency) <= 25.0f);

                // Synthesis side: the scaled value must be 1.5x the estimate...
                AM_EXPECT(std::abs(instance.ShiftedTrueFrequency[h] - shiftedFrequency) <= 1.5f * 25.0f);

                // ...and clearly not identity.
                AM_EXPECT(instance.ShiftedTrueFrequency[h] > inputFrequency * 1.2f);

                // Wrap-sensitive non-dominant bin: its phase deviation is
                // negative, so a floor-based wrap mis-wraps it by +2*Pi (+2
                // bins = +375 Hz). The simulation of this exact scenario
                // measures 439.86 Hz (hop range 427.86-453.57) with the
                // reference truncation wrap and 814.86 Hz with floor, so a
                // +/-25 Hz bound passes only under truncation.
                AM_EXPECT(std::abs(instance.NegativeDeviationBinFrequency[h] - 439.86f) <= 25.0f);
            }

            // Output signature. In-place per-bin frequency scaling is NOT a
            // Bernsee pitch shift (which remaps magnitudes to k*shift bins):
            // each bin keeps oscillating at its bin-center frequency within a
            // frame and only its per-hop phase advance is scaled, so the output
            // is a hop-rate sideband complex around 660 Hz rather than a clean
            // 660 Hz tone. Both the Python simulation and the C++ engine with
            // the correct constants produce a dominant peak at 281.25 Hz
            // (= 660 Hz - hop rate 375 Hz); the pre-df30ca0 constants produce
            // 468.75 Hz instead (RED), 187.5 Hz away. Pin the peak within one
            // analysis bin (23.4 Hz).
            const AmReal32 outputFrequency = EstimateDominantFrequency(output, 512, analysisWindow, sampleRate);
            AM_EXPECT(std::abs(outputFrequency - 281.25f) <= 23.44f);

            // Not identity: the output fundamental must be far from the input.
            AM_EXPECT(std::abs(outputFrequency - inputFrequency) > 100.0f);
        }
    };

    AM_REGISTER_TEST(dsp_filters, fft_filter_vocoder_pitch_shift);
} // namespace SparkyStudios::Audio::Amplitude::Tests
