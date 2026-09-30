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
        AmUInt64 position, AmUInt64 clock, AmReal64 sourceFramesPerOutputFrame, AmUInt64 regionStart, AmUInt64 regionEnd, bool loop)
    {
        _start = regionStart;
        _end = AM_MAX(regionEnd, regionStart);
        _position = AM_CLAMP(position, _start, _end);
        _clock = clock;
        _rate = std::isfinite(sourceFramesPerOutputFrame) && sourceFramesPerOutputFrame > 0.0 ? sourceFramesPerOutputFrame : 1.0;
        _loop = loop;
        _anchored = true;
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

        const AmUInt64 length = _end - _start;
        return length == 0 ? _start : _start + (advanced - _start) % length;
    }

    bool VirtualCursor::HasEnded(AmUInt64 clock) const
    {
        return _anchored && !_loop && Advanced(clock) >= _end;
    }
} // namespace SparkyStudios::Audio::Amplitude
