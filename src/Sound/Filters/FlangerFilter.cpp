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

#include <Sound/Filters/FlangerFilter.h>

namespace SparkyStudios::Audio::Amplitude
{
    FlangerFilter::FlangerFilter()
        : _delay(0.05f)
        , _frequency(10.0f)
    {}

    AmResult FlangerFilter::Init(AmReal32 delay, AmReal32 frequency)
    {
        if (delay <= 0 || frequency <= 0)
            return AM_ERROR_INVALID_PARAMETER;

        _delay = delay;
        _frequency = frequency;

        return AM_ERROR_NO_ERROR;
    }

    AmUInt32 FlangerFilter::GetParamCount()
    {
        return ATTRIBUTE_LAST;
    }

    AmString FlangerFilter::GetParamName(AmUInt32 index)
    {
        if (index >= ATTRIBUTE_LAST)
            return "";

        static AmString names[ATTRIBUTE_LAST] = { "Wet", "Delay", "Frequency" };

        return names[index];
    }

    AmUInt32 FlangerFilter::GetParamType(AmUInt32 index)
    {
        return PARAM_FLOAT;
    }

    AmReal32 FlangerFilter::GetParamMax(AmUInt32 index)
    {
        if (index == ATTRIBUTE_DELAY)
            return 0.1f;

        if (index == ATTRIBUTE_FREQUENCY)
            return 100.0f;

        return 1.0f;
    }

    AmReal32 FlangerFilter::GetParamMin(AmUInt32 index)
    {
        if (index == ATTRIBUTE_WET)
            return 0.0f;

        if (index == ATTRIBUTE_FREQUENCY)
            return 0.1f;

        return 0.001f;
    }

    FilterInstance* FlangerFilter::CreateInstance()
    {
        return new FlangerFilterInstance(this);
    }

    FlangerFilterInstance::FlangerFilterInstance(FlangerFilter* parent)
        : FilterInstance(parent)
    {
        _buffer = nullptr;
        _bufferLength = 0;
        _bufferOffset = 0;
        _offset = 0;
        _index = 0;

        Init(parent->GetParamCount());

        m_parameters[FlangerFilter::ATTRIBUTE_DELAY] = parent->_delay;
        m_parameters[FlangerFilter::ATTRIBUTE_FREQUENCY] = parent->_frequency;
    }

    void FlangerFilterInstance::Process(
        AmAudioSampleBuffer buffer, AmUInt64 frames, AmUInt64 bufferSize, AmUInt16 channels, AmUInt32 sampleRate)
    {
        InitBuffer(channels, sampleRate);

        FilterInstance::Process(buffer, frames, bufferSize, channels, sampleRate);

        _offset += frames;
        _offset %= _bufferLength;
    }

    void FlangerFilterInstance::ProcessInterleaved(
        AmAudioSampleBuffer buffer, AmUInt64 frames, AmUInt64 bufferSize, AmUInt16 channels, AmUInt32 sampleRate)
    {
        InitBuffer(channels, sampleRate);

        FilterInstance::ProcessInterleaved(buffer, frames, bufferSize, channels, sampleRate);

        _offset += frames;
        _offset %= _bufferLength;
    }

    void FlangerFilterInstance::ProcessChannel(
        AmAudioSampleBuffer buffer, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate, bool isInterleaved)
    {
        const auto maxSamples =
            static_cast<AmUInt32>(std::ceil(m_parameters[FlangerFilter::ATTRIBUTE_DELAY] * static_cast<AmReal32>(sampleRate)));

        const AmUInt64 o = channel * _bufferLength;
        const AmReal64 i = m_parameters[FlangerFilter::ATTRIBUTE_FREQUENCY] * M_PI * 2 / static_cast<AmReal64>(sampleRate);

        for (AmUInt64 f = 0; f < frames; f++)
        {
            const AmUInt64 s = isInterleaved ? f * channels + channel : f + channel * frames;

            const auto delay = static_cast<AmInt32>(std::floor(static_cast<AmReal64>(maxSamples) * (1 + std::cos(_index))) / 2);
            _index += i;

            const AmReal32 x = buffer[s];
            /* */ AmReal32 y;

            _buffer[o + _offset % _bufferLength] = x;

            y = 0.5f * (x + _buffer[o + (_bufferLength - delay + _offset) % _bufferLength]);
            _offset++;

            y = x + (y - x) * m_parameters[FlangerFilter::ATTRIBUTE_WET];
            y = AM_CLAMP_AUDIO_SAMPLE(y);

            buffer[s] = static_cast<AmAudioSample>(y);
        }

        _offset -= frames;
    }

    FlangerFilterInstance::~FlangerFilterInstance()
    {
        amMemory->Free(MemoryPoolKind::Filtering, _buffer);
    }

    void FlangerFilterInstance::InitBuffer(AmUInt16 channels, AmUInt32 sampleRate)
    {
        const auto maxSamples =
            static_cast<AmUInt32>(std::ceil(m_parameters[FlangerFilter::ATTRIBUTE_DELAY] * static_cast<AmReal32>(sampleRate)));

        if (_bufferLength < maxSamples)
        {
            amMemory->Free(MemoryPoolKind::Filtering, _buffer);

            _bufferLength = maxSamples;
            const AmUInt32 size = _bufferLength * channels * sizeof(AmReal32);

            _buffer = static_cast<AmReal32Buffer>(amMemory->Malloc(MemoryPoolKind::Filtering, size));
            std::memset(_buffer, 0, size);
        }
    }
} // namespace SparkyStudios::Audio::Amplitude
