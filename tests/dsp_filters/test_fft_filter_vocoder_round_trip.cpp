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
    // Routes the spectrum through the phase-vocoder analysis/synthesis helpers
    // without modification: reconstruction must be near-identity.
    class VocoderRoundTripInstance : public FFTFilterInstance
    {
    public:
        explicit VocoderRoundTripInstance(FFTFilter* parent)
            : FFTFilterInstance(parent)
        {}

    protected:
        void ProcessFFTChannel(SplitComplex& fft, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate) override
        {
            Comp2MagPhase(fft, static_cast<AmUInt32>(frames));
            MagPhase2MagFreq(fft, static_cast<AmUInt32>(frames), sampleRate, channel);
            MagFreq2MagPhase(fft, static_cast<AmUInt32>(frames), sampleRate, channel);
            MagPhase2Comp(fft, static_cast<AmUInt32>(frames));
        }
    };
} // namespace

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_filters, fft_filter_vocoder_round_trip)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 4096;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            FFTFilter filter("VocoderRoundTrip");
            VocoderRoundTripInstance instance(&filter);
            FilterInstance& processor = instance;

            AudioBuffer input(frameCount, channelCount);
            AudioBuffer output(frameCount, channelCount);

            // Off-bin-center frequencies stress the true-frequency estimation.
            GenerateMultiTone(input, sampleRate, { 100.0f, 1000.0f, 4400.0f, 12000.0f }, 0.2f);

            processor.Process(input, output, frameCount, sampleRate);

            // Past the 256-sample latency, reconstruction must be near-perfect.
            for (AmUInt64 i = 256; i < frameCount; ++i)
                AM_EXPECT(std::abs(output[0][i] - input[0][i - 256]) < 0.02f);
        }
    };

    AM_REGISTER_TEST(dsp_filters, fft_filter_vocoder_round_trip);
} // namespace SparkyStudios::Audio::Amplitude::Tests
