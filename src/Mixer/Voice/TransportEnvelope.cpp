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

#include <SparkyStudios/Audio/Amplitude/Math/Utils.h>

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

                if (_position == _length)
                    _gain = _target;
                else
                    _gain = static_cast<AmReal32>(_declick ? Evaluate(_position) : Interpolate(_position));
            }

            gains[offset + i] = _gain;
        }
    }

    AmReal64 TransportEnvelope::Interpolate(AmUInt64 position)
    {
        // position is the 1-based end of the frame: frame k has the value at (k + 1) / L.
        const AmUInt64 start = (position - 1) / kSegmentFrames * kSegmentFrames;
        const AmUInt64 end = AM_MIN(start + kSegmentFrames, _length);

        if (start != _knotStart)
        {
            _knotStart = start;
            _knotFrom = Evaluate(start);
            _knotTo = Evaluate(end);
            _slopeFrom = Slope(start);
            _slopeTo = Slope(end);
        }

        // Cubic Hermite between the knots, with the curve's own slopes: the result is C1 at every knot, so no corner
        // train is added to the curve.
        const auto width = static_cast<AmReal64>(end - start);
        const AmReal64 t = static_cast<AmReal64>(position - start) / width;
        const AmReal64 t2 = t * t;
        const AmReal64 t3 = t2 * t;

        return (2.0 * t3 - 3.0 * t2 + 1.0) * _knotFrom + (t3 - 2.0 * t2 + t) * width * _slopeFrom + (-2.0 * t3 + 3.0 * t2) * _knotTo +
            (t3 - t2) * width * _slopeTo;
    }

    AmReal64 TransportEnvelope::Slope(AmUInt64 position)
    {
        // Central difference over one frame (one-sided at the fade ends), in gain per frame.
        const AmUInt64 low = position > 0 ? position - 1 : 0;
        const AmUInt64 high = AM_MIN(position + 1, _length);
        return (Evaluate(high) - Evaluate(low)) / static_cast<AmReal64>(high - low);
    }

    AmReal64 TransportEnvelope::Evaluate(AmUInt64 position)
    {
        const AmReal64 p = static_cast<AmReal64>(position) / static_cast<AmReal64>(_length);
        const auto from = static_cast<AmReal64>(_from);
        const auto target = static_cast<AmReal64>(_target);

        // The de-click is a raised cosine, evaluated exactly on every frame (see Render).
        if (_declick)
            return from + (target - from) * (0.5 - 0.5 * std::cos(AM_PI * p));

        if (_curve != nullptr)
            return _curve->GetFromPercentage(p);

        return from + (target - from) * p;
    }
} // namespace SparkyStudios::Audio::Amplitude
