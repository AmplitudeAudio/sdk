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

#pragma once

#ifndef _AM_IMPLEMENTATION_MIXER_VOICE_VIRTUAL_CURSOR_H
#define _AM_IMPLEMENTATION_MIXER_VOICE_VIRTUAL_CURSOR_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Where a sound would be if it were still playing: a source position anchored to an audio-clock frame.
     */
    class VirtualCursor
    {
    public:
        /**
         * @brief Anchors the cursor: @p position is the source frame heard at audio-clock frame @p clock.
         *
         * @param position The source frame heard at @p clock.
         * @param clock The audio-clock frame at which @p position was heard.
         * @param sourceFramesPerOutputFrame How many source frames elapse per output frame (pitch times speed times
         * the source-to-output sample rate ratio); non-finite or non-positive values fall back to 1.0.
         * @param regionStart The first frame of the playable region.
         * @param regionEnd The first frame past the playable region.
         * @param loop Whether the region loops.
         */
        void Anchor(
            AmUInt64 position, AmUInt64 clock, AmReal64 sourceFramesPerOutputFrame, AmUInt64 regionStart, AmUInt64 regionEnd, bool loop);

        /**
         * @brief Clears the anchor.
         */
        void Clear();

        [[nodiscard]] bool IsAnchored() const
        {
            return _anchored;
        }

        /**
         * @brief Gets the source frame the cursor would be at, at audio-clock frame @p clock.
         *
         * Clocks before the anchor do not rewind: the anchor position is returned.
         */
        [[nodiscard]] AmUInt64 PositionAt(AmUInt64 clock) const;

        /**
         * @brief Checks whether a non-looping cursor reached the end of its region by audio-clock frame @p clock.
         */
        [[nodiscard]] bool HasEnded(AmUInt64 clock) const;

        [[nodiscard]] AmUInt64 GetAnchorPosition() const
        {
            return _position;
        }

        [[nodiscard]] AmUInt64 GetAnchorClock() const
        {
            return _clock;
        }

    private:
        [[nodiscard]] AmUInt64 Advanced(AmUInt64 clock) const;

        AmUInt64 _position = 0;
        AmUInt64 _clock = 0;
        AmReal64 _rate = 1.0;
        AmUInt64 _start = 0;
        AmUInt64 _end = 0;
        bool _loop = false;
        bool _anchored = false;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_VOICE_VIRTUAL_CURSOR_H
