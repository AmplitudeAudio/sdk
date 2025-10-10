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

#include <DSP/Filters/WaveShaperFilter.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    WaveShaperFilter::WaveShaperFilter()
        : Filter("WaveShaper")
        , _amount(0.0f)
    {}

    AmResult WaveShaperFilter::Init(AmReal32 amount)
    {
        if (_amount < -1.0f || _amount > 1.0f)
            return eErrorCode_InvalidParameter;

        _amount = amount;
        return eErrorCode_Success;
    }

    AmUInt32 WaveShaperFilter::GetParameterCount() const
    {
        return ATTRIBUTE_LAST;
    }

    AmString WaveShaperFilter::GetParameterName(AmUInt32 index) const
    {
        if (index >= ATTRIBUTE_LAST)
            return "";

        static constexpr const char* names[] = { "Wet", "Amount" };
        return names[index];
    }

    eParameterType WaveShaperFilter::GetParameterType(AmUInt32 index) const
    {
        return eParameterType_Float;
    }

    AmReal32 WaveShaperFilter::GetParameterMax(AmUInt32 index) const
    {
        return 1.0f;
    }

    AmReal32 WaveShaperFilter::GetParameterMin(AmUInt32 index) const
    {
        if (index == ATTRIBUTE_AMOUNT)
            return -1.0f;

        return 0.0f;
    }

    std::shared_ptr<FilterInstance> WaveShaperFilter::CreateInstance()
    {
        return AmSharedPtr<WaveShaperFilterInstance, eMemoryPoolKind_Filtering>::Make(this);
    }

    WaveShaperFilterInstance::WaveShaperFilterInstance(WaveShaperFilter* parent)
        : FilterInstance(parent)
    {
        Initialize(parent->GetParameterCount());
        m_parameters[WaveShaperFilter::ATTRIBUTE_AMOUNT] = parent->_amount;
    }

    AmAudioSample WaveShaperFilterInstance::ProcessSample(AmAudioSample sample, AmUInt16 channel, AmUInt32 sampleRate)
    {
        AmReal32 k;
        if (std::abs(m_parameters[WaveShaperFilter::ATTRIBUTE_AMOUNT] - 1.0f) < kEpsilon)
            k = 2 * m_parameters[WaveShaperFilter::ATTRIBUTE_AMOUNT] / 0.01f;
        else
            k = 2 * m_parameters[WaveShaperFilter::ATTRIBUTE_AMOUNT] / (1 - m_parameters[WaveShaperFilter::ATTRIBUTE_AMOUNT]);

        const AmReal32 x = sample;
        /* */ AmReal32 y;

        const AmReal32 p = std::abs(x) * k + 1.0f;
        const AmReal32 q = (1.0f + k) * x;

        y = x * (q / p);
        y = x + (y - x) * m_parameters[WaveShaperFilter::ATTRIBUTE_WET];

        return static_cast<AmAudioSample>(y);
    }
} // namespace SparkyStudios::Audio::Amplitude
