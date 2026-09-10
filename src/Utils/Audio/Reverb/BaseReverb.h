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

#pragma once

#ifndef _AM_IMPLEMENTATION_UTILS_AUDIO_REVERB_BASE_REVERB_H
#define _AM_IMPLEMENTATION_UTILS_AUDIO_REVERB_BASE_REVERB_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <Utils/Audio/Reverb/ReverbFilter.h>

namespace SparkyStudios::Audio::Amplitude::Internal
{
    class BaseReverb
    {
    public:
        BaseReverb(AmUInt64 sampleRate);
        virtual ~BaseReverb() = default;

        virtual void Mute();

    protected:
        static constexpr AmUInt32 kNumFilters = 32;
        static constexpr AmUInt32 kInitialActiveCombs = 16;
        static constexpr AmUInt32 kInitialActiveAllPass = 8;

        AmReal32 ParallelLowPassComb(AmReal32 x, AmInt32 firstFilter, AmUInt32 numFilters);
        AmReal32 SerialAllPass(AmReal32 x, AmInt32 firstFilter, AmUInt32 numFilters);
        AmReal32 SerialAllPass(AmReal32 x, AmInt32 firstFilter, AmUInt32 numFilters, AmReal32 feedback);
        AmReal32 ParallelComb(AmReal32 x, AmInt32 firstFilter, AmUInt32 numFilters);
        AmReal32 AllPassCombCombo(AmReal32 x, AmReal32 gainCoeff);

        void SetWeights(const AmReal32* weights, AmUInt32 numSet, AmReal32* filter);
        void SetAllPassWeights(const AmReal32* weights, AmUInt32 numSet);
        void SetCombWeights(const AmReal32* weights, AmUInt32 numSet);
        void SetCombWeightsAll(AmReal32 feedback);

        void SetCombTimesMS(const AmReal32* times, AmUInt32 numSet);
        void SetAllPassTimesMS(const AmReal32* times, AmUInt32 numSet);

        void SetCombTimes(const AmReal32* times, AmUInt32 numSet);
        void SetAllPassTimes(const AmReal32* times, AmUInt32 numSet);

        void SetLowPassCombCutOff(const AmReal32* cutoffs, AmUInt32 numSet);
        void SetLowPassCombCutOff(AmReal32 cutoff);

        void SetCombFeedback(const AmReal32* feedbacks, AmUInt32 numSet);

        AmInt32 MSToDelLength(AmReal32 timeMS) const;
        static void LimitNumFilters(AmUInt32& numFilters, AmUInt32 startIndex);

        AmReal32 _feedBackSignal[8];

        ReverbFilters _arrayAllPass[kNumFilters];
        ReverbFilters _arrayLowPass[kNumFilters];
        ReverbFilters _arrayTwo[kNumFilters];

        AmReal32 _feedBackComb[kNumFilters];
        AmReal32 _feedBackAllPass[kNumFilters];
        AmReal32 _combGainWeight[kNumFilters];
        AmReal32 _allPassGainWeight[kNumFilters];
        AmReal32 _lowPassCombCutoff[kNumFilters];
        AmReal32 _feedBackCombFB[kNumFilters];

        AmReal32 _sy[2];
        AmReal32 _y;

        AmReal32 _accumulator;
        AmReal32 _numSamplesMS;
        AmUInt64 _sampleRate = 0;

        ReverbFilters _earlyRef;
        AmReal32 _taps[kNumFilters];
        AmReal32 _tapsGain[kNumFilters];

        AmInt32 _numTaps;
        AmInt32 _tapDelLength;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_UTILS_AUDIO_REVERB_BASE_REVERB_H
