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

#include <Sound/Filters/FlangerFilter.h>
#include <Utils/Utils.h>

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
            return nullptr;

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
        {
            return 0.1f;
        }

        if (index == ATTRIBUTE_FREQUENCY)
        {
            return 100.0f;
        }

        return 1.0f;
    }

    AmReal32 FlangerFilter::GetParamMin(AmUInt32 index)
    {
        if (index == ATTRIBUTE_WET)
        {
            return 0.0f;
        }

        if (index == ATTRIBUTE_FREQUENCY)
        {
            return 0.1f;
        }

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
        _offset = 0;
        _index = 0;

        Init(3);

        m_parameters[FlangerFilter::ATTRIBUTE_DELAY] = parent->_delay;
        m_parameters[FlangerFilter::ATTRIBUTE_FREQUENCY] = parent->_frequency;
    }

    void FlangerFilterInstance::Process(AmInt16Buffer buffer, AmUInt64 frames, AmUInt64 bufferSize, AmUInt16 channels, AmUInt32 sampleRate)
    {
        const auto maxSamples =
            static_cast<AmUInt32>(std::ceil(m_parameters[FlangerFilter::ATTRIBUTE_DELAY] * static_cast<AmReal32>(sampleRate)));

        if (_bufferLength < maxSamples)
        {
            delete[] _buffer;

            _bufferLength = maxSamples;
            const AmUInt32 length = _bufferLength * channels;

            _buffer = new AmInt32[length];
            memset(_buffer, 0, length * sizeof(AmInt32));
        }

        FilterInstance::Process(buffer, frames, bufferSize, channels, sampleRate);

        _offset += frames;
        _offset %= _bufferLength;
    }

    void FlangerFilterInstance::ProcessInterleaved(
        AmInt16Buffer buffer, AmUInt64 frames, AmUInt64 bufferSize, AmUInt16 channels, AmUInt32 sampleRate)
    {
        const auto maxSamples =
            static_cast<AmUInt32>(std::ceil(m_parameters[FlangerFilter::ATTRIBUTE_DELAY] * static_cast<AmReal32>(sampleRate)));

        if (_bufferLength < maxSamples)
        {
            delete[] _buffer;

            _bufferLength = maxSamples;
            const AmUInt32 length = _bufferLength * channels;

            _buffer = new AmInt32[length];
            memset(_buffer, 0, length * sizeof(AmInt32));
        }

        FilterInstance::ProcessInterleaved(buffer, frames, bufferSize, channels, sampleRate);

        _offset += frames;
        _offset %= _bufferLength;
    }

    void FlangerFilterInstance::ProcessChannel(
        AmInt16Buffer buffer, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate, bool isInterleaved)
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

            const AmInt32 x = buffer[s];
            /* */ AmInt32 y;

            _buffer[o + _offset % _bufferLength] = x;

            // clang-format off
            y = AmFloatToFixedPoint(0.5f) * (x + _buffer[o + (_bufferLength - delay + _offset) % _bufferLength]) >> kAmFixedPointShift;
            _offset++;

            y = x + ((y - x) * AmFloatToFixedPoint(m_parameters[FlangerFilter::ATTRIBUTE_WET]) >> kAmFixedPointShift);
            y = AM_CLAMP(y, INT16_MIN, INT16_MAX);
            // clang-format on

            buffer[s] = static_cast<AmInt16>(y);
        }

        _offset -= frames;
    }

    FlangerFilterInstance::~FlangerFilterInstance()
    {
        delete[] _buffer;
    }
} // namespace SparkyStudios::Audio::Amplitude
