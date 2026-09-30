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

#include <Mixer/Voice/VirtualCursor.h>

namespace SparkyStudios::Audio::Amplitude
{
    void VirtualCursor::Anchor(
        AmUInt64 position,
        AmUInt64 clock,
        AmReal64 sourceFramesPerOutputFrame,
        AmUInt64 regionStart,
        AmUInt64 regionEnd,
        bool loop,
        AmUInt32 loopsRemaining)
    {
        _start = regionStart;
        _end = AM_MAX(regionEnd, regionStart);
        _position = AM_CLAMP(position, _start, _end);
        _clock = clock;
        _rate = std::isfinite(sourceFramesPerOutputFrame) && sourceFramesPerOutputFrame > 0.0 ? sourceFramesPerOutputFrame : 1.0;
        _loop = loop;
        _anchored = true;

        const AmUInt64 length = _end - _start;
        _endUnwrapped =
            (!loop || loopsRemaining == 0 || length == 0) ? kUnbounded : _end + static_cast<AmUInt64>(loopsRemaining - 1) * length;
    }

    void VirtualCursor::Clear()
    {
        _anchored = false;
    }

    AmUInt64 VirtualCursor::Advanced(AmUInt64 clock) const
    {
        const AmUInt64 elapsed = clock > _clock ? clock - _clock : 0;
        return _position + static_cast<AmUInt64>(std::floor(static_cast<AmReal64>(elapsed) * _rate));
    }

    AmUInt64 VirtualCursor::PositionAt(AmUInt64 clock) const
    {
        const AmUInt64 advanced = Advanced(clock);
        if (!_loop)
            return AM_MIN(advanced, _end);

        if (_endUnwrapped != kUnbounded && advanced >= _endUnwrapped)
            return _end; // A finite loop count ran out: clamp at the end of its final pass, like a non-looping cursor.

        const AmUInt64 length = _end - _start;
        return length == 0 ? _start : _start + (advanced - _start) % length;
    }

    bool VirtualCursor::HasEnded(AmUInt64 clock) const
    {
        if (!_anchored)
            return false;

        if (!_loop)
            return Advanced(clock) >= _end;

        return _endUnwrapped != kUnbounded && Advanced(clock) >= _endUnwrapped;
    }

    AmUInt32 VirtualCursor::LoopsRemainingAt(AmUInt64 clock) const
    {
        if (!_loop || _endUnwrapped == kUnbounded)
            return 0;

        const AmUInt64 length = _end - _start;
        if (length == 0)
            return 1;

        const AmUInt64 advanced = Advanced(clock);
        if (advanced >= _endUnwrapped)
            return 0;

        const AmUInt64 remaining = _endUnwrapped - advanced;
        return static_cast<AmUInt32>((remaining + length - 1) / length); // Ceiling division: a partial pass still counts as one.
    }
} // namespace SparkyStudios::Audio::Amplitude
