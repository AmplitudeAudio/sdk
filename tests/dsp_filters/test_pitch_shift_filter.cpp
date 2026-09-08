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

#include <DSP/Filters/PitchShiftFilter.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace
{
    // Estimates the dominant frequency of a mono buffer segment with an FFT
    // peak search over the [minFrequency, maxFrequency] band.
    AmReal32 EstimateDominantFrequencyInBand(
        const AudioBuffer& buffer, AmUInt64 start, AmUInt64 windowSize, AmUInt32 sampleRate, AmReal32 minFrequency, AmReal32 maxFrequency)
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

        const AmReal32 binWidth = static_cast<AmReal32>(sampleRate) / static_cast<AmReal32>(windowSize);
        const AmSize firstBin = std::max<AmSize>(1, static_cast<AmSize>(minFrequency / binWidth));
        const AmSize lastBin = std::min<AmSize>(spectrum.GetSize() - 1, static_cast<AmSize>(maxFrequency / binWidth));

        AmSize peakBin = firstBin;
        AmReal32 peakMag = 0.0f;
        for (AmSize s = firstBin; s <= lastBin; ++s)
        {
            const AmReal32 mag = std::sqrt(spectrum.re()[s] * spectrum.re()[s] + spectrum.im()[s] * spectrum.im()[s]);
            if (mag > peakMag)
            {
                peakMag = mag;
                peakBin = s;
            }
        }

        return static_cast<AmReal32>(peakBin) * binWidth;
    }
} // namespace

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_filters, pitch_shift_filter_shifts_sine_by_semitones)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 4096;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;
            constexpr AmReal32 inputFrequency = 440.0f;
            constexpr AmReal32 shiftSemitones = 7.0f;
            constexpr AmReal32 shiftRatio = 1.4983071f; // 2^(7/12)
            constexpr AmReal32 shiftedFrequency = inputFrequency * shiftRatio; // 659.26 Hz
            constexpr AmUInt64 analysisWindow = 2048;
            constexpr AmReal32 binResolution = 23.44f; // sampleRate / analysisWindow

            auto filter = amshared(PitchShiftFilter);

            // Out-of-range shifts are rejected.
            AM_EXPECT(filter->Initialize(-24.5f) == eErrorCode_InvalidParameter);
            AM_EXPECT(filter->Initialize(24.5f) == eErrorCode_InvalidParameter);
            AM_EXPECT(filter->Initialize(shiftSemitones) == eErrorCode_Success);

            // Parameter metadata.
            AM_EXPECT(filter->GetParameterCount() == PitchShiftFilter::ATTRIBUTE_LAST);
            AM_EXPECT(filter->GetParameterCount() == 2);

            AM_EXPECT(filter->GetParameterName(PitchShiftFilter::ATTRIBUTE_WET) == "Wet");
            AM_EXPECT(filter->GetParameterName(PitchShiftFilter::ATTRIBUTE_SHIFT) == "Pitch Shift");

            AM_EXPECT(filter->GetParameterType(PitchShiftFilter::ATTRIBUTE_WET) == eParameterType_Float);
            AM_EXPECT(filter->GetParameterType(PitchShiftFilter::ATTRIBUTE_SHIFT) == eParameterType_Float);

            AM_EXPECT(filter->GetParameterMin(PitchShiftFilter::ATTRIBUTE_WET) == 0.0f);
            AM_EXPECT(filter->GetParameterMax(PitchShiftFilter::ATTRIBUTE_WET) == 1.0f);
            AM_EXPECT(filter->GetParameterMin(PitchShiftFilter::ATTRIBUTE_SHIFT) == -24.0f);
            AM_EXPECT(filter->GetParameterMax(PitchShiftFilter::ATTRIBUTE_SHIFT) == 24.0f);

            auto instance = filter->CreateInstance();
            AM_EXPECT_NOT(instance == nullptr);

            // Parameter plumbing: the instance must start from the shift given
            // to the parent filter.
            AM_EXPECT(instance->GetParameter(PitchShiftFilter::ATTRIBUTE_SHIFT) == shiftSemitones);

            AudioBuffer input(frameCount, channelCount);
            AudioBuffer output(frameCount, channelCount);

            GenerateSineWaveAtFrequency(input, sampleRate, inputFrequency, 0.5f);

            instance->Process(input, output, frameCount, sampleRate);

            // Output signature, past the 256-sample priming latency. The
            // dominant peak of the sideband complex sits at the shifted target
            // minus the 375 Hz hop rate: bin 12 = 281.25 Hz in the simulation
            // (magnitude 170.6, clearly dominant). Pin within one analysis bin.
            const AmReal32 outputPeak = EstimateDominantFrequencyInBand(output, 512, analysisWindow, sampleRate, 0.0f, 24000.0f);
            AM_EXPECT(std::abs(outputPeak - 281.25f) <= binResolution);

            // The shifted partial itself: the simulation's strongest peak in
            // the 500-900 Hz band is bin 28 = 656.25 Hz (magnitude 88.6).
            // Asserting it lands within 25 Hz of the 659.26 Hz target proves
            // the 7-semitone parameter drove the shift (identity leaves no
            // energy in the band; 5 semitones would peak near 587 Hz, 12
            // semitones near 880 Hz).
            const AmReal32 shiftedPartial = EstimateDominantFrequencyInBand(output, 512, analysisWindow, sampleRate, 500.0f, 900.0f);
            AM_EXPECT(std::abs(shiftedPartial - shiftedFrequency) <= 25.0f);

            // Clearly not identity.
            AM_EXPECT(std::abs(outputPeak - inputFrequency) > 100.0f);
        }
    };

    AM_REGISTER_TEST(dsp_filters, pitch_shift_filter_shifts_sine_by_semitones);

    AM_TEST_CASE(DSPTestCase, dsp_filters, pitch_shift_filter_with_zero_shift_is_identity)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 4096;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            auto filter = amshared(PitchShiftFilter);
            AM_EXPECT(filter->Initialize(0.0f) == eErrorCode_Success);

            auto instance = filter->CreateInstance();
            AM_EXPECT_NOT(instance == nullptr);

            AudioBuffer input(frameCount, channelCount);
            AudioBuffer output(frameCount, channelCount);

            // Off-bin-center frequencies stress the true-frequency estimation.
            GenerateMultiTone(input, sampleRate, { 100.0f, 1000.0f, 4400.0f, 12000.0f }, 0.2f);

            instance->Process(input, output, frameCount, sampleRate);

            // A zero shift is a pure analysis/synthesis round trip: past the
            // 256-sample latency, reconstruction must be near-perfect (the
            // simulation measures a 1.3e-13 max error).
            for (AmUInt64 i = 256; i < frameCount; ++i)
                AM_EXPECT(std::abs(output[0][i] - input[0][i - 256]) < 0.02f);
        }
    };

    AM_REGISTER_TEST(dsp_filters, pitch_shift_filter_with_zero_shift_is_identity);
} // namespace SparkyStudios::Audio::Amplitude::Tests
