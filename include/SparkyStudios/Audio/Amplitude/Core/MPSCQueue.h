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

#ifndef _AM_CORE_MPSCQUEUE_H
#define _AM_CORE_MPSCQUEUE_H

#include <atomic>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Lock-free bounded multi-producer single-consumer queue.
     *
     * Based on Dmitry Vyukov's bounded MPMC queue, specialized for multiple
     * producers and a single consumer. Each slot carries a sequence counter
     * that coordinates producers via CAS on the tail index and lets the
     * single consumer advance the head without contention.
     *
     * The tail (producers) and head (consumer) are aligned to separate cache
     * lines to prevent false sharing.
     *
     * @tparam T The element type.
     * @tparam Capacity Queue capacity. Must be a power of 2. Default is 256.
     *
     * @ingroup core
     */
    template<typename T, AmSize Capacity = 256>
    class MPSCQueue
    {
        static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2");
        static_assert(Capacity > 0, "Capacity must be greater than 0");

    public:
        /**
         * @brief Constructs an empty queue.
         *
         * Initializes every slot's sequence to its index so that the first
         * Capacity enqueues can proceed without waiting.
         */
        MPSCQueue()
            : _tail(0)
            , _head(0)
        {
            for (AmSize i = 0; i < Capacity; ++i)
                _buffer[i].sequence.store(i, std::memory_order_relaxed);
        }

        ~MPSCQueue() = default;

        MPSCQueue(const MPSCQueue&) = delete;
        MPSCQueue& operator=(const MPSCQueue&) = delete;

        /**
         * @brief Attempts to enqueue an item by copy (multi-producer safe).
         *
         * @param[in] item The item to enqueue.
         * @return @c true if enqueued, @c false if the queue is full.
         */
        bool TryEnqueue(const T& item)
        {
            Cell* cell;
            AmSize pos = _tail.load(std::memory_order_relaxed);

            for (;;)
            {
                cell = &_buffer[pos & kMask];
                const AmSize seq = cell->sequence.load(std::memory_order_acquire);
                const auto diff = static_cast<std::ptrdiff_t>(seq) - static_cast<std::ptrdiff_t>(pos);

                if (diff == 0)
                {
                    // Slot is free — try to claim it
                    if (_tail.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed))
                        break;
                }
                else if (diff < 0)
                {
                    // Queue is full
                    return false;
                }
                else
                {
                    // Another producer claimed this slot, reload tail
                    pos = _tail.load(std::memory_order_relaxed);
                }
            }

            cell->data = item;
            cell->sequence.store(pos + 1, std::memory_order_release);
            return true;
        }

        /**
         * @brief Attempts to enqueue an item by move (multi-producer safe).
         *
         * @param[in] item The item to enqueue.
         * @return @c true if enqueued, @c false if the queue is full.
         */
        bool TryEnqueue(T&& item)
        {
            Cell* cell;
            AmSize pos = _tail.load(std::memory_order_relaxed);

            for (;;)
            {
                cell = &_buffer[pos & kMask];
                const AmSize seq = cell->sequence.load(std::memory_order_acquire);
                const auto diff = static_cast<std::ptrdiff_t>(seq) - static_cast<std::ptrdiff_t>(pos);

                if (diff == 0)
                {
                    if (_tail.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed))
                        break;
                }
                else if (diff < 0)
                {
                    return false;
                }
                else
                {
                    pos = _tail.load(std::memory_order_relaxed);
                }
            }

            cell->data = std::move(item);
            cell->sequence.store(pos + 1, std::memory_order_release);
            return true;
        }

        /**
         * @brief Attempts to dequeue an item.
         *
         * @param[out] item Receives the dequeued item on success.
         * @return @c true if an item was dequeued, @c false if the queue is empty.
         *
         * @note Must only be called from the single consumer thread.
         */
        bool TryDequeue(T& item)
        {
            Cell* cell = &_buffer[_head & kMask];
            const AmSize seq = cell->sequence.load(std::memory_order_acquire);
            const auto diff = static_cast<std::ptrdiff_t>(seq) - static_cast<std::ptrdiff_t>(_head + 1);

            if (diff < 0)
                return false; // Queue is empty

            item = std::move(cell->data);
            cell->sequence.store(_head + Capacity, std::memory_order_release);
            ++_head;
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
            const Cell& cell = _buffer[_head & kMask];
            const AmSize seq = cell.sequence.load(std::memory_order_acquire);
            const auto diff = static_cast<std::ptrdiff_t>(seq) - static_cast<std::ptrdiff_t>(_head + 1);
            return diff < 0;
        }

        /**
         * @brief Gets the queue capacity (maximum storable items).
         *
         * @return The queue capacity.
         */
        [[nodiscard]] static constexpr AmSize GetCapacity()
        {
            return Capacity;
        }

    private:
        static constexpr AmSize kCacheLineSize = 64;
        static constexpr AmSize kMask = Capacity - 1;

        struct Cell
        {
            std::atomic<AmSize> sequence;
            T data;
        };

        alignas(kCacheLineSize) Cell _buffer[Capacity];
        alignas(kCacheLineSize) std::atomic<AmSize> _tail;
        alignas(kCacheLineSize) AmSize _head; // Consumer-only, no atomic needed
    };

} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_MPSCQUEUE_H
