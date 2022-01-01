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

#include <Sound/Filters/EqualizerFilter.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    EqualizerFilter::EqualizerFilter()
        : _volume{}
    {
        for (AmUInt32 i = 0; i < 8; i++)
        {
            _volume[i] = 1.0f;
        }
    }

    AmResult EqualizerFilter::Init(
        AmReal32 volume1,
        AmReal32 volume2,
        AmReal32 volume3,
        AmReal32 volume4,
        AmReal32 volume5,
        AmReal32 volume6,
        AmReal32 volume7,
        AmReal32 volume8)
    {
        if (volume1 < GetParamMin(ATTRIBUTE_BAND_1) || volume1 > GetParamMax(ATTRIBUTE_BAND_1))
            return AM_ERROR_INVALID_PARAMETER;

        if (volume2 < GetParamMin(ATTRIBUTE_BAND_2) || volume2 > GetParamMax(ATTRIBUTE_BAND_2))
            return AM_ERROR_INVALID_PARAMETER;

        if (volume3 < GetParamMin(ATTRIBUTE_BAND_3) || volume3 > GetParamMax(ATTRIBUTE_BAND_3))
            return AM_ERROR_INVALID_PARAMETER;

        if (volume4 < GetParamMin(ATTRIBUTE_BAND_4) || volume4 > GetParamMax(ATTRIBUTE_BAND_4))
            return AM_ERROR_INVALID_PARAMETER;

        if (volume5 < GetParamMin(ATTRIBUTE_BAND_5) || volume5 > GetParamMax(ATTRIBUTE_BAND_5))
            return AM_ERROR_INVALID_PARAMETER;

        if (volume6 < GetParamMin(ATTRIBUTE_BAND_6) || volume6 > GetParamMax(ATTRIBUTE_BAND_6))
            return AM_ERROR_INVALID_PARAMETER;

        if (volume7 < GetParamMin(ATTRIBUTE_BAND_7) || volume7 > GetParamMax(ATTRIBUTE_BAND_7))
            return AM_ERROR_INVALID_PARAMETER;

        if (volume8 < GetParamMin(ATTRIBUTE_BAND_8) || volume8 > GetParamMax(ATTRIBUTE_BAND_8))
            return AM_ERROR_INVALID_PARAMETER;

        _volume[ATTRIBUTE_BAND_1 - ATTRIBUTE_BAND_1] = volume1;
        _volume[ATTRIBUTE_BAND_2 - ATTRIBUTE_BAND_1] = volume2;
        _volume[ATTRIBUTE_BAND_3 - ATTRIBUTE_BAND_1] = volume3;
        _volume[ATTRIBUTE_BAND_4 - ATTRIBUTE_BAND_1] = volume4;
        _volume[ATTRIBUTE_BAND_5 - ATTRIBUTE_BAND_1] = volume5;
        _volume[ATTRIBUTE_BAND_6 - ATTRIBUTE_BAND_1] = volume6;
        _volume[ATTRIBUTE_BAND_7 - ATTRIBUTE_BAND_1] = volume7;
        _volume[ATTRIBUTE_BAND_8 - ATTRIBUTE_BAND_1] = volume8;

        return AM_ERROR_NO_ERROR;
    }

    AmUInt32 EqualizerFilter::GetParamCount()
    {
        return ATTRIBUTE_LAST;
    }

    AmString EqualizerFilter::GetParamName(AmUInt32 index)
    {
        switch (index)
        {
        case ATTRIBUTE_WET:
            return "Wet";
        case ATTRIBUTE_BAND_1:
            return "Band 1";
        case ATTRIBUTE_BAND_2:
            return "Band 2";
        case ATTRIBUTE_BAND_3:
            return "Band 3";
        case ATTRIBUTE_BAND_4:
            return "Band 4";
        case ATTRIBUTE_BAND_5:
            return "Band 5";
        case ATTRIBUTE_BAND_6:
            return "Band 6";
        case ATTRIBUTE_BAND_7:
            return "Band 7";
        case ATTRIBUTE_BAND_8:
            return "Band 8";
        default:
            return nullptr;
        }
    }

    AmUInt32 EqualizerFilter::GetParamType(AmUInt32 index)
    {
        return PARAM_FLOAT;
    }

    AmReal32 EqualizerFilter::GetParamMax(AmUInt32 index)
    {
        if (index == ATTRIBUTE_WET)
            return 1.0f;

        return 4.0f;
    }

    AmReal32 EqualizerFilter::GetParamMin(AmUInt32 index)
    {
        return 0.0f;
    }

    FilterInstance* EqualizerFilter::CreateInstance()
    {
        return new EqualizerFilterInstance(this);
    }

    EqualizerFilterInstance::EqualizerFilterInstance(EqualizerFilter* parent)
        : FFTFilterInstance(parent)
    {
        Init(parent->GetParamCount());

        m_parameters[EqualizerFilter::ATTRIBUTE_BAND_1] =
            parent->_volume[EqualizerFilter::ATTRIBUTE_BAND_1 - EqualizerFilter::ATTRIBUTE_BAND_1];
        m_parameters[EqualizerFilter::ATTRIBUTE_BAND_2] =
            parent->_volume[EqualizerFilter::ATTRIBUTE_BAND_2 - EqualizerFilter::ATTRIBUTE_BAND_1];
        m_parameters[EqualizerFilter::ATTRIBUTE_BAND_3] =
            parent->_volume[EqualizerFilter::ATTRIBUTE_BAND_3 - EqualizerFilter::ATTRIBUTE_BAND_1];
        m_parameters[EqualizerFilter::ATTRIBUTE_BAND_4] =
            parent->_volume[EqualizerFilter::ATTRIBUTE_BAND_4 - EqualizerFilter::ATTRIBUTE_BAND_1];
        m_parameters[EqualizerFilter::ATTRIBUTE_BAND_5] =
            parent->_volume[EqualizerFilter::ATTRIBUTE_BAND_5 - EqualizerFilter::ATTRIBUTE_BAND_1];
        m_parameters[EqualizerFilter::ATTRIBUTE_BAND_6] =
            parent->_volume[EqualizerFilter::ATTRIBUTE_BAND_6 - EqualizerFilter::ATTRIBUTE_BAND_1];
        m_parameters[EqualizerFilter::ATTRIBUTE_BAND_7] =
            parent->_volume[EqualizerFilter::ATTRIBUTE_BAND_7 - EqualizerFilter::ATTRIBUTE_BAND_1];
        m_parameters[EqualizerFilter::ATTRIBUTE_BAND_8] =
            parent->_volume[EqualizerFilter::ATTRIBUTE_BAND_8 - EqualizerFilter::ATTRIBUTE_BAND_1];
    }

    void EqualizerFilterInstance::ProcessFFTChannel(
        AmReal64Buffer buffer, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate)
    {
        Comp2MagPhase(buffer, frames / 2);

        for (AmUInt32 p = 0; p < frames / 2; p++)
        {
            const AmInt32 i = static_cast<AmInt32>(std::floor(std::sqrt(p / (frames / 2.0f)) * (frames / 2)));

            AmInt32 p2 = (i / (frames / 16));
            AmInt32 p1 = p2 - 1;
            AmInt32 p0 = p1 - 1;
            AmInt32 p3 = p2 + 1;

            if (p1 < 0)
                p1 = 0;
            if (p0 < 0)
                p0 = 0;
            if (p3 > 7)
                p3 = 7;

            const AmReal32 v = static_cast<AmReal32>((i % (frames / 16))) / (frames / 16.0f);
            buffer[p * 2] *= CatmullRom(v, m_parameters[p0 + 1], m_parameters[p1 + 1], m_parameters[p2 + 1], m_parameters[p3 + 1]);
        }

        memset(buffer + frames, 0, sizeof(AmReal32) * frames);

        MagPhase2Comp(buffer, frames / 2);
    }
} // namespace SparkyStudios::Audio::Amplitude
