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

#include <Sound/Filters/WaveShaperFilter.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    WaveShaperFilter::WaveShaperFilter()
        : _amount(0.0f)
    {}

    AmResult WaveShaperFilter::Init(AmReal32 amount)
    {
        if (_amount < -1.0f || _amount > 1.0f)
            return AM_ERROR_INVALID_PARAMETER;

        _amount = amount;
        return AM_ERROR_NO_ERROR;
    }

    AmUInt32 WaveShaperFilter::GetParamCount()
    {
        return ATTRIBUTE_LAST;
    }

    AmString WaveShaperFilter::GetParamName(AmUInt32 index)
    {
        if (index >= ATTRIBUTE_LAST)
            return nullptr;

        AmString names[] = { "Wet", "Amount" };
        return names[index];
    }

    AmUInt32 WaveShaperFilter::GetParamType(AmUInt32 index)
    {
        return PARAM_FLOAT;
    }

    AmReal32 WaveShaperFilter::GetParamMax(AmUInt32 index)
    {
        return 1.0f;
    }

    AmReal32 WaveShaperFilter::GetParamMin(AmUInt32 index)
    {
        if (index == ATTRIBUTE_AMOUNT)
            return -1.0f;

        return 0.0f;
    }

    FilterInstance* WaveShaperFilter::CreateInstance()
    {
        return new WaveShaperFilterInstance(this);
    }

    WaveShaperFilterInstance::WaveShaperFilterInstance(WaveShaperFilter* parent)
        : FilterInstance(parent)
    {
        Init(parent->GetParamCount());
        m_parameters[WaveShaperFilter::ATTRIBUTE_AMOUNT] = parent->_amount;
    }

    AmInt16 WaveShaperFilterInstance::ProcessSample(AmInt16 sample, AmUInt16 channel, AmUInt32 sampleRate)
    {
        AmReal32 k;
        if (std::abs(m_parameters[WaveShaperFilter::ATTRIBUTE_AMOUNT] - 1.0f) < kEpsilon)
            k = 2 * m_parameters[WaveShaperFilter::ATTRIBUTE_AMOUNT] / 0.01f;
        else
            k = 2 * m_parameters[WaveShaperFilter::ATTRIBUTE_AMOUNT] / (1 - m_parameters[WaveShaperFilter::ATTRIBUTE_AMOUNT]);

        const AmInt32 x = sample;
        /* */ AmInt32 y;

        const AmReal32 p = (std::abs(x) * AmFloatToFixedPoint(k) + AmFloatToFixedPoint(1.0f)) >> kAmFixedPointBits;
        const AmReal32 q = (AmFloatToFixedPoint(1.0f + k) * x) >> kAmFixedPointBits;

        y = x * AmFloatToFixedPoint(q / p) >> kAmFixedPointBits;
        y = x + ((y - x) * AmFloatToFixedPoint(m_parameters[WaveShaperFilter::ATTRIBUTE_WET]) >> kAmFixedPointBits);
        y = AM_CLAMP(y, INT16_MIN, INT16_MAX);

        return static_cast<AmInt16>(y);
    }
} // namespace SparkyStudios::Audio::Amplitude
