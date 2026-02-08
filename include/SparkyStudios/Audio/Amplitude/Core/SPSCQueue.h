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

#ifndef _AM_CORE_SPSCQUEUE_H
#define _AM_CORE_SPSCQUEUE_H

#include <atomic>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Lock-free single-producer single-consumer queue.
     *
     * Provides wait-free enqueue (producer) and dequeue (consumer) operations
     * for exactly one producer thread and one consumer thread. No locks are used;
     * synchronization relies solely on atomic head/tail indices with acquire/release
     * memory ordering.
     *
     * The head and tail indices are aligned to separate cache lines to prevent
     * false sharing between the producer and consumer.
     *
     * @tparam T The element type.
     * @tparam Capacity Queue capacity. Must be a power of 2. Default is 256.
     *
     * @ingroup core
     */
    template<typename T, AmSize Capacity = 256>
    class SPSCQueue
    {
        static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2");
        static_assert(Capacity > 0, "Capacity must be greater than 0");

    public:
        /**
         * @brief Constructs an empty queue.
         */
        SPSCQueue()
            : _head(0)
            , _tail(0)
        {}

        ~SPSCQueue() = default;

        SPSCQueue(const SPSCQueue&) = delete;
        SPSCQueue& operator=(const SPSCQueue&) = delete;

        /**
         * @brief Attempts to enqueue an item by copy.
         *
         * @param[in] item The item to enqueue.
         * @return @c true if enqueued, @c false if the queue is full.
         *
         * @note Must only be called from the producer thread.
         */
        bool TryEnqueue(const T& item)
        {
            const AmSize tail = _tail.load(std::memory_order_relaxed);
            const AmSize nextTail = (tail + 1) & kMask;

            if (nextTail == _head.load(std::memory_order_acquire))
                return false;

            _buffer[tail] = item;
            _tail.store(nextTail, std::memory_order_release);
            return true;
        }

        /**
         * @brief Attempts to enqueue an item by move.
         *
         * @param[in] item The item to enqueue.
         * @return @c true if enqueued, @c false if the queue is full.
         *
         * @note Must only be called from the producer thread.
         */
        bool TryEnqueue(T&& item)
        {
            const AmSize tail = _tail.load(std::memory_order_relaxed);
            const AmSize nextTail = (tail + 1) & kMask;

            if (nextTail == _head.load(std::memory_order_acquire))
                return false;

            _buffer[tail] = std::move(item);
            _tail.store(nextTail, std::memory_order_release);
            return true;
        }

        /**
         * @brief Attempts to dequeue an item.
         *
         * @param[out] item Receives the dequeued item on success.
         * @return @c true if an item was dequeued, @c false if the queue is empty.
         *
         * @note Must only be called from the consumer thread.
         */
        bool TryDequeue(T& item)
        {
            const AmSize head = _head.load(std::memory_order_relaxed);

            if (head == _tail.load(std::memory_order_acquire))
                return false;

            item = std::move(_buffer[head]);
            _head.store((head + 1) & kMask, std::memory_order_release);
            return true;
        }

        /**
         * @brief Checks if the queue is empty.
         *
         * @return @c true if empty.
         *
         * @note The result is a snapshot and may be stale immediately.
         */
        [[nodiscard]] bool IsEmpty() const
        {
            return _head.load(std::memory_order_relaxed) == _tail.load(std::memory_order_relaxed);
        }

        /**
         * @brief Gets the queue capacity (maximum storable items).
         *
         * One slot is reserved internally for full/empty disambiguation, so the
         * usable capacity is @c Capacity - 1.
         *
         * @return The usable queue capacity.
         */
        [[nodiscard]] static constexpr AmSize GetCapacity()
        {
            return Capacity - 1;
        }

    private:
        static constexpr AmSize kCacheLineSize = 64;
        static constexpr AmSize kMask = Capacity - 1;

        alignas(kCacheLineSize) std::atomic<AmSize> _head;
        alignas(kCacheLineSize) std::atomic<AmSize> _tail;
        alignas(kCacheLineSize) T _buffer[Capacity];
    };

} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_SPSCQUEUE_H
