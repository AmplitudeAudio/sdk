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

#include <Mixer/SoundData.h>
#include <Mixer/Voice/SourceReader.h>
#include <Sound/Sound.h>

namespace SparkyStudios::Audio::Amplitude
{
    namespace
    {
        AmUInt64 DecodeSoundInstance(void* context, AmUInt64 offset, AmUInt64 frames)
        {
            return static_cast<SoundInstance*>(context)->GetAudio(offset, frames);
        }

        void ZeroFrames(AudioBuffer& out, AmUInt64 offset, AmUInt64 frames)
        {
            for (AmSize c = 0; c < out.GetChannelCount(); ++c)
                std::fill_n(out[c].begin() + offset, frames, 0.0f);
        }
    } // namespace

    VoiceSource MakeVoiceSource(SoundData* data)
    {
        VoiceSource source;
        if (data == nullptr || data->chunk == nullptr)
            return source;

        source.buffer = data->chunk->buffer;
        source.length = data->length;
        source.channels = data->format.GetNumChannels();
        source.sampleRate = data->format.GetSampleRate();

        if (data->stream)
        {
            source.decode = &DecodeSoundInstance;
            source.context = data->sound.get();
            source.decodeCapacity = data->chunk->frames;
        }

        return source;
    }

    void SourceReader::Initialize(const VoiceSource& source, AmUInt64 regionStart, AmUInt64 regionEnd, bool loop, AmUInt32 loopCount)
    {
        _source = source;
        _end = regionEnd == 0 ? source.length : AM_MIN(regionEnd, source.length);
        _start = AM_MIN(regionStart, _end);
        _loop = loop;
        _remainingWraps = !loop ? 0 : loopCount == 0 ? kInfiniteWraps : loopCount - 1;
        _cursor = _start;
        _ended = _end <= _start;
    }

    AmUInt64 SourceReader::Read(AudioBuffer& out, AmUInt64 offset, AmUInt64 frames, SourceReadReport& report)
    {
        AMPLITUDE_ASSERT(out.GetChannelCount() == _source.channels);
        AMPLITUDE_ASSERT(offset + frames <= out.GetFrameCount());

        AmUInt64 written = 0;
        AmUInt64 delivered = 0;

        while (written < frames)
        {
            if (_ended)
            {
                ZeroFrames(out, offset + written, frames - written);
                break;
            }

            if (_cursor >= _end)
            {
                if (_remainingWraps > 0)
                {
                    if (_remainingWraps != kInfiniteWraps)
                        --_remainingWraps;

                    _cursor = _start;
                    if (report.wraps++ == 0)
                        report.firstWrapOffset = written;

                    continue;
                }

                _ended = true;
                report.ended = true;
                report.endOffset = written;
                continue;
            }

            const AmUInt64 wanted = AM_MIN(frames - written, _end - _cursor);
            const AmUInt64 got = CopyFrames(out, offset + written, wanted);

            _cursor += got;
            written += got;
            delivered += got;

            if (got < wanted)
            {
                // A stream that runs dry before its known end is treated as the end.
                report.starved = true;
                _ended = true;
                report.ended = true;
                report.endOffset = written;
            }
        }

        return delivered;
    }

    void SourceReader::Seek(AmUInt64 frame)
    {
        _ended = _end <= _start;
        _cursor = _ended ? _start : AM_CLAMP(frame, _start, _end - 1);
    }

    AmUInt64 SourceReader::Rewind(AmUInt64 frames) const
    {
        const AmUInt64 played = _cursor - _start;
        if (frames <= played)
            return _cursor - frames;

        if (!_loop || _end <= _start)
            return _start;

        const AmUInt64 length = _end - _start;
        const AmUInt64 back = (frames - played) % length;
        return back == 0 ? _start : _end - back;
    }

    AmUInt64 SourceReader::CopyFrames(AudioBuffer& out, AmUInt64 offset, AmUInt64 frames)
    {
        if (!_source.IsStreamed())
        {
            AudioBuffer::Copy(*_source.buffer, _cursor, out, offset, frames);
            return frames;
        }

        AmUInt64 copied = 0;
        while (copied < frames)
        {
            const AmUInt64 wanted = AM_MIN(frames - copied, _source.decodeCapacity);
            const AmUInt64 got = _source.decode(_source.context, _cursor + copied, wanted);
            AudioBuffer::Copy(*_source.buffer, 0, out, offset + copied, got);
            copied += got;

            if (got < wanted)
                break;
        }

        return copied;
    }
} // namespace SparkyStudios::Audio::Amplitude
