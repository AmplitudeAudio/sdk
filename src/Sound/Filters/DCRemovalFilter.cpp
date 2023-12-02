// Copyright (c) 2021-present Sparky Studios. All rights reserved.
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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

#include <Sound/Filters/DCRemovalFilter.h>

namespace SparkyStudios::Audio::Amplitude
{
    DCRemovalFilter::DCRemovalFilter()
        : Filter("DCRemoval")
        , _length(0.1f)
    {}

    AmResult DCRemovalFilter::Init(AmReal32 length)
    {
        if (length <= 0)
            return AM_ERROR_INVALID_PARAMETER;

        _length = length;

        return AM_ERROR_NO_ERROR;
    }

    FilterInstance* DCRemovalFilter::CreateInstance()
    {
        return ampoolnew(MemoryPoolKind::Filtering, DCRemovalFilterInstance, this);
    }

    void DCRemovalFilter::DestroyInstance(FilterInstance* instance)
    {
        ampooldelete(MemoryPoolKind::Filtering, DCRemovalFilterInstance, (DCRemovalFilterInstance*)instance);
    }

    DCRemovalFilterInstance::DCRemovalFilterInstance(DCRemovalFilter* parent)
        : FilterInstance(parent)
        , _buffer(nullptr)
        , _totals(nullptr)
        , _bufferLength(0)
        , _offset(0)
    {
        Init(1);
    }

    DCRemovalFilterInstance::~DCRemovalFilterInstance()
    {
        ampoolfree(MemoryPoolKind::Filtering, _buffer);
        ampoolfree(MemoryPoolKind::Filtering, _totals);
    }

    void DCRemovalFilterInstance::Process(
        AmAudioSampleBuffer buffer, AmUInt64 frames, AmUInt64 bufferSize, AmUInt16 channels, AmUInt32 sampleRate)
    {
        if (_buffer == nullptr)
        {
            InitBuffer(channels, sampleRate);
        }

        for (AmInt64 i = 0; i < frames; i++)
        {
            for (AmUInt16 c = 0; c < channels; c++)
            {
                const AmUInt64 o = i * channels + c;
                buffer[o] = ProcessSample(buffer[o], c, sampleRate);
            }

            _offset = (_offset + 1) % _bufferLength;
        }
    }

    AmAudioSample DCRemovalFilterInstance::ProcessSample(AmAudioSample sample, AmUInt16 channel, AmUInt32 sampleRate)
    {
        const AmUInt64 o = _offset + channel * _bufferLength;

        const AmReal32 x = sample;
        /* */ AmReal32 y;

        _totals[channel] -= _buffer[o];
        _totals[channel] += x;

        _buffer[o] = x;

        y = x - _totals[channel] / static_cast<AmReal32>(_bufferLength);
        y = x + (y - x) * m_parameters[DCRemovalFilter::ATTRIBUTE_WET];
        y = AM_CLAMP_AUDIO_SAMPLE(y);

        return static_cast<AmAudioSample>(y);
    }

    void DCRemovalFilterInstance::InitBuffer(AmUInt16 channels, AmUInt32 sampleRate)
    {
        _bufferLength = static_cast<AmUInt64>(std::ceil(dynamic_cast<DCRemovalFilter*>(m_parent)->_length * sampleRate));

        _buffer = static_cast<AmReal32Buffer>(ampoolmalloc(MemoryPoolKind::Filtering, _bufferLength * channels * sizeof(AmReal32)));
        _totals = static_cast<AmReal32Buffer>(ampoolmalloc(MemoryPoolKind::Filtering, channels * sizeof(AmReal32)));

        std::memset(_buffer, 0, _bufferLength * channels * sizeof(AmReal32));
        std::memset(_totals, 0, channels * sizeof(AmReal32));
    }
} // namespace SparkyStudios::Audio::Amplitude
