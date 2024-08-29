// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#include <DSP/Filters/BiquadResonantFilter.h>
#include <DSP/Gain.h>
#include <DSP/NearFieldProcessor.h>

namespace SparkyStudios::Audio::Amplitude
{
    // Cross-over frequency of the band-splitting filter.
    constexpr AmReal32 kCrossOverFrequencyHz = 1000.0f;

    // +6dB bass boost factor converted to linear scale.
    constexpr AmReal32 kBassBoost = 2.0f;

    // Average group delay of the HRTF filters in seconds. Please see
    constexpr AmReal32 kMeanHrtfGroupDelaySeconds = 0.00066667f;

    // Average group delay of the shelf-filter in samples.
    constexpr AmSize kMeanShelfFilterGroupDelaySamples = 1;

    NearFieldProcessor::NearFieldProcessor(AmUInt32 sampleRate, AmUInt32 framesCount)
        : _sampleRate(sampleRate)
        , _framesCount(framesCount)
        , _delayCompensation(static_cast<AmSize>(kMeanHrtfGroupDelaySeconds * sampleRate) - kMeanShelfFilterGroupDelaySamples)
        , _lowPassFilter(nullptr)
        , _highPassFilter(nullptr)
        , _lowPassBuffer(framesCount, 1)
        , _delay(_delayCompensation, framesCount)
    {
        AMPLITUDE_ASSERT(sampleRate > 0 && framesCount > 0);
        AMPLITUDE_ASSERT(kCrossOverFrequencyHz < 0.5f * sampleRate);

        _lowPassFilter = Filter::Construct("BiquadResonant");
        _highPassFilter = Filter::Construct("BiquadResonant");

        _lowPassFilter->SetParameter(BiquadResonantFilter::ATTRIBUTE_TYPE, BiquadResonantFilter::TYPE_DUAL_BAND_LOW_PASS);
        _lowPassFilter->SetParameter(BiquadResonantFilter::ATTRIBUTE_FREQUENCY, kCrossOverFrequencyHz);

        _highPassFilter->SetParameter(BiquadResonantFilter::ATTRIBUTE_TYPE, BiquadResonantFilter::TYPE_DUAL_BAND_HIGH_PASS);
        _highPassFilter->SetParameter(BiquadResonantFilter::ATTRIBUTE_FREQUENCY, kCrossOverFrequencyHz);
    }

    void NearFieldProcessor::Process(const AudioBufferChannel& in, AudioBufferChannel& out, bool isHrtfEnabled)
    {
        // Only mono input and output channels are supported.
        AMPLITUDE_ASSERT(in.size() == _framesCount);
        AMPLITUDE_ASSERT(out.size() == _framesCount);

        {
            AudioBuffer tempIn(in.size(), 1);
            tempIn[0] = in;

            AudioBuffer tempOut(in.size(), 1);
            tempOut[0] = out;

            // Apply low-pass filter to the input signal.
            _lowPassFilter->Process(tempIn, _lowPassBuffer, _framesCount, _sampleRate);

            // Apply high-pass filter to the output signal.
            _highPassFilter->Process(tempIn, tempOut, _framesCount, _sampleRate);

            out = tempOut[0];
        }

        // Apply bass boost to the output signal.
        Gain::ApplyAccumulateConstantGain(-kBassBoost, _lowPassBuffer[0], 0, out, 0, _framesCount);

        // Apply delay compensation to the output signal.
        if (isHrtfEnabled)
        {
            _delay.Insert(out);
            _delay.Process(out, _delayCompensation);
        }
    }
} // namespace SparkyStudios::Audio::Amplitude
