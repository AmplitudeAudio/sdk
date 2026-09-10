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

#include <Utils/Audio/Reverb/BaseReverb.h>

#include <cmath>

namespace SparkyStudios::Audio::Amplitude
{
    BaseReverb::BaseReverb(AmUInt64 sampleRate)
    {
        const constexpr AmUInt32 kSizeOfSample = sizeof(AmReal32);
        const constexpr AmUInt32 kNumFilterSize = kSizeOfSample * kNumFilters;

        std::memset(_feedBackComb, 0, kNumFilterSize);
        std::memset(_feedBackAllPass, 0, kNumFilterSize);
        std::memset(_combGainWeight, 0, kNumFilterSize);
        std::memset(_allPassGainWeight, 0, kNumFilterSize);
        std::memset(_taps, 0, kNumFilterSize);
        std::memset(_tapsGain, 0, kNumFilterSize);
        std::memset(_feedBackCombFB, 0, kNumFilterSize);
        std::memset(_lowPassCombCutoff, 0, kNumFilterSize);
        std::memset(_sy, 0, kSizeOfSample * 2);

        const AmUInt32 maxAllPassDelay = sampleRate > 0 ? static_cast<AmUInt32>(std::ceil(sampleRate * (1000.0f / 48000.0f))) : 1000u;
        const AmUInt32 maxCombDelay = sampleRate > 0 ? static_cast<AmUInt32>(std::ceil(sampleRate * (2500.0f / 48000.0f))) : 2500u;
        const AmUInt32 maxEarlyRefDelay = sampleRate > 0 ? static_cast<AmUInt32>(sampleRate / 10.0f) : 4800u;

        for (auto& i : _arrayAllPass)
            i.Init(sampleRate, maxAllPassDelay);

        for (auto& i : _arrayLowPass)
            i.Init(sampleRate, 1);

        for (auto& i : _arrayTwo)
            i.Init(sampleRate, maxCombDelay);

        _earlyRef.Init(sampleRate, maxEarlyRefDelay);

        _numSamplesMS = sampleRate / kAmSecond;
        _y = 0.0f;
        _accumulator = 0.0f;
        _numTaps = kNumFilters;

        for (AmUInt32 i = 0; i < kNumFilters; ++i)
        {
            _taps[i] = 1.0f / 10.0f;
            _tapsGain[i] = 0.1f;
        }

        _taps[0] = 0.5f;
        _taps[1] = 0.2f;
        _tapDelLength = sampleRate / 10.0f;

        for (AmUInt32 i = 0; i < kNumFilters; ++i)
        {
            _feedBackComb[i] = 13 * (i + 1);
            _feedBackAllPass[i] = 13 * (i + 1);
            _combGainWeight[i] = 1.0f;
            _allPassGainWeight[i] = 1.0f;
            _lowPassCombCutoff[i] = 0.2f;
            _feedBackCombFB[i] = 0.5f;
        }

        _feedBackComb[0] = 50.0f;
        _feedBackComb[1] = 100.0f;
        _feedBackComb[2] = 150.0f;
    }

    AmReal32 BaseReverb::ParallelLowPassComb(AmReal32 x, AmInt32 firstFilter, AmUInt32 numFilters)
    {
        _accumulator = 0.0f;
        LimitNumFilters(numFilters, firstFilter);

        for (AmInt32 i = firstFilter; i < numFilters; ++i)
            _accumulator +=
                _arrayTwo[i].LowPassCombFeedBack(x, static_cast<AmInt32>(_feedBackComb[i]), _combGainWeight[i], _lowPassCombCutoff[i]);

        return _accumulator;
    }

    void BaseReverb::SetCombTimesMS(const AmReal32* times, AmUInt32 numSet)
    {
        LimitNumFilters(numSet, 0);
        for (AmUInt32 i = 0; i < numSet; ++i)
            _feedBackComb[i] = MSToDelLength(times[i]);
    }

    void BaseReverb::SetAllPassTimesMS(const AmReal32* times, AmUInt32 numSet)
    {
        LimitNumFilters(numSet, 0);
        for (AmUInt32 i = 0; i < numSet; ++i)
            _feedBackAllPass[i] = MSToDelLength(times[i]);
    }

    AmReal32 BaseReverb::SerialAllPass(AmReal32 x, AmInt32 firstFilter, AmUInt32 numFilters)
    {
        AmReal32 t = x;
        LimitNumFilters(numFilters, firstFilter);

        for (AmInt32 i = firstFilter; i < numFilters; ++i)
            t = _arrayAllPass[i].AllPass(t, static_cast<AmInt32>(_feedBackAllPass[i]), _allPassGainWeight[i]);

        _y = t;
        return _y;
    }

    AmReal32 BaseReverb::SerialAllPass(AmReal32 x, AmInt32 firstFilter, AmUInt32 numFilters, AmReal32 feedback)
    {
        AmReal32 t = x;
        LimitNumFilters(numFilters, firstFilter);

        for (AmInt32 i = firstFilter; i < numFilters; ++i)
            t = _arrayAllPass[i].AllPass(t, static_cast<AmInt32>(_feedBackAllPass[i]), feedback);

        _y = t;
        return _y;
    }

    AmReal32 BaseReverb::ParallelComb(AmReal32 x, AmInt32 firstFilter, AmUInt32 numFilters)
    {
        _accumulator = 0.0f;
        LimitNumFilters(numFilters, firstFilter);

        for (AmInt32 i = firstFilter; i < numFilters; ++i)
            _accumulator += _arrayTwo[i].CombFeedBack(x, static_cast<AmInt32>(_feedBackComb[i]), _combGainWeight[i]);

        return _accumulator;
    }

    AmReal32 BaseReverb::AllPassCombCombo(AmReal32 x, AmReal32 gainCoeff)
    {
        // Implementation of Schroeders first Reverb structure
        // input -> 8 parallel comb filters -> 4 serial combs -> output + input
        // http://www.music.miami.edu/programs/mue/mue2003/research/jfrenette/chapter_2/chapter_2.html
        // _earlyRef mimics early reflections, 3 taps
        AmReal32 t = _earlyRef.TapDelayWithGain(x, _tapDelLength, _taps, 3, _tapsGain);
        AmReal32 combResult = ParallelComb(t, 0, 4);
        AmReal32 allPassResult = SerialAllPass(combResult, 0, 2);
        _y = allPassResult * gainCoeff;

        return _y + t;
    }

    void BaseReverb::SetWeights(const AmReal32* weights, AmUInt32 numSet, AmReal32* filter)
    {
        LimitNumFilters(numSet, 0);
        for (AmUInt32 i = 0; i < numSet; ++i)
            filter[i] = weights[i];
    }

    void BaseReverb::SetAllPassWeights(const AmReal32* weights, AmUInt32 numSet)
    {
        SetWeights(weights, numSet, _allPassGainWeight);
    }

    void BaseReverb::SetCombWeights(const AmReal32* weights, AmUInt32 numSet)
    {
        SetWeights(weights, numSet, _combGainWeight);
    }

    void BaseReverb::SetCombWeightsAll(AmReal32 feedback)
    {
        feedback = AM_CLAMP(feedback, 0.0f, 1.0f);
        for (AmReal32& i : _combGainWeight)
            i = feedback;
    }

    void BaseReverb::SetCombTimes(const AmReal32* times, AmUInt32 numSet)
    {
        LimitNumFilters(numSet, 0);
        for (AmUInt32 i = 0; i < numSet; ++i)
            _feedBackComb[i] = times[i];
    }

    void BaseReverb::SetAllPassTimes(const AmReal32* times, AmUInt32 numSet)
    {
        LimitNumFilters(numSet, 0);
        for (AmUInt32 i = 0; i < numSet; ++i)
            _feedBackAllPass[i] = times[i];
    }

    void BaseReverb::SetLowPassCombCutOff(const AmReal32* cutoffs, AmUInt32 numSet)
    {
        LimitNumFilters(numSet, 0);
        for (AmUInt32 i = 0; i < numSet; ++i)
            _lowPassCombCutoff[i] = cutoffs[i];
    }

    void BaseReverb::SetLowPassCombCutOff(AmReal32 cutoff)
    {
        cutoff = AM_CLAMP(cutoff, 0.0f, 1.0f);
        for (AmReal32& i : _lowPassCombCutoff)
            i = cutoff;
    }

    void BaseReverb::SetCombFeedback(const AmReal32* feedbacks, AmUInt32 numSet)
    {
        LimitNumFilters(numSet, 0);
        for (AmUInt32 i = 0; i < numSet; ++i)
            _feedBackCombFB[i] = feedbacks[i];
    }

    AmInt32 BaseReverb::MSToDelLength(AmReal32 timeMS) const
    {
        return static_cast<AmInt32>(timeMS * _numSamplesMS);
    }

    void BaseReverb::LimitNumFilters(AmUInt32& numFilters, AmUInt32 startIndex)
    {
        if (numFilters + startIndex > kNumFilters)
            numFilters = kNumFilters;
        else
            numFilters += startIndex;
    }
} // namespace SparkyStudios::Audio::Amplitude
