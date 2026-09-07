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

#include <DSP/Filters/EqualizerFilter.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    EqualizerFilter::EqualizerFilter()
        : FFTFilter("Equalizer")
        , _volume{}
    {
        for (float& i : _volume)
            i = 1.0f;
    }

    AmResult EqualizerFilter::Initialize(
        AmReal32 volume1,
        AmReal32 volume2,
        AmReal32 volume3,
        AmReal32 volume4,
        AmReal32 volume5,
        AmReal32 volume6,
        AmReal32 volume7,
        AmReal32 volume8)
    {
        if (volume1 < GetParameterMin(ATTRIBUTE_BAND_1) || volume1 > GetParameterMax(ATTRIBUTE_BAND_1))
            return eErrorCode_InvalidParameter;

        if (volume2 < GetParameterMin(ATTRIBUTE_BAND_2) || volume2 > GetParameterMax(ATTRIBUTE_BAND_2))
            return eErrorCode_InvalidParameter;

        if (volume3 < GetParameterMin(ATTRIBUTE_BAND_3) || volume3 > GetParameterMax(ATTRIBUTE_BAND_3))
            return eErrorCode_InvalidParameter;

        if (volume4 < GetParameterMin(ATTRIBUTE_BAND_4) || volume4 > GetParameterMax(ATTRIBUTE_BAND_4))
            return eErrorCode_InvalidParameter;

        if (volume5 < GetParameterMin(ATTRIBUTE_BAND_5) || volume5 > GetParameterMax(ATTRIBUTE_BAND_5))
            return eErrorCode_InvalidParameter;

        if (volume6 < GetParameterMin(ATTRIBUTE_BAND_6) || volume6 > GetParameterMax(ATTRIBUTE_BAND_6))
            return eErrorCode_InvalidParameter;

        if (volume7 < GetParameterMin(ATTRIBUTE_BAND_7) || volume7 > GetParameterMax(ATTRIBUTE_BAND_7))
            return eErrorCode_InvalidParameter;

        if (volume8 < GetParameterMin(ATTRIBUTE_BAND_8) || volume8 > GetParameterMax(ATTRIBUTE_BAND_8))
            return eErrorCode_InvalidParameter;

        _volume[ATTRIBUTE_BAND_1 - ATTRIBUTE_BAND_1] = volume1;
        _volume[ATTRIBUTE_BAND_2 - ATTRIBUTE_BAND_1] = volume2;
        _volume[ATTRIBUTE_BAND_3 - ATTRIBUTE_BAND_1] = volume3;
        _volume[ATTRIBUTE_BAND_4 - ATTRIBUTE_BAND_1] = volume4;
        _volume[ATTRIBUTE_BAND_5 - ATTRIBUTE_BAND_1] = volume5;
        _volume[ATTRIBUTE_BAND_6 - ATTRIBUTE_BAND_1] = volume6;
        _volume[ATTRIBUTE_BAND_7 - ATTRIBUTE_BAND_1] = volume7;
        _volume[ATTRIBUTE_BAND_8 - ATTRIBUTE_BAND_1] = volume8;

        return eErrorCode_Success;
    }

    AmUInt32 EqualizerFilter::GetParameterCount() const
    {
        return ATTRIBUTE_LAST;
    }

    AmString EqualizerFilter::GetParameterName(AmUInt32 index) const
    {
        if (index >= ATTRIBUTE_LAST)
            return "Unknown";

        static const AmString names[ATTRIBUTE_LAST] = {
            "Wet", "Band 1", "Band 2", "Band 3", "Band 4",
            "Band 5", "Band 6", "Band 7", "Band 8"
        };

        return names[index];
    }

    eParameterType EqualizerFilter::GetParameterType(AmUInt32 index) const
    {
        return eParameterType_Float;
    }

    AmReal32 EqualizerFilter::GetParameterMax(AmUInt32 index) const
    {
        if (index >= ATTRIBUTE_LAST)
            return 0.0f;

        static const AmReal32 values[ATTRIBUTE_LAST] = { 1.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f };

        return values[index];
    }

    AmReal32 EqualizerFilter::GetParameterMin(AmUInt32 index) const
    {
        if (index >= ATTRIBUTE_LAST)
            return 0.0f;

        static const AmReal32 values[ATTRIBUTE_LAST] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

        return values[index];
    }

    std::shared_ptr<FilterInstance> EqualizerFilter::CreateInstance()
    {
        return ampoolshared(eMemoryPoolKind_Filtering, EqualizerFilterInstance, this);
    }

    EqualizerFilterInstance::EqualizerFilterInstance(EqualizerFilter* parent)
        : FFTFilterInstance(parent)
    {
        Initialize(parent->GetParameterCount());

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
        SplitComplex& fft, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate)
    {
        Comp2MagPhase(fft, frames);

        const AmUInt32 bandSize = frames / 8;

        for (AmUInt32 p = 0, l = frames; p < l; p++)
        {
            const auto i = static_cast<AmUInt32>(std::floor(std::sqrt(p / static_cast<AmReal32>(frames)) * static_cast<AmReal32>(frames)));

            AmInt32 p2 = static_cast<AmInt32>(i / bandSize);
            AmInt32 p1 = p2 - 1;
            AmInt32 p0 = p1 - 1;
            AmInt32 p3 = p2 + 1;

            if (p1 < 0)
                p1 = 0;
            if (p0 < 0)
                p0 = 0;
            if (p2 > 7)
                p2 = 7;
            if (p3 > 7)
                p3 = 7;

            const AmReal32 v = static_cast<AmReal32>(i % bandSize) / static_cast<AmReal32>(bandSize);
            fft.re()[p] *= CatmullRom(v, m_parameters[p0 + 1], m_parameters[p1 + 1], m_parameters[p2 + 1], m_parameters[p3 + 1]);
        }

        MagPhase2Comp(fft, frames);
    }
} // namespace SparkyStudios::Audio::Amplitude
