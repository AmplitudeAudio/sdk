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

#ifndef _AM_IMPLEMENTATION_MIXER_VOICE_SOURCE_READER_H
#define _AM_IMPLEMENTATION_MIXER_VOICE_SOURCE_READER_H

#include <limits>

#include <SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h>
#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct SoundData;

    /**
     * @brief Where a voice reads its frames: a whole sound in memory, or a stream decoded on demand.
     */
    struct VoiceSource
    {
        /// Decodes up to @p frames frames starting at source frame @p offset into @c buffer, from frame 0. Returns the count.
        using DecodeFunction = AmUInt64 (*)(void* context, AmUInt64 offset, AmUInt64 frames);

        const AudioBuffer* buffer = nullptr; ///< In memory: the whole sound. Streamed: the decode target.
        DecodeFunction decode = nullptr;     ///< Null for in-memory sounds.
        void* context = nullptr;
        AmUInt64 decodeCapacity = 0;         ///< Frames @c buffer holds, for streams.
        AmUInt64 length = 0;                 ///< Sound length in frames.
        AmUInt16 channels = 0;
        AmUInt32 sampleRate = 0;

        [[nodiscard]] bool IsStreamed() const
        {
            return decode != nullptr;
        }

        [[nodiscard]] bool IsValid() const
        {
            return buffer != nullptr && length > 0 && channels > 0 && sampleRate > 0 && (!IsStreamed() || decodeCapacity > 0);
        }
    };

    /**
     * @brief Builds the voice source of a loaded sound.
     */
    VoiceSource MakeVoiceSource(SoundData* data);

    struct SourceReadReport
    {
        AmUInt32 wraps = 0;           ///< Loop seams crossed during the read.
        AmUInt64 firstWrapOffset = 0; ///< Offset in the read of the first frame after the first seam.
        bool ended = false;           ///< The end of a non-looping source was reached during this read.
        AmUInt64 endOffset = 0;       ///< Offset in the read of the first frame past the end.
        bool starved = false;         ///< A stream returned fewer frames than asked before its end.
    };

    /**
     * @brief Reads a source region from an exact integer cursor, wrapping at the loop point inside a read.
     *
     * Audio thread. Never allocates.
     */
    class SourceReader
    {
    public:
        static constexpr AmUInt32 kInfiniteWraps = std::numeric_limits<AmUInt32>::max();

        /**
         * @brief Binds a source region. @p regionEnd of 0 means the source length. @p loopCount is the total number of
         * plays when looping; 0 loops forever.
         */
        void Initialize(const VoiceSource& source, AmUInt64 regionStart, AmUInt64 regionEnd, bool loop, AmUInt32 loopCount);

        /**
         * @brief Fills frames [@p offset, @p offset + @p frames) of every channel of @p out. Frames past a non-looping end
         * are zero.
         *
         * @return The number of source frames delivered.
         */
        AmUInt64 Read(AudioBuffer& out, AmUInt64 offset, AmUInt64 frames, SourceReadReport& report);

        /**
         * @brief Moves the cursor, clamped into the region, and clears the ended state. Keeps the remaining loop count.
         */
        void Seek(AmUInt64 frame);

        /**
         * @brief Returns the position @p frames frames before the cursor, wrapped into the loop region when looping,
         * clamped at the region start otherwise.
         */
        [[nodiscard]] AmUInt64 Rewind(AmUInt64 frames) const;

        [[nodiscard]] AmUInt64 GetCursor() const
        {
            return _cursor;
        }

        [[nodiscard]] bool IsEnded() const
        {
            return _ended;
        }

        [[nodiscard]] bool IsLooping() const
        {
            return _loop;
        }

        [[nodiscard]] AmUInt64 GetRegionStart() const
        {
            return _start;
        }

        [[nodiscard]] AmUInt64 GetRegionEnd() const
        {
            return _end;
        }

    private:
        AmUInt64 CopyFrames(AudioBuffer& out, AmUInt64 offset, AmUInt64 frames);

        VoiceSource _source;
        AmUInt64 _start = 0;
        AmUInt64 _end = 0;
        AmUInt64 _cursor = 0;
        bool _loop = false;
        AmUInt32 _remainingWraps = 0;
        bool _ended = true;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_VOICE_SOURCE_READER_H
