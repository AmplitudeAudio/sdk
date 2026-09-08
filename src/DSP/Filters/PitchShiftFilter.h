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

#pragma once

#ifndef _AM_IMPLEMENTATION_DSP_FILTERS_PITCH_SHIFT_FILTER_H
#define _AM_IMPLEMENTATION_DSP_FILTERS_PITCH_SHIFT_FILTER_H

#include <DSP/Filters/FFTFilter.h>

namespace SparkyStudios::Audio::Amplitude
{
    class PitchShiftFilter;

    class PitchShiftFilterInstance final : public FFTFilterInstance
    {
    public:
        explicit PitchShiftFilterInstance(PitchShiftFilter* parent);
        ~PitchShiftFilterInstance() override = default;

    protected:
        void ProcessFFTChannel(SplitComplex& fft, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate) override;
    };

    /**
     * @brief Phase-vocoder pitch shifter, with the shift expressed in semitones.
     *
     * Each analysis frame is routed through the phase-vocoder helpers
     * (Comp2MagPhase -> MagPhase2MagFreq -> MagFreq2MagPhase -> MagPhase2Comp)
     * with every bin's true frequency multiplied by 2^(shift/12); magnitudes
     * are left untouched. Note this scales per-hop phase advances in place
     * rather than remapping bins, so the output carries hop-rate sidebands
     * around the shifted partials.
     */
    class PitchShiftFilter final : public FFTFilter
    {
        friend class PitchShiftFilterInstance;

    public:
        enum ATTRIBUTE
        {
            ATTRIBUTE_WET = 0,
            ATTRIBUTE_SHIFT,
            ATTRIBUTE_LAST
        };

        PitchShiftFilter();

        /**
         * @brief Initializes the filter with the given pitch shift.
         *
         * @param[in] shiftSemitones The pitch shift in semitones, in the range [-24, +24].
         *
         * @return @c eErrorCode_Success on success, @c eErrorCode_InvalidParameter when
         * the shift is out of range.
         */
        AmResult Initialize(AmReal32 shiftSemitones);

        [[nodiscard]] AmUInt32 GetParameterCount() const override;

        [[nodiscard]] AmString GetParameterName(AmUInt32 index) const override;

        [[nodiscard]] eParameterType GetParameterType(AmUInt32 index) const override;

        [[nodiscard]] AmReal32 GetParameterMax(AmUInt32 index) const override;

        [[nodiscard]] AmReal32 GetParameterMin(AmUInt32 index) const override;

        std::shared_ptr<FilterInstance> CreateInstance() override;

    protected:
        AmReal32 m_shiftSemitones;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_DSP_FILTERS_PITCH_SHIFT_FILTER_H
