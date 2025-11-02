// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#include <DSP/Filters/MonoPoleFilter.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    MonoPoleFilterInstance::MonoPoleFilterInstance(MonoPoleFilter* parent)
        : FilterInstance(parent)
        , _previousSample(0.0f)
    {
        Initialize(MonoPoleFilter::ATTRIBUTE_LAST);
        SetParameter(MonoPoleFilter::ATTRIBUTE_COEFFICIENT, parent->_coefficient);
    }

    AmAudioSample MonoPoleFilterInstance::ProcessSample(AmAudioSample sample, AmUInt16 channel, AmUInt32 sampleRate)
    {
        const AmReal32 coefficient = m_parameters[MonoPoleFilter::ATTRIBUTE_COEFFICIENT];

        if (coefficient < kEpsilon)
        {
            _previousSample = sample;
            return 0.0f;
        }

        const AmReal32 x = sample;
        /* */ AmReal32 y = coefficient * (_previousSample - x) + x;
        _previousSample = y;

        y = x + (y - x) * m_parameters[MonoPoleFilter::ATTRIBUTE_WET];

        return static_cast<AmAudioSample>(y);
    }

    MonoPoleFilter::MonoPoleFilter()
        : Filter("MonoPole")
        , _coefficient(0.0f)
    {}

    AmResult MonoPoleFilter::Initialize(AmReal32 coefficient)
    {
        if (coefficient < 0.0f || coefficient > 1.0f)
            return eErrorCode_InvalidParameter;

        _coefficient = AM_CLAMP(coefficient, 0.0f, 1.0f);

        return eErrorCode_Success;
    }

    AmUInt32 MonoPoleFilter::GetParameterCount() const
    {
        return ATTRIBUTE_LAST;
    }

    AmString MonoPoleFilter::GetParameterName(AmUInt32 index) const
    {
        if (index >= ATTRIBUTE_LAST)
            return "Unknown";

        static const AmString names[ATTRIBUTE_LAST] = { "Wet", "Coefficient" };

        return names[index];
    }

    eParameterType MonoPoleFilter::GetParameterType(AmUInt32 index) const
    {
        return eParameterType_Float;
    }

    AmReal32 MonoPoleFilter::GetParameterMax(AmUInt32 index) const
    {
        if (index >= ATTRIBUTE_LAST)
            return 0.0f;

        static const AmReal32 values[ATTRIBUTE_LAST] = { 1.0f, 1.0f };

        return values[index];
    }

    AmReal32 MonoPoleFilter::GetParameterMin(AmUInt32 index) const
    {
        if (index >= ATTRIBUTE_LAST)
            return 0.0f;

        static const AmReal32 values[ATTRIBUTE_LAST] = { 0.0f, 0.0f };

        return values[index];
    }

    std::shared_ptr<FilterInstance> MonoPoleFilter::CreateInstance()
    {
        return ampoolshared(eMemoryPoolKind_Filtering, MonoPoleFilterInstance, this);
    }
} // namespace SparkyStudios::Audio::Amplitude
