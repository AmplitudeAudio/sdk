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

#ifndef _AM_IMPLEMENTATION_UTILS_TRIPLE_BUFFER_H
#define _AM_IMPLEMENTATION_UTILS_TRIPLE_BUFFER_H

#include <array>
#include <atomic>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    // The hand-off must not fall back to a lock: the audio thread can never block.
    static_assert(std::atomic<AmUInt32>::is_always_lock_free, "The hand-off must be lock-free.");

    /**
     * @brief A lock-free triple buffer: one thread publishes values, one thread reads the latest of them.
     *
     * Three buffers rotate between three owners: the publisher's private buffer, a "middle" slot, and the reader's
     * buffer. Publishing swaps the private buffer into the middle slot, and reading swaps the reader's buffer for the
     * middle one when it holds something new. Each hand-off is a single atomic exchange, so neither side ever waits,
     * allocates or locks.
     *
     * Unlike a double buffer, the buffer the reader holds is never written by the publisher, however many values are
     * published while the reader still holds it. That is what makes it safe to read a container that the publisher
     * reallocates on every publish.
     *
     * A published value that the reader never saw is dropped when a newer one replaces it: the reader is always given
     * the newest value it observed at exchange time, and a publish that races with a @c Read call is picked up by the
     * next one.
     *
     * @tparam T The published type. Must be default-constructible. The publisher owns the storage T manages.
     *
     * @note Exactly one thread may call @c Write and @c Publish, and exactly one thread may call @c Read. The two
     * threads may be the same one. No other use is safe: the buffers are not synchronized beyond the hand-off
     * exchanges, and the object must outlive every reference returned by @c Read.
     */
    template<typename T>
    class TripleBuffer
    {
    public:
        TripleBuffer() = default;

        TripleBuffer(const TripleBuffer&) = delete;
        TripleBuffer& operator=(const TripleBuffer&) = delete;

        // Moving a live triple buffer would be meaningless: the publisher and the reader own indices into it.
        // Holders that are themselves moved must default-construct this member, like a mutex.
        TripleBuffer(TripleBuffer&&) = delete;
        TripleBuffer& operator=(TripleBuffer&&) = delete;

        /**
         * @brief Gets the buffer to fill before calling @c Publish.
         *
         * Only the publisher can reach it. It may still hold data from an older publish, so the publisher must
         * overwrite it entirely and must not assume it is empty.
         */
        [[nodiscard]] T& Write()
        {
            return _buffers[_writeIndex];
        }

        /**
         * @brief Hands the buffer returned by @c Write over to the reader.
         *
         * After this, @c Write returns another buffer: do not keep references to the previous one.
         */
        void Publish()
        {
            // The acquire half orders the writes the other thread made to the buffer we are about to take; the release
            // half hands our writes to whoever takes this buffer next.
            const AmUInt32 previous = _middle.exchange(_writeIndex | kFreshBit, std::memory_order_acq_rel);
            _writeIndex = previous & kIndexMask;
        }

        /**
         * @brief Gets the latest published value.
         *
         * @return The reader's buffer, which is unchanged until the next call to @c Read.
         */
        [[nodiscard]] const T& Read()
        {
            // Optimization only: skip the exchange when nothing was published since the last call. The load may be
            // stale and miss a publish that raced with it, so Read() can lag one call behind; correctness does not
            // depend on it, because the exchange always returns a buffer neither side owns.
            if ((_middle.load(std::memory_order_relaxed) & kFreshBit) != 0)
            {
                // The mirror image of the publish hand-off: the release half ends our reads of the buffer we are
                // handing back, the acquire half picks up the publisher's writes to the buffer we are taking.
                const AmUInt32 previous = _middle.exchange(_readIndex, std::memory_order_acq_rel);
                _readIndex = previous & kIndexMask;
            }

            return _buffers[_readIndex];
        }

    private:
        static constexpr AmUInt32 kBufferCount = 3;
        static constexpr AmUInt32 kIndexBits = 2; // One bit above the index carries kFreshBit.
        static constexpr AmUInt32 kIndexMask = (1u << kIndexBits) - 1;
        static constexpr AmUInt32 kFreshBit = 1u << kIndexBits;

        static_assert(kBufferCount <= kIndexMask + 1, "The index must encode a buffer index.");

        std::array<T, kBufferCount> _buffers{};

        // The index of the middle buffer, with kFreshBit set when it holds a value the reader has not taken.
        std::atomic<AmUInt32> _middle{ 1 };

        // Owned by the publisher.
        AmUInt32 _writeIndex = 0;

        // Owned by the reader.
        AmUInt32 _readIndex = 2;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_UTILS_TRIPLE_BUFFER_H