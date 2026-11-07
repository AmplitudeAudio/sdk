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

#ifndef _AM_TESTS_MIXER_STREAMING_STREAM_TEST_DECODER_H
#define _AM_TESTS_MIXER_STREAMING_STREAM_TEST_DECODER_H

#include <algorithm>
#include <atomic>
#include <limits>
#include <mutex>
#include <set>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

namespace SparkyStudios::Audio::Amplitude::Tests
{
    /**
     * @brief A decoder serving frames from memory: `frame + 1` on channel 0, `-(frame + 1)` on channel 1.
     */
    class StreamTestDecoder final : public Codec::Decoder
    {
    public:
        StreamTestDecoder(AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate = 48000)
            : Decoder(nullptr)
            , _frames(frames)
            , _channels(channels)
        {
            m_format.SetAll(sampleRate, channels, 32, frames, channels * sizeof(AmReal32), eAudioSampleFormat_Float32);
        }

        static AmReal32 ValueAt(AmUInt64 frame, AmUInt16 channel)
        {
            const auto value = static_cast<AmReal32>(frame + 1);
            return channel == 0 ? value : -value;
        }

        bool Open(std::shared_ptr<File>) override
        {
            return true;
        }

        bool Close() override
        {
            _closed.store(true);
            return true;
        }

        AmUInt64 Load(AudioBuffer* out) override
        {
            return Stream(out, 0, 0, _frames);
        }

        AmUInt64 Stream(AudioBuffer* out, AmUInt64 bufferOffset, AmUInt64 seekOffset, AmUInt64 length) override
        {
            Record();
            if (_delayMs > 0)
                Thread::Sleep(_delayMs);

            if (seekOffset >= _frames || seekOffset >= _failAt)
                return 0;

            const AmUInt64 count = std::min({ length, _frames - seekOffset, _failAt - seekOffset });
            for (AmUInt16 c = 0; c < _channels; ++c)
                for (AmUInt64 i = 0; i < count; ++i)
                    (*out)[c][bufferOffset + i] = ValueAt(seekOffset + i, c);

            _streamCalls.fetch_add(1);
            return count;
        }

        bool Seek(AmUInt64 offset) override
        {
            Record();
            return offset <= _frames;
        }

        void FailAt(AmUInt64 frame)
        {
            _failAt = frame;
        }

        void SetDelay(AmInt32 milliseconds)
        {
            _delayMs = milliseconds;
        }

        [[nodiscard]] bool IsClosed() const
        {
            return _closed.load();
        }

        [[nodiscard]] AmUInt64 GetStreamCalls() const
        {
            return _streamCalls.load();
        }

        [[nodiscard]] std::set<AmUInt64> GetCallingThreads() const
        {
            std::lock_guard lock(_mutex);
            return _threads;
        }

    private:
        void Record()
        {
            std::lock_guard lock(_mutex);
            _threads.insert(Thread::GetCurrentThreadId());
        }

        AmUInt64 _frames;
        AmUInt16 _channels;
        AmUInt64 _failAt = std::numeric_limits<AmUInt64>::max();
        AmInt32 _delayMs = 0;
        std::atomic<bool> _closed{ false };
        std::atomic<AmUInt64> _streamCalls{ 0 };
        mutable std::mutex _mutex;
        std::set<AmUInt64> _threads;
    };
} // namespace SparkyStudios::Audio::Amplitude::Tests

#endif // _AM_TESTS_MIXER_STREAMING_STREAM_TEST_DECODER_H
