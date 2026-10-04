// Copyright 2018 Google Inc. All Rights Reserved.
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

#include <algorithm>
#include <cmath>
#include <numeric>

#include <SparkyStudios/Audio/Amplitude/Math/Utils.h>

#include <DSP/Resamplers/DefaultResampler.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    bool DefaultResamplerInstance::IsConversionExact(AmUInt32 sampleRateIn, AmUInt32 sampleRateOut) const
    {
        if (sampleRateIn == 0 || sampleRateOut == 0)
            return false;

        return ApproximateRational(sampleRateOut, sampleRateIn, kMaxPolyphaseRate).exact;
    }

    DefaultResamplerInstance::DefaultResamplerInstance()
        : _upRate(0)
        , _downRate(0)
        , _timeModuloUpRate(0)
        , _lastProcessedSample(0)
        , _channelCount(0)
        , _coefficientsPerPhase(0)
        , _transposedFilterCoefficients(kResamplerFilterCapacity, 1)
        , _temporaryFilterCoefficients(kResamplerFilterCapacity, 1)
        , _state(kResamplerStateFrames, kAmMaxSupportedChannelCount)
    {
        _state.Clear();
    }

    bool DefaultResamplerInstance::Process(const AudioBuffer& input, AmUInt64& inputFrames, AudioBuffer& output, AmUInt64& outputFrames)
    {
        // See "Digital Signal Processing", 4th Edition, Prolakis and Manolakis,
        // Pearson, Chapter 11 (specifically Figures 11.5.10 and 11.5.13).

        AMPLITUDE_ASSERT(input.GetChannelCount() == _channelCount);
        AMPLITUDE_ASSERT(output.GetChannelCount() == _channelCount);

        output.Clear();

        if (IsIdentity())
        {
            const AmUInt64 frames = AM_MIN(inputFrames, outputFrames);
            AudioBuffer::Copy(input, 0, output, 0, frames);
            inputFrames = outputFrames = frames;
            return true;
        }

        AmUInt64 inputSample = _lastProcessedSample;
        AmUInt64 outputSample = 0;

        const auto& filterCoeffs = _transposedFilterCoefficients[0];

        while (inputSample < inputFrames && outputSample < outputFrames)
        {
            AmUInt64 filterIndex = _timeModuloUpRate * _coefficientsPerPhase;
            AmUInt64 offsetInputIndex = inputSample - _coefficientsPerPhase + 1;
            const AmInt64 offset = -static_cast<AmInt64>(offsetInputIndex);

            if (offset > 0)
            {
                // We will need to draw data from the _state buffer.
                const AmInt64 stateFrameCount = static_cast<AmInt64>(_coefficientsPerPhase - 1);
                AmInt64 stateIndex = stateFrameCount - offset;

                while (stateIndex < stateFrameCount)
                {
                    for (AmUInt64 channel = 0; channel < _channelCount; ++channel)
                        output[channel][outputSample] += _state[channel][stateIndex] * filterCoeffs[filterIndex];

                    stateIndex++;
                    filterIndex++;
                }

                // Move along by offset samples up as far as input.
                offsetInputIndex += offset;
            }

            // We now move back to where inputSample "points".
            while (offsetInputIndex <= inputSample)
            {
                for (AmUInt64 channel = 0; channel < _channelCount; ++channel)
                    output[channel][outputSample] += input[channel][offsetInputIndex] * filterCoeffs[filterIndex];

                offsetInputIndex++;
                filterIndex++;
            }

            outputSample++;

            _timeModuloUpRate += _downRate;
            // Advance the input pointer.
            inputSample += _timeModuloUpRate / _upRate;
            // Decide which phase of the polyphase filter to use next.
            _timeModuloUpRate %= _upRate;
        }

        AMPLITUDE_ASSERT(inputSample >= inputFrames || outputSample >= outputFrames);

        // Only the frames before inputSample are consumed: inputSample itself is the newest frame of the next output.
        const AmUInt64 consumed = AM_MIN(inputSample, inputFrames);
        PushHistory(input, consumed);
        _lastProcessedSample = inputSample - consumed;

        inputFrames = consumed;
        outputFrames = outputSample;

        return true;
    }

    void DefaultResamplerInstance::PushHistory(const AudioBuffer& input, AmUInt64 frames)
    {
        const AmUInt64 historyFrames = _coefficientsPerPhase > 0 ? _coefficientsPerPhase - 1 : 0;
        if (historyFrames == 0 || frames == 0)
            return;

        for (AmUInt64 channel = 0; channel < _channelCount; ++channel)
        {
            auto& state = _state[channel];

            if (frames >= historyFrames)
            {
                std::copy_n(input[channel].begin() + (frames - historyFrames), historyFrames, state.begin());
                continue;
            }

            // Shift the kept history left, then append the new frames.
            std::copy(state.begin() + frames, state.begin() + historyFrames, state.begin());
            std::copy_n(input[channel].begin(), frames, state.begin() + (historyFrames - frames));
        }
    }

    AmUInt64 DefaultResamplerInstance::GetInputFramesNeeded(AmUInt64 outputFrameCount) const
    {
        if (outputFrameCount == 0)
            return 0;

        if (IsIdentity())
            return outputFrameCount;

        // Output j uses input frames up to _lastProcessedSample + floor((_timeModuloUpRate + j * _downRate) / _upRate).
        const AmUInt64 newest = _lastProcessedSample + (_timeModuloUpRate + (outputFrameCount - 1) * _downRate) / _upRate;
        return newest + 1;
    }

    AmUInt64 DefaultResamplerInstance::GetLatency() const
    {
        if (IsIdentity() || _upRate == 0)
            return 0;

        // The sinc is centred on filterLength / 2 taps at the up-sampled rate.
        return (_filterLength / 2 + _upRate - 1) / _upRate;
    }

    void DefaultResamplerInstance::SetRatio(AmReal64 inputPerOutput)
    {
        if (!std::isfinite(inputPerOutput) || inputPerOutput <= 0.0)
            inputPerOutput = 1.0;

        constexpr AmReal64 kScale = static_cast<AmReal64>(1ULL << 30);
        const auto out = static_cast<AmUInt64>(kScale);
        const auto in = AM_MAX(static_cast<AmUInt64>(std::llround(inputPerOutput * kScale)), 1ULL);

        ApplyRational(ApproximateRational(out, in, kMaxPolyphaseRate));
    }

    void DefaultResamplerInstance::Initialize(AmUInt16 channelCount, AmUInt32 sampleRateIn, AmUInt32 sampleRateOut)
    {
        AMPLITUDE_ASSERT(channelCount > 0);

        // A rate of zero can reach this in release builds, where the assertions above are compiled out: clamp it.
        sampleRateIn = AM_MAX(sampleRateIn, 1U);
        sampleRateOut = AM_MAX(sampleRateOut, 1U);

        // Reported unconditionally: two different rate pairs can reduce (or snap) to the same ratio, and the
        // accessors are documented to return what the caller requested.
        _sampleRateIn = sampleRateIn;
        _sampleRateOut = sampleRateOut;

        // Obtain the size of the _state before _coefficientsPerPhase is updated in ApplyRational().
        const AmUInt64 oldStateSize = _coefficientsPerPhase > 0 ? _coefficientsPerPhase - 1 : 0;

        // Reduce the rate pair, approximating it when it cannot be represented within the filter budget.
        // This keeps max(_upRate, _downRate) <= kMaxPolyphaseRate, which bounds every buffer written by
        // GenerateInterpolatingFilter() and ArrangeFilterAsPolyphase().
        ApplyRational(ApproximateRational(sampleRateOut, sampleRateIn, kMaxPolyphaseRate));

        if (_channelCount != channelCount)
        {
            _channelCount = channelCount;
            InitializeStateBuffer(oldStateSize);
        }
    }

    void DefaultResamplerInstance::ApplyRational(const AmRational& ratio)
    {
        const AmUInt64 destination = ratio.numerator;
        const AmUInt64 source = ratio.denominator;

        if (destination == _upRate && source == _downRate)
            return;

        const AmUInt64 oldStateSize = _coefficientsPerPhase > 0 ? _coefficientsPerPhase - 1 : 0;

        _upRate = destination;
        _downRate = source;

        if (IsIdentity())
            return;

        // The cutoff depends on the ratio only: any rate works as the reference here.
        GenerateInterpolatingFilter(_upRate * 1000);
        _timeModuloUpRate = 0;

        if (_channelCount > 0)
            InitializeStateBuffer(oldStateSize);
    }

    void DefaultResamplerInstance::Reset()
    {
        _timeModuloUpRate = 0;
        _lastProcessedSample = 0;
        _state.Clear();
    }

    void DefaultResamplerInstance::Clear()
    {
        Reset();

        _upRate = 0;
        _downRate = 0;
        _channelCount = 0;
        _coefficientsPerPhase = 0;
        _filterLength = 0;
        _transposedFilterCoefficients.Clear();
        _temporaryFilterCoefficients.Clear();

        _sampleRateIn = 0;
        _sampleRateOut = 0;
    }

    void DefaultResamplerInstance::InitializeStateBuffer(AmUInt64 oldFrameCount)
    {
        // Update the state buffer if it is null or if the number of coefficients per phase in the polyphase filter has changed.
        if (IsIdentity() || _channelCount == 0)
            return;

        // If the state buffer is to be kept. For example in the case of a change
        // in either source or destination sampling rate, maintaining the old state
        // buffers contents allows a glitch free transition.
        const AmUInt64 newFrameCount = _coefficientsPerPhase > 0 ? _coefficientsPerPhase - 1 : 0;
        if (oldFrameCount != newFrameCount)
        {
            const AmUInt64 stateCapacity = static_cast<AmUInt64>(_state.GetFrameCount());
            const AmUInt64 minSize = AM_MIN(std::min(newFrameCount, oldFrameCount), stateCapacity);
            const AmUInt64 maxSize = AM_MIN(std::max(newFrameCount, oldFrameCount), stateCapacity);

            for (AmUInt64 channel = 0; channel < _channelCount; ++channel)
            {
                auto& state_channel = _state[channel];
                AMPLITUDE_ASSERT(state_channel.begin() + maxSize <= state_channel.end());
                std::fill(state_channel.begin() + minSize, state_channel.begin() + maxSize, 0.0f);
            }
        }
    }

    void DefaultResamplerInstance::GenerateInterpolatingFilter(AmUInt64 sampleRate)
    {
        // See "Digital Signal Processing", 4th Edition, Prolakis and Manolakis,
        // Pearson, Chapter 11 (specifically Figures 11.5.10 and 11.5.13).
        const AmUInt64 maxRate = std::max(_upRate, _downRate);
        const AmReal32 cutoffFrequency = static_cast<AmReal32>(sampleRate) / static_cast<AmReal32>(2 * maxRate);

        AmUInt64 filterLength = maxRate * kTransitionBandwidthRatio;
        filterLength += filterLength % 2;

        // Defense in depth: Initialize() snaps the rate pair so this clamp cannot trigger.
        filterLength = AM_MIN(filterLength, static_cast<AmUInt64>(_temporaryFilterCoefficients.GetFrameCount()));
        _filterLength = filterLength;

        auto* filterChannel = &_temporaryFilterCoefficients[0];
        filterChannel->clear();

        GenerateSincFilter(cutoffFrequency, static_cast<AmReal32>(sampleRate), filterLength, filterChannel);

        // Pad out the filter length so that it can be arranged in polyphase fashion.
        const AmUInt64 transposedLength = filterLength + maxRate - (filterLength % maxRate);
        _coefficientsPerPhase = transposedLength / maxRate;

        ArrangeFilterAsPolyphase(filterLength, *filterChannel);
    }

    void DefaultResamplerInstance::ArrangeFilterAsPolyphase(AmUInt64 filterLength, const AudioBufferChannel& filter)
    {
        // Coefficients are transposed and flipped.
        // Suppose _upRate is 3, and the input number of coefficients is 10,
        // h[0], ..., h[9].
        // Then the _transposedFilterCoefficients buffer will look like this:
        // h[9], h[6], h[3], h[0],   flipped phase 0 coefs.
        //  0,   h[7], h[4], h[1],   flipped phase 1 coefs (zero-padded).
        //  0,   h[8], h[5], h[2],   flipped phase 2 coefs (zero-padded).
        _transposedFilterCoefficients.Clear();
        auto& transposedCoefficientsChannel = _transposedFilterCoefficients[0];

        const AmUInt64 transposedCapacity = static_cast<AmUInt64>(_transposedFilterCoefficients.GetFrameCount());

        for (AmUInt64 i = 0; i < _upRate; ++i)
        {
            for (AmUInt64 j = 0; j < _coefficientsPerPhase; ++j)
            {
                if (j * _upRate + i >= filterLength)
                    continue;

                const AmUInt64 coeffIndex = (_coefficientsPerPhase - 1 - j) + i * _coefficientsPerPhase;
                if (coeffIndex >= transposedCapacity)
                    continue;

                transposedCoefficientsChannel[coeffIndex] = filter[j * _upRate + i];
            }
        }
    }

    void DefaultResamplerInstance::GenerateSincFilter(
        AmReal32 cutoffFrequency, AmUInt64 sampleRate, AmUInt64 filterLength, AudioBufferChannel* filter)
    {
        AMPLITUDE_ASSERT(sampleRate > 0.0f);
        const AmReal32 angularCutoffFrequency = 2.0f * AM_PI32 * cutoffFrequency / sampleRate;

        const size_t half_filter_length = filterLength / 2;
        GenerateHannWindow(true, filterLength, filter);
        auto* filterChannel = &filter[0];

        for (size_t i = 0; i < filterLength; ++i)
        {
            if (i == half_filter_length)
            {
                (*filterChannel)[half_filter_length] *= angularCutoffFrequency;
            }
            else
            {
                const AmReal32 denominator = static_cast<AmReal32>(i) - (static_cast<AmReal32>(filterLength) / 2.0f);
                AMPLITUDE_ASSERT(std::abs(denominator) > kEpsilon);
                (*filterChannel)[i] *= std::sin(angularCutoffFrequency * denominator) / denominator;
            }
        }

        // Normalize.
        const AmReal32 normalizingFactor =
            static_cast<AmReal32>(_upRate) / std::accumulate(filterChannel->begin(), filterChannel->end(), 0.0f);
        ScalarMultiply(filterChannel->begin(), filterChannel->begin(), normalizingFactor, filterLength);
    }

    std::shared_ptr<ResamplerInstance> DefaultResampler::CreateInstance()
    {
        return ampoolshared(eMemoryPoolKind_Filtering, DefaultResamplerInstance);
    }
} // namespace SparkyStudios::Audio::Amplitude
