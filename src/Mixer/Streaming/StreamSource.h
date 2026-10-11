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

#ifndef _AM_IMPLEMENTATION_MIXER_STREAMING_STREAM_SOURCE_H
#define _AM_IMPLEMENTATION_MIXER_STREAMING_STREAM_SOURCE_H

#include <atomic>
#include <memory>

#include <SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h>
#include <SparkyStudios/Audio/Amplitude/Core/Codec.h>
#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <Mixer/Streaming/StreamRing.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Describes the frames a stream ring will hold, in play order.
     */
    struct StreamPlan
    {
        /// First source frame of the playable region.
        AmUInt64 regionStart = 0;

        /// One past the last source frame of the playable region.
        AmUInt64 regionEnd = 0;

        /// Whether the region loops.
        bool loop = false;

        /// Wraps the producer may still make (SourceReader::kInfiniteWraps for forever).
        AmUInt32 remainingWraps = 0;

        /// The source frame at ring index 0.
        AmUInt64 start = 0;

        /// The source frame the reader starts at.
        AmUInt64 target = 0;

        /// The ring index of #target.
        AmUInt64 targetIndex = 0;
    };

    /**
     * @brief Plans a ring so that @p target sits at ring index @p prerollOut.
     *
     * The preroll is read from before @p target, or from the loop tail, like ResampleStream::Seek.
     *
     * @param regionStart First source frame of the region.
     * @param regionEnd One past the last source frame of the region.
     * @param loop Whether the region loops.
     * @param remainingWraps Wraps still allowed (SourceReader::kInfiniteWraps for forever).
     * @param target The frame the reader starts at.
     * @param prerollFrames The preroll length wanted.
     * @param prerollOut Receives the preroll length actually planned.
     *
     * @return The plan.
     */
    StreamPlan PlanStream(
        AmUInt64 regionStart,
        AmUInt64 regionEnd,
        bool loop,
        AmUInt32 remainingWraps,
        AmUInt64 target,
        AmUInt64 prerollFrames,
        AmUInt64& prerollOut);

    /**
     * @brief Decodes a stream ahead of its reader, along a play plan, into a StreamRing.
     */
    class StreamSource
    {
    public:
        /**
         * @brief Creates a stream source.
         *
         * @param decoder The decoder to read frames from. Closed when the source is destroyed.
         * @param plan The play plan.
         * @param capacityFrames The ring capacity in frames.
         * @param channels The channel count.
         * @param chunkFrames The maximum frames decoded per decoder call.
         * @param sampleRate The sample rate of the decoded frames.
         * @param synchronous Whether the source is filled by its consumer instead of the worker.
         */
        StreamSource(
            std::shared_ptr<Codec::Decoder> decoder,
            const StreamPlan& plan,
            AmUInt64 capacityFrames,
            AmUInt16 channels,
            AmUInt64 chunkFrames,
            AmUInt32 sampleRate,
            bool synchronous);

        /**
         * @brief Closes the decoder.
         */
        ~StreamSource();

        StreamSource(const StreamSource&) = delete;
        StreamSource& operator=(const StreamSource&) = delete;

        /**
         * @brief Decodes up to @p maxFrames frames along the plan, bounded by the ring's writable frames.
         *
         * Producer only: every call must come from the same thread.
         *
         * @param maxFrames The maximum frames to write.
         *
         * @return The frames written. Marks the ring ended or failed when the plan is exhausted or the decoder fails.
         */
        AmUInt64 Fill(AmUInt64 maxFrames);

        /**
         * @brief Whether nothing more will be written (the end is marked or the stream failed).
         *
         * Any thread.
         */
        [[nodiscard]] bool IsFilled() const;

        /**
         * @brief Finds the ring index of a source frame.
         *
         * Returns the plan's target index for the target frame; otherwise the first occurrence after the plan start,
         * following one wrap. Any thread.
         *
         * @param sourceFrame The source frame.
         * @param index Receives the ring index.
         *
         * @return @c false if the frame is outside the planned region.
         */
        [[nodiscard]] bool IndexOf(AmUInt64 sourceFrame, AmUInt64& index) const;

        /**
         * @brief Gets the source frame stored at a ring index. Any thread.
         */
        [[nodiscard]] AmUInt64 SourceFrameOf(AmUInt64 index) const;

        /**
         * @brief Gets the ring.
         */
        [[nodiscard]] StreamRing& GetRing();

        /**
         * @brief Gets the ring.
         */
        [[nodiscard]] const StreamRing& GetRing() const;

        /**
         * @brief Gets the play plan. Any thread.
         */
        [[nodiscard]] const StreamPlan& GetPlan() const;

        /**
         * @brief Gets the frames written and not yet released. Any thread.
         */
        [[nodiscard]] AmUInt64 GetBufferedFrames() const;

        /**
         * @brief Gets the sample rate of the decoded frames. Any thread.
         */
        [[nodiscard]] AmUInt32 GetSampleRate() const;

        /**
         * @brief Whether the consumer fills this source itself. Any thread.
         */
        [[nodiscard]] bool IsSynchronous() const;

        /**
         * @brief Suspends or resumes this source: the worker skips suspended sources. Any thread.
         */
        void SetSuspended(bool suspended);

        /**
         * @brief Whether this source is suspended. Any thread.
         */
        [[nodiscard]] bool IsSuspended() const;

    private:
        void AssertProducerThread();

        std::shared_ptr<Codec::Decoder> _decoder;
        StreamPlan _plan;
        StreamRing _ring;
        AudioBuffer _chunk;
        AmUInt64 _chunkFrames;
        AmUInt64 _next;
        AmUInt32 _wrapsLeft;
        AmUInt32 _sampleRate;
        bool _synchronous;
        std::atomic<bool> _suspended{ false };
#if !defined(NDEBUG)
        std::atomic<AmUInt64> _producerThread{ 0 };
#endif
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_STREAMING_STREAM_SOURCE_H
