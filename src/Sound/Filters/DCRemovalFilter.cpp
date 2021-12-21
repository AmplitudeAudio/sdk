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

#include <Sound/Filters/DCRemovalFilter.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    DCRemovalFilter::DCRemovalFilter()
        : _length(0.1f)
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
        return new DCRemovalFilterInstance(this);
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
        delete[] _buffer;
        delete[] _totals;
    }

    void DCRemovalFilterInstance::Process(
        AmInt16Buffer buffer, AmUInt64 frames, AmUInt64 bufferSize, AmUInt16 channels, AmUInt32 sampleRate)
    {
        if (_buffer == nullptr)
        {
            InitBuffer(channels, sampleRate);
        }

        for (AmInt64 i = 0; i < frames; i++)
        {
            for (AmUInt16 c = 0; c < channels; c++)
            {
                const AmUInt64 o = i + c * frames;
                buffer[o] = ProcessSample(buffer[o], c, sampleRate);
            }

            _offset = (_offset + 1) % _bufferLength;
        }
    }

    void DCRemovalFilterInstance::ProcessInterleaved(
        AmInt16Buffer buffer, AmUInt64 frames, AmUInt64 bufferSize, AmUInt16 channels, AmUInt32 sampleRate)
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

    AmInt16 DCRemovalFilterInstance::ProcessSample(AmInt16 sample, AmUInt16 channel, AmUInt32 sampleRate)
    {
        const AmUInt64 o = _offset + channel * _bufferLength;

        const AmInt32 x = sample;
        /* */ AmInt32 y;

        _totals[channel] -= _buffer[o];
        _totals[channel] += x;

        _buffer[o] = x;

        y = x - (AmFloatToFixedPoint(static_cast<AmReal32>(_totals[channel]) / _bufferLength) >> kAmFixedPointShift);
        y = x + ((y - x) * AmFloatToFixedPoint(m_parameters[DCRemovalFilter::ATTRIBUTE_WET]) >> kAmFixedPointShift);
        y = AM_CLAMP(y, INT16_MIN, INT16_MAX);

        return static_cast<AmInt16>(y);
    }

    void DCRemovalFilterInstance::InitBuffer(AmUInt16 channels, AmUInt32 sampleRate)
    {
        _bufferLength = static_cast<AmUInt64>(
            AmFloatToFixedPoint(dynamic_cast<DCRemovalFilter*>(m_parent)->_length) * sampleRate >> kAmFixedPointShift);

        _buffer = new AmInt32[_bufferLength * channels];
        _totals = new AmInt32[channels];

        memset(_buffer, 0, _bufferLength * channels * sizeof(AmInt32));
        memset(_totals, 0, channels * sizeof(AmInt32));
    }
} // namespace SparkyStudios::Audio::Amplitude
