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

#ifndef _AM_IMPLEMENTATION_MIXER_STREAMING_STREAMING_THREAD_H
#define _AM_IMPLEMENTATION_MIXER_STREAMING_STREAMING_THREAD_H

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Core/Codec.h>
#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Thread.h>

#include <Mixer/Streaming/StreamSource.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Settings of the streaming thread.
     */
    struct StreamingSettings
    {
        /// Decoded audio kept ahead of each streamed reader, in milliseconds of source audio.
        AmTime bufferMs = 500.0;

        /// Decoded audio required before a streamed sound starts or a seek takes effect, in milliseconds.
        AmTime prefillMs = 60.0;

        /// Whether sources are filled by their consumer instead of the worker.
        bool synchronous = false;

        /// The maximum frames decoded per decoder call.
        AmUInt64 chunkFrames = 4096;
    };

    /**
     * @brief Owns streamed sources and decodes them ahead of their readers on a worker thread.
     *
     * Threading: Create(), CollectRetired(), Start(), Stop() and WakeUp() are game thread only. Retire() may be called
     * from any game-side thread, including while Stop() is joining the worker: the source is then queued and freed
     * after the join. Retire() must not run concurrently with the destructor. The worker is the only producer of the
     * registered sources. A retired source is freed only after the worker acknowledged it (or immediately when the
     * worker is not running and not being joined), so the worker never touches freed memory.
     */
    class StreamingThread
    {
    public:
        /**
         * @brief Creates a streaming thread. The worker is not started.
         */
        explicit StreamingThread(const StreamingSettings& settings);

        /**
         * @brief Stops the worker and frees every remaining source.
         */
        ~StreamingThread();

        StreamingThread(const StreamingThread&) = delete;
        StreamingThread& operator=(const StreamingThread&) = delete;

        /**
         * @brief Starts the worker. Game thread. No-op in synchronous mode or when already running.
         */
        void Start();

        /**
         * @brief Stops the worker and joins it. Game thread. Idempotent.
         *
         * Blocks until the worker returns from any decoder call in progress.
         */
        void Stop();

        /**
         * @brief Whether the worker is running.
         */
        [[nodiscard]] bool IsRunning() const;

        /**
         * @brief Creates and registers a stream source.
         *
         * In synchronous mode, or when the worker is not running, the source is filled inline to the prefill
         * threshold before returning. Game thread.
         *
         * @param decoder The decoder, closed when the source is freed.
         * @param plan The play plan.
         * @param channels The channel count.
         * @param sampleRate The sample rate.
         *
         * @return The source, owned by this object until Retire().
         */
        StreamSource* Create(std::shared_ptr<Codec::Decoder> decoder, const StreamPlan& plan, AmUInt16 channels, AmUInt32 sampleRate);

        /**
         * @brief Whether a source holds at least the prefill, or is completely filled.
         */
        [[nodiscard]] bool IsReady(const StreamSource* source) const;

        /**
         * @brief Unregisters a source and frees it once the worker acknowledged it. Game thread.
         *
         * The caller guarantees the audio thread no longer reaches the source. When the worker is not running the
         * source is freed immediately.
         */
        void Retire(StreamSource* source);

        /**
         * @brief Frees the retired sources the worker acknowledged. Game thread.
         */
        void CollectRetired();

        /**
         * @brief Fills a source on the calling thread. Synchronous mode only.
         */
        void FillInline(StreamSource* source, AmUInt64 frames);

        /**
         * @brief Whether sources are filled by their consumer.
         */
        [[nodiscard]] bool IsSynchronous() const;

        /**
         * @brief Gets the prefill threshold in frames at a sample rate.
         */
        [[nodiscard]] AmUInt64 GetPrefillFrames(AmUInt32 sampleRate) const;

        /**
         * @brief Counts a stream underrun. Any thread.
         */
        void RecordUnderrun();

        /**
         * @brief Gets the number of recorded underruns. Any thread.
         */
        [[nodiscard]] AmUInt64 GetUnderrunCount() const;

        /**
         * @brief Wakes the worker up. Game thread.
         */
        void WakeUp();

    private:
        struct Entry
        {
            StreamSource* source = nullptr;
            AmUInt64 lastRelease = 0;
            AmReal64 rate = 0.0;
        };

        struct Candidate
        {
            StreamSource* source = nullptr;
            AmReal64 timeLeft = 0.0;
        };

        static void Run(AmVoidPtr param);

        void Pass();
        bool UpdateRates(std::vector<Entry>& entries, AmReal64 elapsedSeconds);
        bool HasPendingWork();
        static void Free(StreamSource* source);

        StreamingSettings _settings;
        AmThreadHandle _thread = nullptr;
        std::atomic<bool> _running{ false };
        bool _stopping = false; ///< Set under _mutex while Stop() joins the worker.

        std::mutex _mutex;
        std::condition_variable _wake;
        bool _wakePending = false;
        std::vector<Entry> _sources; // guarded by _mutex
        std::vector<StreamSource*> _retireRequests; // guarded by _mutex
        std::vector<StreamSource*> _acknowledged; // guarded by _mutex

        // Worker only.
        std::vector<Entry> _active;
        std::vector<Candidate> _candidates;
        AmInt64 _lastPassTicks = 0;

        std::atomic<AmUInt64> _underruns{ 0 };
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_STREAMING_STREAMING_THREAD_H
