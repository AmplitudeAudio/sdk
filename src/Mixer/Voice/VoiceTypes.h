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

#ifndef _AM_IMPLEMENTATION_MIXER_VOICE_VOICE_TYPES_H
#define _AM_IMPLEMENTATION_MIXER_VOICE_VOICE_TYPES_H

#include <array>
#include <cmath>
#include <limits>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/SPSCQueue.h>

namespace SparkyStudios::Audio::Amplitude
{
    class StreamSource;

    /// Target frame meaning "the first frame of the next block the voice renders".
    constexpr AmUInt64 kVoiceAsap = std::numeric_limits<AmUInt64>::max();

    /// Floor for transport fades, in milliseconds. A shorter fade, including 0, becomes a raised-cosine fade of this length.
    constexpr AmTime kDeclickFade = 5.0;

    /// Fade used when a voice is released for virtualization or promoted back, in milliseconds.
    constexpr AmTime kStealFade = 10.0;

    /// Length of the equal-power crossfade that makes a seek click-free, in milliseconds.
    constexpr AmTime kSeekCrossfade = 8.0;

    /// Capacity of the engine-wide voice command and event queues.
    constexpr AmSize kVoiceQueueCapacity = 4096;

    /// State of a voice, owned by the audio thread.
    enum class eVoiceState : AmUInt8
    {
        Idle = 0,
        Scheduled,
        Playing,
        FadingOut,
        Paused,
        Ending,
        Finished,
    };

    /// Transport command sent by the game thread to a voice.
    enum class eVoiceCommandKind : AmUInt8
    {
        Stop = 0,
        Pause,
        Resume,
        Seek,
        Release,
    };

    /// What a fade-out ends in.
    enum class eVoiceFadeTarget : AmUInt8
    {
        None = 0,
        Paused,
        Stopped,
        Released,
    };

    /// Event sent by a voice to the game thread. The order is the publication order within one block.
    enum class eVoiceEventKind : AmUInt8
    {
        Started = 0,
        FadedOut,
        Looped,
        Underrun,      ///< A streamed read ran short; the missing frames were silence.

        /**
         * @brief The voice's current stream changed. @c VoiceEvent::stream is the stream the voice reads after the event.
         *
         * Informational: it says nothing about any other stream (the replaced one is reported by @c StreamDropped).
         * Posted when a crossfade ends and promotes its incoming stream, or when a seek binds a stream directly (paused,
         * scheduled, source already finished). Never posted when the current stream is unchanged. Coalesced: one slot
         * per voice that always holds the newest current stream. The pointer is identity only: the consumer must never
         * dereference it on the audio thread.
         *
         * Consumer contract: retire X on @c StreamDropped(X); adopt on @c StreamSwapped(X). Streams still held when the
         * voice ends (@c Finished) are retired by the consumer when the layer is destroyed.
         */
        StreamSwapped,

        /**
         * @brief @c VoiceEvent::stream is no longer read by this voice and never will be again: the consumer may retire
         * it (after the usual deferred/ack retire). Identity only, never dereferenced by the consumer.
         *
         * Posted for every stream that stops being read: a stream seek that was never adopted (Idle/Finished voice,
         * evicted from a full command queue, left unconsumed, replaced before the crossfade midpoint, superseded by a
         * later seek of the same block) and the previous current stream whenever the current one changes. Never posted
         * for the current stream, for the stream being adopted, or for a stream seek that re-sends one of them. Each
         * stream is reported once per time it stops being read. Not coalesced: published in order.
         *
         * The mailbox holds @c VoiceEventMailbox::kMaxDroppedStreams of them; on overflow a single event with a null
         * @c stream is published instead, meaning "a drop was lost": the consumer must keep every stream it holds for
         * this voice alive until the voice ends and must never retire anything because of it.
         */
        StreamDropped,
        Ended,
        Error, ///< Ordered before Finished: the game side must hear it while the layer still exists.
        Finished,
        Count,
    };

    struct VoiceCommand
    {
        AmUInt32 layer = 0;                            ///< Mixer layer handle.
        AmUInt32 id = 0;                               ///< Owner id checked against the layer.
        eVoiceCommandKind kind = eVoiceCommandKind::Stop;
        AmUInt64 frame = kVoiceAsap;                   ///< Audio-clock frame the command applies at.
        AmTime duration = 0.0;                         ///< Fade duration in milliseconds.
        AmUInt64 position = 0;                         ///< Source frame, for seeks.
        StreamSource* stream = nullptr;                ///< For seeks: the pending stream to bind. Null for in-memory voices.
    };

    struct VoiceEvent
    {
        AmUInt32 layer = 0;
        AmUInt32 id = 0;
        eVoiceEventKind kind = eVoiceEventKind::Started;
        eVoiceFadeTarget target = eVoiceFadeTarget::None;
        AmUInt64 frame = 0;                            ///< Audio-clock frame the event happened at.
        AmUInt64 sourcePosition = 0;                   ///< Source frame, for FadedOut{Paused, Released}.
        AmUInt32 count = 1;                            ///< Number of loops, for Looped.
        const StreamSource* stream = nullptr;          ///< For StreamSwapped / StreamDropped: identity only, never dereferenced.
    };

    using VoiceCommandQueue = SPSCQueue<VoiceCommand, kVoiceQueueCapacity>;
    using VoiceEventQueue = SPSCQueue<VoiceEvent, kVoiceQueueCapacity>;

    /**
     * @brief Converts a duration in milliseconds to a frame count at the given rate, rounded to nearest.
     */
    AM_INLINE AmUInt64 MillisecondsToFrames(AmTime ms, AmUInt32 rate)
    {
        return ms <= 0.0 ? 0 : static_cast<AmUInt64>(std::llround(ms * static_cast<AmTime>(rate) / kAmSecond));
    }

    /**
     * @brief Events of one voice waiting to be published: at most one per kind, loops coalesced.
     *
     * @c StreamDropped is the exception: dropped streams are kept in a fixed list (never coalesced) so none is lost,
     * with a documented overflow fallback. A later @c StreamSwapped overwrites the pending one, so the slot always
     * holds the newest current stream, also when the queue was full and the earlier one is still pending.
     *
     * Audio thread only. Never allocates.
     */
    class VoiceEventMailbox
    {
    public:
        /**
         * @brief Adds an event. A second @c Looped event adds its count and keeps the first frame; other kinds replace.
         */
        void Post(const VoiceEvent& event);

        /**
         * @brief Sets the source position of a pending event of the given kind, if any.
         */
        void Amend(eVoiceEventKind kind, AmUInt64 sourcePosition);

        /**
         * @brief Moves pending events to @p queue in kind order, stopping at the first that does not fit.
         */
        void Publish(VoiceEventQueue& queue);

        /// Dropped streams kept before falling back to a single null-stream overflow event.
        static constexpr AmSize kMaxDroppedStreams = 64;

        [[nodiscard]] bool IsEmpty() const;

        void Clear();

    private:
        static constexpr AmSize kKinds = static_cast<AmSize>(eVoiceEventKind::Count);

        bool PublishDropped(VoiceEventQueue& queue);

        std::array<VoiceEvent, kKinds> _events{};
        std::array<bool, kKinds> _pending{};

        std::array<VoiceEvent, kMaxDroppedStreams> _dropped{};
        AmSize _droppedHead = 0;
        AmSize _droppedCount = 0;
        bool _overflow = false;
        VoiceEvent _overflowEvent{};
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_VOICE_VOICE_TYPES_H
