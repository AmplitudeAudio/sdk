// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#include <DSP/Filters/PitchShiftFilter.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    PitchShiftFilter::PitchShiftFilter()
        : FFTFilter("PitchShift")
        , m_shiftSemitones(0.0f)
    {}

    AmResult PitchShiftFilter::Initialize(AmReal32 shiftSemitones)
    {
        if (shiftSemitones < -24.0f || shiftSemitones > 24.0f)
            return eErrorCode_InvalidParameter;

        m_shiftSemitones = shiftSemitones;
        return eErrorCode_Success;
    }

    AmUInt32 PitchShiftFilter::GetParameterCount() const
    {
        return ATTRIBUTE_LAST;
    }

    AmString PitchShiftFilter::GetParameterName(AmUInt32 index) const
    {
        if (index >= ATTRIBUTE_LAST)
            return "Unknown";

        static const AmString names[ATTRIBUTE_LAST] = { "Wet", "Pitch Shift" };

        return names[index];
    }

    eParameterType PitchShiftFilter::GetParameterType(AmUInt32 index) const
    {
        return eParameterType_Float;
    }

    AmReal32 PitchShiftFilter::GetParameterMax(AmUInt32 index) const
    {
        if (index >= ATTRIBUTE_LAST)
            return 0.0f;

        static const AmReal32 values[ATTRIBUTE_LAST] = { 1.0f, 24.0f };

        return values[index];
    }

    AmReal32 PitchShiftFilter::GetParameterMin(AmUInt32 index) const
    {
        if (index >= ATTRIBUTE_LAST)
            return 0.0f;

        static const AmReal32 values[ATTRIBUTE_LAST] = { 0.0f, -24.0f };

        return values[index];
    }

    std::shared_ptr<FilterInstance> PitchShiftFilter::CreateInstance()
    {
        return ampoolshared(eMemoryPoolKind_Filtering, PitchShiftFilterInstance, this);
    }

    PitchShiftFilterInstance::PitchShiftFilterInstance(PitchShiftFilter* parent)
        : FFTFilterInstance(parent)
    {
        Initialize(PitchShiftFilter::ATTRIBUTE_LAST);
        m_parameters[PitchShiftFilter::ATTRIBUTE_SHIFT] = parent->m_shiftSemitones;
    }

    void PitchShiftFilterInstance::ProcessFFTChannel(
        SplitComplex& fft, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate)
    {
        const auto samples = static_cast<AmUInt32>(frames);
        const AmReal32 ratio = std::pow(2.0f, m_parameters[PitchShiftFilter::ATTRIBUTE_SHIFT] / 12.0f);

        Comp2MagPhase(fft, samples);
        MagPhase2MagFreq(fft, samples, sampleRate, channel);

        // Scale every bin's true frequency; magnitudes (re[]) are untouched.
        for (AmUInt32 s = 0; s < samples; s++)
            fft.im()[s] *= ratio;

        MagFreq2MagPhase(fft, samples, sampleRate, channel);
        MagPhase2Comp(fft, samples);
    }
} // namespace SparkyStudios::Audio::Amplitude
