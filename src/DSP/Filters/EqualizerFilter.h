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

#pragma once

#ifndef _AM_IMPLEMENTATION_DSP_FILTERS_EQUALIZER_FILTER_H
#define _AM_IMPLEMENTATION_DSP_FILTERS_EQUALIZER_FILTER_H

#include <DSP/Filters/FFTFilter.h>

namespace SparkyStudios::Audio::Amplitude
{
    class EqualizerFilter;

    class EqualizerFilterInstance : public FFTFilterInstance
    {
    public:
        explicit EqualizerFilterInstance(EqualizerFilter* parent);
        ~EqualizerFilterInstance() override = default;

    protected:
        void ProcessFFTChannel(SplitComplex& fft, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate) override;
    };

    class EqualizerFilter final : public FFTFilter
    {
        friend class EqualizerFilterInstance;

    public:
        enum ATTRIBUTE
        {
            ATTRIBUTE_WET = 0,
            ATTRIBUTE_BAND_1,
            ATTRIBUTE_BAND_2,
            ATTRIBUTE_BAND_3,
            ATTRIBUTE_BAND_4,
            ATTRIBUTE_BAND_5,
            ATTRIBUTE_BAND_6,
            ATTRIBUTE_BAND_7,
            ATTRIBUTE_BAND_8,
            ATTRIBUTE_LAST,
        };

        EqualizerFilter();

        AmResult Init(
            AmReal32 volume1 = 0.0f,
            AmReal32 volume2 = 0.0f,
            AmReal32 volume3 = 0.0f,
            AmReal32 volume4 = 0.0f,
            AmReal32 volume5 = 0.0f,
            AmReal32 volume6 = 0.0f,
            AmReal32 volume7 = 0.0f,
            AmReal32 volume8 = 0.0f);

        [[nodiscard]] AmUInt32 GetParamCount() const override;

        [[nodiscard]] AmString GetParamName(AmUInt32 index) const override;

        [[nodiscard]] AmUInt32 GetParamType(AmUInt32 index) const override;

        [[nodiscard]] AmReal32 GetParamMax(AmUInt32 index) const override;

        [[nodiscard]] AmReal32 GetParamMin(AmUInt32 index) const override;

        FilterInstance* CreateInstance() override;

        void DestroyInstance(FilterInstance* instance) override;

    private:
        AmReal32 _volume[8];
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_DSP_FILTERS_EQUALIZER_FILTER_H