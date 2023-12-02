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

#include <Sound/Filters/DelayFilter.h>

namespace SparkyStudios::Audio::Amplitude
{
    DelayFilter::DelayFilter()
        : Filter("Delay")
        , _delay(0.3f)
        , _decay(0.7f)
        , _delayStart(0.0f)
    {}

    AmResult DelayFilter::Init(AmReal32 delay, AmReal32 decay, AmReal32 delayStart)
    {
        if (delay <= 0 || decay <= 0)
            return AM_ERROR_INVALID_PARAMETER;

        _delay = delay;
        _decay = decay;
        _delayStart = delayStart;

        return AM_ERROR_NO_ERROR;
    }

    AmUInt32 DelayFilter::GetParamCount()
    {
        return ATTRIBUTE_LAST;
    }

    AmString DelayFilter::GetParamName(AmUInt32 index)
    {
        if (index >= ATTRIBUTE_LAST)
            return "";

        AmString names[ATTRIBUTE_LAST] = { "Wet", "Delay", "Decay", "DelayStart" };

        return names[index];
    }

    AmUInt32 DelayFilter::GetParamType(AmUInt32 index)
    {
        if (index == ATTRIBUTE_DELAY_START)
            return PARAM_BOOL;

        return PARAM_FLOAT;
    }

    AmReal32 DelayFilter::GetParamMax(AmUInt32 index)
    {
        return 1.0f;
    }

    AmReal32 DelayFilter::GetParamMin(AmUInt32 index)
    {
        return 0.0f;
    }

    FilterInstance* DelayFilter::CreateInstance()
    {
        return ampoolnew(MemoryPoolKind::Filtering, DelayFilterInstance, this);
    }

    void DelayFilter::DestroyInstance(FilterInstance* instance)
    {
        ampooldelete(MemoryPoolKind::Filtering, DelayFilterInstance, (DelayFilterInstance*)instance);
    }

    DelayFilterInstance::DelayFilterInstance(DelayFilter* parent)
        : FilterInstance(parent)
    {
        _buffer = nullptr;
        _bufferLength = 0;
        _bufferOffset = 0;
        _bufferMaxLength = 0;
        _offset = 0;

        Init(parent->GetParamCount());

        m_parameters[DelayFilter::ATTRIBUTE_DELAY] = parent->_delay;
        m_parameters[DelayFilter::ATTRIBUTE_DECAY] = parent->_decay;
        m_parameters[DelayFilter::ATTRIBUTE_DELAY_START] = parent->_delayStart;
    }

    DelayFilterInstance::~DelayFilterInstance()
    {
        if (_buffer == nullptr)
            return;

        ampoolfree(MemoryPoolKind::Filtering, _buffer);
    }

    void DelayFilterInstance::Process(
        AmAudioSampleBuffer buffer, AmUInt64 frames, AmUInt64 bufferSize, AmUInt16 channels, AmUInt32 sampleRate)
    {
        InitBuffer(channels, sampleRate);

        for (AmUInt64 f = 0; f < frames; f++)
        {
            for (AmUInt16 c = 0; c < channels; c++)
            {
                _bufferOffset = _offset * channels + c;

                const AmUInt64 o = f * channels + c;
                buffer[o] = ProcessSample(buffer[o], c, sampleRate);
            }

            _offset = (_offset + 1) % _bufferLength;
        }
    }

    AmAudioSample DelayFilterInstance::ProcessSample(AmAudioSample sample, AmUInt16 channel, AmUInt32 sampleRate)
    {
        const AmUInt32 o = _bufferOffset;

        const AmReal32 x = sample;
        /* */ AmReal32 y;

        if (m_parameters[DelayFilter::ATTRIBUTE_DELAY_START] != 0.0f)
        {
            // Read first
            y = _buffer[o] * m_parameters[DelayFilter::ATTRIBUTE_WET];
            y = AM_CLAMP_AUDIO_SAMPLE(y);

            // Produce feedback
            _buffer[o] = _buffer[o] * m_parameters[DelayFilter::ATTRIBUTE_DECAY] + x;
        }
        else
        {
            // Produce feedback first
            _buffer[o] = _buffer[o] * m_parameters[DelayFilter::ATTRIBUTE_DECAY] + x;

            // Read
            y = _buffer[o] * m_parameters[DelayFilter::ATTRIBUTE_WET];
            y = AM_CLAMP_AUDIO_SAMPLE(y);
        }

        return static_cast<AmAudioSample>(y);
    }

    void DelayFilterInstance::InitBuffer(AmUInt16 channels, AmUInt32 sampleRate)
    {
        const auto maxSamples =
            static_cast<AmUInt32>(std::ceil(m_parameters[DelayFilter::ATTRIBUTE_DELAY] * static_cast<AmReal32>(sampleRate)));

        if (_buffer == nullptr)
        {
            _offset = 0;
            _bufferOffset = 0;

            _bufferMaxLength = maxSamples;
            const AmUInt32 size = _bufferMaxLength * channels * sizeof(AmReal32);

            _buffer = static_cast<AmReal32Buffer>(ampoolmalloc(MemoryPoolKind::Filtering, size));
            std::memset(_buffer, 0, size);
        }

        _bufferLength = maxSamples;
        if (_bufferLength > _bufferMaxLength)
            _bufferLength = _bufferMaxLength;
    }

} // namespace SparkyStudios::Audio::Amplitude
