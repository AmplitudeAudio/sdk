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

#ifndef _AM_IMPLEMENTATION_UTILS_AUDIO_REVERB_REVERB_FILTERS_H
#define _AM_IMPLEMENTATION_UTILS_AUDIO_REVERB_REVERB_FILTERS_H

#include <valarray>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude::Internal
{
    class ReverbFilters
    {
    public:
        explicit ReverbFilters();

        void Init(AmUInt64 sampleRate, AmUInt32 maxDelaySamples = 0);
        void Mute();

        AmReal32 TwoPoint(AmReal32 x);
        AmReal32 Comb1(AmReal32 x, AmInt32 size);
        AmReal32 CombFeedForward(AmReal32 x, AmInt32 size);
        AmReal32 CombFeedBack(AmReal32 x, AmInt32 size, AmReal32 fb);
        AmReal32 LowPassCombFeedBack(AmReal32 x, AmInt32 size, AmReal32 fb, AmReal32 cutOff);

        AmReal32 AllPass(AmReal32 x, AmInt32 size);
        AmReal32 AllPass(AmReal32 x, AmInt32 size, AmReal32 feedBack);
        AmReal32 AllPassTap(AmReal32 x, SparkyStudios::Audio::Amplitude::AmInt32 size, AmInt32 tap);

        void SetLength(AmInt32 length);

        AmReal32 OneTap(AmReal32 x, AmInt32 size);
        AmReal32 TapDelay(AmReal32 x, AmInt32 size, AmReal32Buffer taps, AmInt32 numTaps);
        AmReal32 TapDelayWithGain(AmReal32 x, AmInt32 size, AmReal32Buffer taps, AmInt32 numTaps, AmReal32Buffer gain);
        AmReal32 TapDelayPos(AmReal32 x, AmInt32 size, AmInt32Buffer taps, AmInt32 numTaps);
        AmReal32 GetTap(AmInt32 tap);

        AmReal32 LowPass(AmReal32 x, AmReal32 cutOff);

        bool _initialized = false;

        AmReal32 _lastLowPassY;
        std::valarray<AmReal32> _delayLines;
        AmReal32 _a;
        AmInt32 _delayIndex;
        AmInt32 _delaySize;
        AmReal32 _feedback;
        AmReal32 _y;
        AmReal32 _gainCoeff;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_UTILS_AUDIO_REVERB_REVERB_FILTERS_H
