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

    AmResult DCRemovalFilter::Init(float length)
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
            _bufferLength = static_cast<AmUInt64>(
                AmFloatToFixedPoint(dynamic_cast<DCRemovalFilter*>(m_parent)->_length) * sampleRate >> kAmFixedPointShift);
            _buffer = new AmInt32[_bufferLength * channels];
            _totals = new AmInt32[channels];

            memset(_buffer, 0, _bufferLength * channels * sizeof(AmInt32));
            memset(_totals, 0, channels * sizeof(AmInt32));
        }

        AmUInt64 prevOffset = (_offset + _bufferLength - 1) % _bufferLength;
        for (AmInt64 i = 0; i < frames * channels; i++)
        {
            const AmInt32 x = buffer[i];
            /* */ AmInt32 y;

            const AmUInt64 c = i % channels;
            const AmUInt64 o = _offset + c * _bufferLength;

            _totals[c] -= _buffer[o];
            _totals[c] += x;

            _buffer[o] = x;

            y = x - (AmFloatToFixedPoint(static_cast<AmReal32>(_totals[c]) / _bufferLength) >> kAmFixedPointShift);
            y = x + ((y - x) * AmFloatToFixedPoint(m_parameters[DCRemovalFilter::ATTRIBUTE_WET]) >> kAmFixedPointShift);
            y = AM_CLAMP(y, INT16_MIN, INT16_MAX);

            buffer[i] = static_cast<AmInt16>(y);

            if (c == channels - 1)
            {
                prevOffset = _offset;
                _offset = (_offset + 1) % _bufferLength;
            }
        }
    }
} // namespace SparkyStudios::Audio::Amplitude
