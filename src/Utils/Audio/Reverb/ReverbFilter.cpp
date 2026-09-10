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

#include <Utils/Audio/Reverb/ReverbFilter.h>

namespace SparkyStudios::Audio::Amplitude
{
    ReverbFilters::ReverbFilters()
    {
        _a = 0.0f;
        _y = 0.0f;
        _delayIndex = 0;
        _feedback = 0.8f;
        _gainCoeff = 0.85f;
        _lastLowPassY = 0.0f;
    }

    void ReverbFilters::Init(AmUInt64 sampleRate, AmUInt32 maxDelaySamples)
    {
        const AmUInt32 delaySize = maxDelaySamples > 0 ? maxDelaySamples : (sampleRate > 0 ? static_cast<AmUInt32>(sampleRate / 6) : 1u);
        _delayLines.resize(delaySize, 0.0f);
        _initialized = true;
    }

    AmReal32 ReverbFilters::TwoPoint(AmReal32 x)
    {
        _a = 0.5f * (x + _a);
        return _a;
    }

    AmReal32 ReverbFilters::Comb1(AmReal32 x, AmInt32 size)
    {
        _delaySize = size;
        _y = _delayLines[_delayIndex];
        _delayLines[_delayIndex] = x + _feedback * _y;
        _delayIndex != _delaySize - 1 ? _delayIndex++ : _delayIndex = 0;

        return _y;
    }

    AmReal32 ReverbFilters::CombFeedForward(AmReal32 x, AmInt32 size)
    {
        _delaySize = size;
        _y = x + _delayLines[_delayIndex];
        _delayLines[_delayIndex] = x;
        _delayIndex != _delaySize - 1 ? _delayIndex++ : _delayIndex = 0;

        return _y;
    }

    AmReal32 ReverbFilters::CombFeedBack(AmReal32 x, AmInt32 size, AmReal32 fb)
    {
        _delaySize = size;
        _y = x + _delayLines[_delayIndex] * fb;
        _delayLines[_delayIndex] = _y;
        _delayIndex != _delaySize - 1 ? _delayIndex++ : _delayIndex = 0;

        return _y;
    }

    AmReal32 ReverbFilters::LowPassCombFeedBack(AmReal32 x, AmInt32 size, AmReal32 fb, AmReal32 cutOff)
    {
        _delaySize = size;
        _y = x + LowPass(_delayLines[_delayIndex], 1.0f - cutOff) * fb;
        _delayLines[_delayIndex] = _y;
        _delayIndex != _delaySize - 1 ? _delayIndex++ : _delayIndex = 0;

        return _y;
    }

    AmReal32 ReverbFilters::AllPass(AmReal32 x, AmInt32 size)
    {
        _delaySize = size;
        x += _delayLines[_delayIndex] * _gainCoeff;
        _y = _delayLines[_delayIndex] + (x * (-_gainCoeff));
        _delayLines[_delayIndex] = x;
        _delayIndex != _delaySize - 1 ? _delayIndex++ : _delayIndex = 0;

        return _y;
    }

    AmReal32 ReverbFilters::AllPass(AmReal32 x, AmInt32 size, AmReal32 feedBack)
    {
        _delaySize = size;
        x += _delayLines[_delayIndex] * feedBack;
        _y = _delayLines[_delayIndex] + (x * (-feedBack));
        _delayLines[_delayIndex] = x;
        _delayIndex != _delaySize - 1 ? _delayIndex++ : _delayIndex = 0;

        return _y;
    }

    AmReal32 ReverbFilters::AllPassTap(
        SparkyStudios::Audio::Amplitude::AmReal32 x,
        SparkyStudios::Audio::Amplitude::AmInt32 size,
        SparkyStudios::Audio::Amplitude::AmInt32 tap)
    {
        _delaySize = size;
        x += _delayLines[_delayIndex] * _gainCoeff;

        AmInt32 t = _delayIndex + tap;
        if (t >= _delaySize - 1)
            t -= _delaySize;

        _y = _delayLines[t];
        _delayLines[_delayIndex] = x;
        _delayIndex != _delaySize - 1 ? _delayIndex++ : _delayIndex = 0;

        return _y;
    }

    void ReverbFilters::SetLength(AmInt32 length)
    {}

    AmReal32 ReverbFilters::OneTap(AmReal32 x, AmInt32 size)
    {
        _delaySize = size;
        _y = _delayLines[_delayIndex];
        _delayLines[_delayIndex] = x;
        _delayIndex != _delaySize - 1 ? _delayIndex++ : _delayIndex = 0;

        return _y;
    }

    AmReal32 ReverbFilters::TapDelay(AmReal32 x, AmInt32 size, AmReal32Buffer taps, AmInt32 numTaps)
    {
        _y = 0.0f;
        _delaySize = size;

        for (AmInt32 i = 0; i < numTaps; i++)
        {
            AmInt32 t = (AmInt32)(taps[i] * (size - 1));
            AmInt32 y = _delayIndex + t;

            if (y > _delaySize - 1)
                y -= _delaySize;

            _y += _delayLines[y];
        }

        _delayLines[_delayIndex] = x;
        _delayIndex != _delaySize - 1 ? _delayIndex++ : _delayIndex = 0;

        return _y;
    }

    AmReal32 ReverbFilters::TapDelayWithGain(AmReal32 x, AmInt32 size, AmReal32Buffer taps, AmInt32 numTaps, AmReal32Buffer gain)
    {
        _y = 0.0f;
        _delaySize = size;

        for (AmInt32 i = 0; i < numTaps; i++)
        {
            AmInt32 t = (AmInt32)(taps[i] * (size - 1));
            AmInt32 y = _delayIndex + t;

            if (y > _delaySize - 1)
                y -= _delaySize;

            _y += _delayLines[y] * gain[i];
        }

        _delayLines[_delayIndex] = x;
        _delayIndex != _delaySize - 1 ? _delayIndex++ : _delayIndex = 0;

        return _y;
    }

    AmReal32 ReverbFilters::TapDelayPos(AmReal32 x, AmInt32 size, AmInt32Buffer taps, AmInt32 numTaps)
    {
        _y = 0.0f;
        _delaySize = size;

        for (AmInt32 i = 0; i < numTaps; i++)
            _y += _delayLines[taps[i]];

        _delayLines[_delayIndex] = x;
        _delayIndex != _delaySize - 1 ? _delayIndex++ : _delayIndex = 0;

        return _y;
    }

    AmReal32 ReverbFilters::GetTap(AmInt32 tap)
    {
        AmInt32 t = _delayIndex + tap;
        if (t > _delaySize - 1)
            t -= _delaySize;

        _y = _delayLines[t];
        return _delayLines[t];
    }

    AmReal32 ReverbFilters::LowPass(AmReal32 x, AmReal32 cutOff)
    {
        AmReal32 y = _lastLowPassY + cutOff * (x - _lastLowPassY);
        _lastLowPassY = y;
        return y;
    }
} // namespace SparkyStudios::Audio::Amplitude
