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

#include <Sound/Filters/EchoFilter.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    EchoFilter::EchoFilter()
        : _delay(0.3f)
        , _decay(0.7f)
        , _filter(0.0f)
    {}

    AmResult EchoFilter::Init(AmReal32 delay, AmReal32 decay, AmReal32 filter)
    {
        if (delay <= 0 || decay <= 0 || filter < 0 || filter > 1.0f)
            return AM_ERROR_INVALID_PARAMETER;

        _delay = delay;
        _decay = decay;
        _filter = filter;

        return AM_ERROR_NO_ERROR;
    }

    AmUInt32 EchoFilter::GetParamCount()
    {
        return ATTRIBUTE_LAST;
    }

    AmString EchoFilter::GetParamName(AmUInt32 index)
    {
        if (index >= ATTRIBUTE_LAST)
            return "";

        AmString names[ATTRIBUTE_LAST] = { "Wet", "Delay", "Decay", "Filter" };

        return names[index];
    }

    AmUInt32 EchoFilter::GetParamType(AmUInt32 index)
    {
        return PARAM_FLOAT;
    }

    AmReal32 EchoFilter::GetParamMax(AmUInt32 index)
    {
        if (index == ATTRIBUTE_DELAY)
        {
            return _delay;
        }

        return 1.0f;
    }

    AmReal32 EchoFilter::GetParamMin(AmUInt32 index)
    {
        return 0.0f;
    }

    FilterInstance* EchoFilter::CreateInstance()
    {
        return new EchoFilterInstance(this);
    }

    EchoFilterInstance::EchoFilterInstance(EchoFilter* parent)
        : FilterInstance(parent)
    {
        _buffer = nullptr;
        _bufferLength = 0;
        _bufferMaxLength = 0;
        _offset = 0;

        Init(4);

        m_parameters[EchoFilter::ATTRIBUTE_DELAY] = parent->_delay;
        m_parameters[EchoFilter::ATTRIBUTE_DECAY] = parent->_decay;
        m_parameters[EchoFilter::ATTRIBUTE_FILTER] = parent->_filter;
    }

    EchoFilterInstance::~EchoFilterInstance()
    {
        delete[] _buffer;
    }

    void EchoFilterInstance::Process(AmInt16Buffer buffer, AmUInt64 frames, AmUInt64 bufferSize, AmUInt16 channels, AmUInt32 sampleRate)
    {
        InitBuffer(channels, sampleRate);

        _prevOffset = (_offset + _bufferLength - 1) % _bufferLength;

        for (AmInt64 i = 0; i < frames; i++)
        {
            for (AmUInt16 c = 0; c < channels; c++)
            {
                const AmUInt64 o = i + c * frames;
                buffer[o] = ProcessSample(buffer[o], c, sampleRate);
            }

            _prevOffset = _offset;
            _offset = (_offset + 1) % _bufferLength;
        }
    }

    void EchoFilterInstance::ProcessInterleaved(
        AmInt16Buffer buffer, AmUInt64 frames, AmUInt64 bufferSize, AmUInt16 channels, AmUInt32 sampleRate)
    {
        InitBuffer(channels, sampleRate);

        _prevOffset = (_offset + _bufferLength - 1) % _bufferLength;

        for (AmInt64 i = 0; i < frames; i++)
        {
            for (AmUInt16 c = 0; c < channels; c++)
            {
                const AmUInt64 o = i * channels + c;
                buffer[o] = ProcessSample(buffer[o], c, sampleRate);
            }

            _prevOffset = _offset;
            _offset = (_offset + 1) % _bufferLength;
        }
    }

    AmInt16 EchoFilterInstance::ProcessSample(AmInt16 sample, AmUInt16 channel, AmUInt32 sampleRate)
    {
        const AmUInt32 o = channel * _bufferLength;

        const AmInt32 x = sample;
        /* */ AmInt32 y;

        // clang-format off
        const AmInt32 p = AmFloatToFixedPoint(m_parameters[EchoFilter::ATTRIBUTE_FILTER]) * _buffer[_prevOffset + o] >> kAmFixedPointBits;
        const AmInt32 q = AmFloatToFixedPoint(1.0f - m_parameters[EchoFilter::ATTRIBUTE_FILTER]) * _buffer[_offset + o] >> kAmFixedPointBits;

        y = p + q;
        y = x + (y * AmFloatToFixedPoint(m_parameters[EchoFilter::ATTRIBUTE_DECAY]) >> kAmFixedPointBits);
        // clang-format on

        _buffer[_offset + o] = y;

        y = x + ((y - x) * AmFloatToFixedPoint(m_parameters[EchoFilter::ATTRIBUTE_WET]) >> kAmFixedPointBits);
        y = AM_CLAMP(y, INT16_MIN, INT16_MAX);

        return static_cast<AmInt16>(y);
    }

    void EchoFilterInstance::InitBuffer(AmUInt16 channels, AmUInt32 sampleRate)
    {
        const auto maxSamples =
            static_cast<AmUInt32>(std::ceil(m_parameters[EchoFilter::ATTRIBUTE_DELAY] * static_cast<AmReal32>(sampleRate)));

        if (_buffer == nullptr)
        {
            // We only know channels and sample rate at this point... not really optimal
            _bufferMaxLength = maxSamples;
            const AmUInt32 length = _bufferMaxLength * channels;

            _buffer = new AmInt32[length];
            memset(_buffer, 0, length * sizeof(AmInt32));
        }

        _bufferLength = maxSamples;
        if (_bufferLength > _bufferMaxLength)
            _bufferLength = _bufferMaxLength;
    }

} // namespace SparkyStudios::Audio::Amplitude
