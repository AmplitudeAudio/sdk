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

#include <cmath>

#include <Mixer/Voice/TransportEnvelope.h>

namespace SparkyStudios::Audio::Amplitude
{
    void TransportEnvelope::Initialize(std::shared_ptr<FaderInstance> curve, AmUInt32 outputRate)
    {
        _curve = std::move(curve);
        _rate = AM_MAX(outputRate, 1U);
        SetGain(1.0f);
    }

    void TransportEnvelope::SetGain(AmReal32 gain)
    {
        _gain = _from = _target = gain;
        _length = _position = 0;
    }

    AmUInt64 TransportEnvelope::GetFadeLength(AmTime duration) const
    {
        return AM_MAX(MillisecondsToFrames(AM_MAX(duration, kDeclickFade), _rate), 1ULL);
    }

    AmUInt64 TransportEnvelope::FadeTo(AmReal32 target, AmTime duration)
    {
        _declick = duration < kDeclickFade;
        _length = GetFadeLength(duration);
        _position = 0;
        _from = _gain;
        _target = target;
        _knotStart = std::numeric_limits<AmUInt64>::max();

        if (!_declick && _curve != nullptr)
            _curve->Set(_from, _target);

        return _length;
    }

    void TransportEnvelope::Render(AudioBufferChannel& gains, AmUInt64 offset, AmUInt64 frames)
    {
        for (AmUInt64 i = 0; i < frames; ++i)
        {
            if (_position < _length)
            {
                ++_position;
                _gain = _position == _length ? _target : Interpolate(_position);
            }

            gains[offset + i] = _gain;
        }
    }

    AmReal32 TransportEnvelope::Interpolate(AmUInt64 position)
    {
        // position is the 1-based end of the frame: frame k has the value at (k + 1) / L.
        const AmUInt64 start = (position - 1) / kSegmentFrames * kSegmentFrames;
        const AmUInt64 end = AM_MIN(start + kSegmentFrames, _length);

        if (start != _knotStart)
        {
            _knotStart = start;
            _knotFrom = Evaluate(start);
            _knotTo = Evaluate(end);
        }

        const AmReal32 t = static_cast<AmReal32>(position - start) / static_cast<AmReal32>(end - start);
        return _knotFrom + (_knotTo - _knotFrom) * t;
    }

    AmReal32 TransportEnvelope::Evaluate(AmUInt64 position)
    {
        const AmReal64 p = static_cast<AmReal64>(position) / static_cast<AmReal64>(_length);

        if (_declick)
            return _from + (_target - _from) * static_cast<AmReal32>(0.5 - 0.5 * std::cos(3.14159265358979323846 * p));

        if (_curve != nullptr)
            return static_cast<AmReal32>(_curve->GetFromPercentage(p));

        return _from + (_target - _from) * static_cast<AmReal32>(p);
    }
} // namespace SparkyStudios::Audio::Amplitude
