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

#ifndef _AM_IMPLEMENTATION_DSP_FILTERS_LOFI_FILTER_H
#define _AM_IMPLEMENTATION_DSP_FILTERS_LOFI_FILTER_H

#include <SparkyStudios/Audio/Amplitude/DSP/Filter.h>

namespace SparkyStudios::Audio::Amplitude
{
    class LofiFilter;

    struct LofiChannelData
    {
        AmAudioSample m_sample;
        AmReal32 m_samplesToSkip;
    };

    class LofiFilterInstance : public FilterInstance
    {
    public:
        explicit LofiFilterInstance(LofiFilter* parent);
        ~LofiFilterInstance() override = default;

    protected:
        AmAudioSample ProcessSample(AmAudioSample sample, AmUInt16 channel, AmUInt32 sampleRate) override;

    private:
        LofiChannelData _channelData[kAmMaxSupportedChannelCount]{};
    };

    class LofiFilter final : public Filter
    {
        friend class LofiFilterInstance;

    public:
        enum ATTRIBUTE
        {
            ATTRIBUTE_WET,
            ATTRIBUTE_SAMPLERATE,
            ATTRIBUTE_BITDEPTH,
            ATTRIBUTE_LAST
        };

        LofiFilter();
        ~LofiFilter() override = default;

        AmResult Init(AmReal32 sampleRate, AmReal32 bitDepth);

        [[nodiscard]] AmUInt32 GetParamCount() const override;

        [[nodiscard]] AmString GetParamName(AmUInt32 index) const override;

        [[nodiscard]] AmUInt32 GetParamType(AmUInt32 index) const override;

        [[nodiscard]] AmReal32 GetParamMax(AmUInt32 index) const override;

        [[nodiscard]] AmReal32 GetParamMin(AmUInt32 index) const override;

        FilterInstance* CreateInstance() override;

        void DestroyInstance(FilterInstance* instance) override;

    private:
        AmReal32 _sampleRate;
        AmReal32 _bitDepth;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_DSP_FILTERS_LOFI_FILTER_H