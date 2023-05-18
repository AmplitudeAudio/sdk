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

#include <Sound/Filters/BassBoostFilter.h>

namespace SparkyStudios::Audio::Amplitude
{
    BassBoostFilter::BassBoostFilter()
        : FFTFilter("BassBoost")
        , m_boost(2.0f)
    {}

    AmResult BassBoostFilter::Init(AmReal32 aBoost)
    {
        if (aBoost < ATTRIBUTE_WET)
            return AM_ERROR_INVALID_PARAMETER;

        m_boost = aBoost;
        return AM_ERROR_NO_ERROR;
    }

    AmUInt32 BassBoostFilter::GetParamCount()
    {
        return ATTRIBUTE_LAST;
    }

    AmString BassBoostFilter::GetParamName(AmUInt32 index)
    {
        if (index == ATTRIBUTE_BOOST)
            return "Boost";

        return "Wet";
    }

    AmUInt32 BassBoostFilter::GetParamType(AmUInt32 index)
    {
        return PARAM_FLOAT;
    }

    AmReal32 BassBoostFilter::GetParamMax(AmUInt32 index)
    {
        if (index == ATTRIBUTE_BOOST)
            return 10.0f;

        return 1.0f;
    }

    AmReal32 BassBoostFilter::GetParamMin(AmUInt32 index)
    {
        return 0.0f;
    }

    FilterInstance* BassBoostFilter::CreateInstance()
    {
        return amnew(BassBoostFilterInstance, this);
    }

    void BassBoostFilter::DestroyInstance(FilterInstance* instance)
    {
        amdelete(BassBoostFilterInstance, (BassBoostFilterInstance*)instance);
    }

    BassBoostFilterInstance::BassBoostFilterInstance(BassBoostFilter* parent)
        : FFTFilterInstance(parent)
    {
        Init(BassBoostFilter::ATTRIBUTE_LAST);
        m_parameters[BassBoostFilter::ATTRIBUTE_BOOST] = parent->m_boost;
    }

    void BassBoostFilterInstance::ProcessFFTChannel(
        AmReal64Buffer buffer, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate)
    {
        Comp2MagPhase(buffer, 2);

        for (AmUInt32 i = 0; i < 2; i++)
            buffer[i * 2] *= m_parameters[BassBoostFilter::ATTRIBUTE_BOOST];

        MagPhase2Comp(buffer, 2);
    }
} // namespace SparkyStudios::Audio::Amplitude
