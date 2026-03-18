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

#ifndef _AM_IMPLEMENTATION_DSP_FILTERS_COMPRESSOR_FILTER_H
#define _AM_IMPLEMENTATION_DSP_FILTERS_COMPRESSOR_FILTER_H

#include <SparkyStudios/Audio/Amplitude/DSP/Filter.h>

namespace SparkyStudios::Audio::Amplitude
{
    class CompressorFilter;

    class CompressorFilterInstance final : public FilterInstance
    {
    public:
        explicit CompressorFilterInstance(CompressorFilter* parent);
        ~CompressorFilterInstance() override = default;

        void SetParameter(AmUInt32 index, AmReal32 value) override;

    protected:
        AmAudioSample ProcessSample(AmAudioSample sample, AmUInt16 channel, AmUInt32 sampleRate) override;

    private:
        void UpdateCoefficients(AmUInt32 sampleRate);
        AmReal32 ComputeGainReduction(AmReal32 levelDb) const;

        AmReal32 _prevEnvelopeLinear[kAmMaxSupportedChannelCount];
        AmReal32 _prevGainDb[kAmMaxSupportedChannelCount];
        AmUInt32 _lastSampleRate;
        AmUInt32 _parameterVersion;
        AmUInt32 _lastParameterVersion;

        AmReal32 _attackCoeff;
        AmReal32 _releaseCoeff;
        AmReal32 _levelLpCoeff;
    };

    class CompressorFilter final : public Filter
    {
        friend class CompressorFilterInstance;

    public:
        enum ATTRIBUTE
        {
            ATTRIBUTE_WET = 0,
            ATTRIBUTE_THRESHOLD,
            ATTRIBUTE_RATIO,
            ATTRIBUTE_ATTACK,
            ATTRIBUTE_RELEASE,
            ATTRIBUTE_KNEE,
            ATTRIBUTE_MAKEUP_GAIN,
            ATTRIBUTE_LAST
        };

        CompressorFilter();
        ~CompressorFilter() override = default;

        AmResult Initialize(
            AmReal32 threshold = -20.0f,
            AmReal32 ratio = 4.0f,
            AmReal32 attack = 5.0f,
            AmReal32 release = 200.0f,
            AmReal32 knee = 0.0f,
            AmReal32 makeupGain = 0.0f);

        [[nodiscard]] AmUInt32 GetParameterCount() const override;

        [[nodiscard]] AmString GetParameterName(AmUInt32 index) const override;

        [[nodiscard]] eParameterType GetParameterType(AmUInt32 index) const override;

        [[nodiscard]] AmReal32 GetParameterMax(AmUInt32 index) const override;

        [[nodiscard]] AmReal32 GetParameterMin(AmUInt32 index) const override;

        std::shared_ptr<FilterInstance> CreateInstance() override;

    private:
        AmReal32 _threshold;
        AmReal32 _ratio;
        AmReal32 _attack;
        AmReal32 _release;
        AmReal32 _knee;
        AmReal32 _makeupGain;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_DSP_FILTERS_COMPRESSOR_FILTER_H
