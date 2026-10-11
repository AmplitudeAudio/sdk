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

#ifndef _AM_IMPLEMENTATION_MIXER_STREAMING_STREAM_RING_H
#define _AM_IMPLEMENTATION_MIXER_STREAMING_STREAM_RING_H

#include <atomic>
#include <limits>

#include <SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h>
#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief A planar ring of decoded frames, written by one producer and read by one consumer.
     *
     * Frames are addressed by monotonic play indices: index 0 is the first frame ever written. The producer appends at
     * the write index; the consumer reads any index in [release, write) without consuming it, and releases space by
     * moving the release index forward. Storage is allocated once, on the constructing thread.
     *
     * Producer and consumer calls are lock-free and never allocate.
     */
    class StreamRing
    {
    public:
        static constexpr AmUInt64 kNoEnd = std::numeric_limits<AmUInt64>::max();

        StreamRing(AmUInt64 capacityFrames, AmUInt16 channels);

        [[nodiscard]] AmUInt64 GetCapacity() const;
        [[nodiscard]] AmUInt16 GetChannelCount() const;

        /// Producer: frames that can be written without overwriting unreleased frames.
        [[nodiscard]] AmUInt64 GetWritableFrames() const;

        /// Producer: appends up to @p frames frames of @p source starting at @p sourceOffset. Returns the frames written.
        AmUInt64 Write(const AudioBuffer& source, AmUInt64 sourceOffset, AmUInt64 frames);

        /// Producer: the stream ends at the current write index.
        void MarkEnd();

        /// Producer: the stream failed; no more frames will come.
        void MarkFailed();

        [[nodiscard]] AmUInt64 GetWriteIndex() const;
        [[nodiscard]] AmUInt64 GetReleaseIndex() const;

        /// Consumer: copies the frames of [index, index + frames) that exist into @p out. Returns the frames copied.
        AmUInt64 Read(AmUInt64 index, AudioBuffer& out, AmUInt64 outOffset, AmUInt64 frames) const;

        /// Consumer: frames before @p index may be overwritten. Never moves backwards.
        void Release(AmUInt64 index);

        [[nodiscard]] AmUInt64 GetEndIndex() const;
        [[nodiscard]] bool IsFailed() const;

    private:
        void CopyIn(const AudioBuffer& source, AmUInt64 sourceOffset, AmUInt64 index, AmUInt64 frames);
        void CopyOut(AmUInt64 index, AudioBuffer& out, AmUInt64 outOffset, AmUInt64 frames) const;

        AudioBuffer _storage;
        AmUInt64 _capacity;
        std::atomic<AmUInt64> _write{ 0 };
        std::atomic<AmUInt64> _release{ 0 };
        std::atomic<AmUInt64> _end{ kNoEnd };
        std::atomic<bool> _failed{ false };
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_STREAMING_STREAM_RING_H
