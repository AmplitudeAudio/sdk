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

#ifndef _AM_TESTS_MIXER_STREAMING_SLOW_COUNTING_DECODER_H
#define _AM_TESTS_MIXER_STREAMING_SLOW_COUNTING_DECODER_H

#include <atomic>
#include <chrono>
#include <memory>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

namespace SparkyStudios::Audio::Amplitude::Tests
{
    /**
     * @brief A decoder whose Stream() blocks for 200 ms, counting Close() calls and whether one came before Stop()
     * returned.
     */
    class SlowCountingDecoder final : public Codec::Decoder
    {
    public:
        static constexpr AmInt32 kStreamDelayMs = 200;

        SlowCountingDecoder(std::atomic<int>& closeCount, std::atomic<bool>& stopReturned, std::atomic<bool>& closedBeforeStop)
            : Decoder(nullptr)
            , _closeCount(closeCount)
            , _stopReturned(stopReturned)
            , _closedBeforeStop(closedBeforeStop)
        {
            m_format.SetAll(48000, 2, 32, 48000 * 10, 2 * sizeof(AmReal32), eAudioSampleFormat_Float32);
        }

        bool Open(std::shared_ptr<File>) override
        {
            return true;
        }

        bool Close() override
        {
            if (!_stopReturned.load())
                _closedBeforeStop.store(true);
            _closeCount.fetch_add(1);
            return true;
        }

        AmUInt64 Load(AudioBuffer*) override
        {
            return 0;
        }

        AmUInt64 Stream(AudioBuffer* out, AmUInt64 bufferOffset, AmUInt64, AmUInt64 length) override
        {
            _inStream.store(true, std::memory_order_release);
            Thread::Sleep(kStreamDelayMs);
            for (AmUInt16 c = 0; c < 2; ++c)
                for (AmUInt64 i = 0; i < length; ++i)
                    (*out)[c][bufferOffset + i] = 0.0f;
            return length;
        }

        bool Seek(AmUInt64) override
        {
            return true;
        }

        /**
         * @brief Waits until a thread entered Stream(), so the caller knows the worker is inside the slow read
         * instead of guessing it from a fixed sleep that a slow runner can outrun.
         *
         * @param[in] timeout How long to wait.
         *
         * @return @c true once Stream() was entered, @c false if @p timeout elapsed first.
         */
        [[nodiscard]] bool WaitUntilInStream(std::chrono::milliseconds timeout = std::chrono::seconds(2)) const
        {
            const auto deadline = std::chrono::steady_clock::now() + timeout;
            while (!_inStream.load(std::memory_order_acquire))
            {
                if (std::chrono::steady_clock::now() >= deadline)
                    return false;

                Thread::Sleep(1);
            }

            return true;
        }

    private:
        std::atomic<int>& _closeCount;
        std::atomic<bool>& _stopReturned;
        std::atomic<bool>& _closedBeforeStop;
        std::atomic<bool> _inStream{ false };
    };
} // namespace SparkyStudios::Audio::Amplitude::Tests

#endif // _AM_TESTS_MIXER_STREAMING_SLOW_COUNTING_DECODER_H
