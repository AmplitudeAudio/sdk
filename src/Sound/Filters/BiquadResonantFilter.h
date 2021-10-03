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

#ifndef SS_AMPLITUDE_AUDIO_BIQUAD_RESONANT_FILTER_H
#define SS_AMPLITUDE_AUDIO_BIQUAD_RESONANT_FILTER_H

#include <SparkyStudios/Audio/Amplitude/Sound/Filter.h>

namespace SparkyStudios::Audio::Amplitude
{
    class BiquadResonantFilterInstance;

    struct BiquadResonantStateData
    {
        AmInt32 y1, y2, x1, x2;
    };

    class BiquadResonantFilter : public Filter
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
            TYPE_LAST
        };

        BiquadResonantFilter();
        ~BiquadResonantFilter() override = default;

        AmResult Init(TYPE type, float frequency, float qOrS, float gain);

        AmResult InitLowPass(float frequency, float q);

        AmResult InitHighPass(float frequency, float q);

        AmResult InitBandPass(float frequency, float q);

        AmResult InitPeaking(float frequency, float q, float gain);

        AmResult InitNotching(float frequency, float q);

        AmResult InitLowShelf(float frequency, float s, float gain);

        AmResult InitHighShelf(float frequency, float s, float gain);

        AmUInt32 GetParamCount() override;

        AmString GetParamName(AmUInt32 index) override;

        AmUInt32 GetParamType(AmUInt32 index) override;

        float GetParamMax(AmUInt32 index) override;

        float GetParamMin(AmUInt32 index) override;

        FilterInstance* CreateInstance() override;

    private:
        AmInt32 _filterType;
        float _frequency;
        float _resonance;
        float _gain;
    };

    class BiquadResonantFilterInstance : public FilterInstance
    {
    public:
        explicit BiquadResonantFilterInstance(BiquadResonantFilter* parent);
        ~BiquadResonantFilterInstance() override = default;

        void ProcessFrame(AmInt16Buffer frame, AmUInt16 channels, AmUInt32 sampleRate) override;

        AmInt16 ProcessSample(AmInt16 sample, AmUInt16 channel, AmUInt32 sampleRate) override;

    private:
        void ComputeBiquadResonantParams();

        BiquadResonantStateData _state[AM_MAX_CHANNELS];
        AmInt32 _a0, _a1, _a2, _b1, _b2;
        bool _isDirty;
        AmUInt32 _sampleRate;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_BIQUAD_RESONANT_FILTER_H
