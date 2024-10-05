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

#ifndef _AM_IMPLEMENTATION_DSP_FILTERS_BIQUAD_RESONANT_FILTER_H
#define _AM_IMPLEMENTATION_DSP_FILTERS_BIQUAD_RESONANT_FILTER_H

#include <SparkyStudios/Audio/Amplitude/DSP/Filter.h>

namespace SparkyStudios::Audio::Amplitude
{
    class BiquadResonantFilter;

    struct BiquadResonantStateData
    {
        AmReal32 y1, y2, x1, x2;
    };

    class BiquadResonantFilterInstance : public FilterInstance
    {
    public:
        explicit BiquadResonantFilterInstance(BiquadResonantFilter* parent);
        ~BiquadResonantFilterInstance() override = default;

    protected:
        void ProcessChannel(const AudioBuffer& in, AudioBuffer& out, AmUInt16 channel, AmUInt64 frames, AmUInt32 sampleRate) override;

        AmAudioSample ProcessSample(AmAudioSample sample, AmUInt16 channel, AmUInt32 sampleRate) override;

    private:
        void ComputeBiquadResonantParams();

        BiquadResonantStateData _state[kAmMaxSupportedChannelCount];
        AmReal32 _a0, _a1, _a2, _b1, _b2;
        AmUInt32 _sampleRate;
    };

    class BiquadResonantFilter final : public Filter
    {
        friend class BiquadResonantFilterInstance;

    public:
        enum ATTRIBUTE
        {
            ATTRIBUTE_WET = 0,
            ATTRIBUTE_TYPE,
            ATTRIBUTE_FREQUENCY,
            ATTRIBUTE_RESONANCE,
            ATTRIBUTE_GAIN,
            ATTRIBUTE_LAST
        };

        enum TYPE
        {
            TYPE_LOW_PASS = 0,
            TYPE_HIGH_PASS,
            TYPE_BAND_PASS,
            TYPE_PEAK,
            TYPE_NOTCH,
            TYPE_LOW_SHELF,
            TYPE_HIGH_SHELF,
            TYPE_DUAL_BAND_LOW_PASS,
            TYPE_DUAL_BAND_HIGH_PASS,
            TYPE_LAST
        };

        BiquadResonantFilter();
        ~BiquadResonantFilter() override = default;

        AmResult Initialize(TYPE type, AmReal32 frequency, AmReal32 qOrS, AmReal32 gain);

        AmResult InitializeLowPass(AmReal32 frequency, AmReal32 q);

        AmResult InitializeHighPass(AmReal32 frequency, AmReal32 q);

        AmResult InitializeBandPass(AmReal32 frequency, AmReal32 q);

        AmResult InitializePeaking(AmReal32 frequency, AmReal32 q, AmReal32 gain);

        AmResult InitializeNotching(AmReal32 frequency, AmReal32 q);

        AmResult InitializeLowShelf(AmReal32 frequency, AmReal32 s, AmReal32 gain);

        AmResult InitializeHighShelf(AmReal32 frequency, AmReal32 s, AmReal32 gain);

        AmResult InitializeDualBandLowPass(AmReal32 frequency);

        AmResult InitializeDualBandHighPass(AmReal32 frequency);

        [[nodiscard]] AmUInt32 GetParamCount() const override;

        [[nodiscard]] AmString GetParamName(AmUInt32 index) const override;

        [[nodiscard]] AmUInt32 GetParamType(AmUInt32 index) const override;

        [[nodiscard]] AmReal32 GetParamMax(AmUInt32 index) const override;

        [[nodiscard]] AmReal32 GetParamMin(AmUInt32 index) const override;

        FilterInstance* CreateInstance() override;

        void DestroyInstance(FilterInstance* instance) override;

    private:
        AmInt32 _filterType;
        AmReal32 _frequency;
        AmReal32 _resonance;
        AmReal32 _gain;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_DSP_FILTERS_BIQUAD_RESONANT_FILTER_H
