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

#include <algorithm>

#include <Mixer/Streaming/StreamRing.h>

namespace SparkyStudios::Audio::Amplitude
{
    StreamRing::StreamRing(AmUInt64 capacityFrames, AmUInt16 channels)
        : _storage(capacityFrames, channels)
        , _capacity(capacityFrames)
    {}

    AmUInt64 StreamRing::GetCapacity() const
    {
        return _capacity;
    }

    AmUInt16 StreamRing::GetChannelCount() const
    {
        return static_cast<AmUInt16>(_storage.GetChannelCount());
    }

    AmUInt64 StreamRing::GetWritableFrames() const
    {
        const AmUInt64 used = _write.load(std::memory_order_relaxed) - _release.load(std::memory_order_acquire);
        return _capacity - used;
    }

    AmUInt64 StreamRing::Write(const AudioBuffer& source, AmUInt64 sourceOffset, AmUInt64 frames)
    {
        const AmUInt64 count = std::min(frames, GetWritableFrames());
        if (count == 0)
            return 0;

        const AmUInt64 index = _write.load(std::memory_order_relaxed);
        CopyIn(source, sourceOffset, index, count);
        _write.store(index + count, std::memory_order_release);
        return count;
    }

    void StreamRing::MarkEnd()
    {
        _end.store(_write.load(std::memory_order_relaxed), std::memory_order_release);
    }

    void StreamRing::MarkFailed()
    {
        _failed.store(true, std::memory_order_release);
    }

    AmUInt64 StreamRing::GetWriteIndex() const
    {
        return _write.load(std::memory_order_acquire);
    }

    AmUInt64 StreamRing::GetReleaseIndex() const
    {
        return _release.load(std::memory_order_acquire);
    }

    AmUInt64 StreamRing::Read(AmUInt64 index, AudioBuffer& out, AmUInt64 outOffset, AmUInt64 frames) const
    {
        const AmUInt64 write = _write.load(std::memory_order_acquire);
        const AmUInt64 release = _release.load(std::memory_order_relaxed);
        if (index < release || index >= write)
            return 0;

        const AmUInt64 count = std::min(frames, write - index);
        CopyOut(index, out, outOffset, count);
        return count;
    }

    void StreamRing::Release(AmUInt64 index)
    {
        const AmUInt64 current = _release.load(std::memory_order_relaxed);
        if (index > current)
            _release.store(std::min(index, _write.load(std::memory_order_acquire)), std::memory_order_release);
    }

    AmUInt64 StreamRing::GetEndIndex() const
    {
        return _end.load(std::memory_order_acquire);
    }

    bool StreamRing::IsFailed() const
    {
        return _failed.load(std::memory_order_acquire);
    }

    void StreamRing::CopyIn(const AudioBuffer& source, AmUInt64 sourceOffset, AmUInt64 index, AmUInt64 frames)
    {
        const AmUInt64 start = index % _capacity;
        const AmUInt64 first = std::min(frames, _capacity - start);
        for (AmSize c = 0; c < _storage.GetChannelCount(); ++c)
        {
            std::copy_n(source[c].begin() + sourceOffset, first, _storage[c].begin() + start);
            std::copy_n(source[c].begin() + sourceOffset + first, frames - first, _storage[c].begin());
        }
    }

    void StreamRing::CopyOut(AmUInt64 index, AudioBuffer& out, AmUInt64 outOffset, AmUInt64 frames) const
    {
        const AmUInt64 start = index % _capacity;
        const AmUInt64 first = std::min(frames, _capacity - start);
        for (AmSize c = 0; c < _storage.GetChannelCount(); ++c)
        {
            std::copy_n(_storage[c].begin() + start, first, out[c].begin() + outOffset);
            std::copy_n(_storage[c].begin(), frames - first, out[c].begin() + outOffset + first);
        }
    }
} // namespace SparkyStudios::Audio::Amplitude
