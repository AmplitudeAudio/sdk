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

#ifndef _AM_IMPLEMENTATION_DSP_FILTERS_DELAY_FILTER_H
#define _AM_IMPLEMENTATION_DSP_FILTERS_DELAY_FILTER_H

#include <SparkyStudios/Audio/Amplitude/DSP/Filter.h>

namespace SparkyStudios::Audio::Amplitude
{
    class DelayFilter;

    class DelayFilterInstance : public FilterInstance
    {
    public:
        explicit DelayFilterInstance(DelayFilter* parent);
        ~DelayFilterInstance() override;

        void Process(const AudioBuffer& in, AudioBuffer& out, AmUInt64 frames, AmUInt32 sampleRate) override;

    protected:
        AmAudioSample ProcessSample(AmAudioSample sample, AmUInt16 channel, AmUInt32 sampleRate) override;

    private:
        void InitializeBuffer(AmUInt16 channels, AmUInt32 sampleRate);

        AmReal32Buffer _buffer;
        AmUInt32 _bufferLength;
        AmUInt32 _bufferMaxLength;
        AmUInt32 _bufferOffset;
        AmUInt32 _offset;
    };

    class DelayFilter final : public Filter
    {
        friend class DelayFilterInstance;

    public:
        enum ATTRIBUTE
        {
            ATTRIBUTE_WET = 0,
            ATTRIBUTE_DELAY,
            ATTRIBUTE_DECAY,
            ATTRIBUTE_DELAY_START,
            ATTRIBUTE_LAST
        };

        DelayFilter();
        ~DelayFilter() override = default;

        AmResult Initialize(AmReal32 delay, AmReal32 decay = 0.7f, AmReal32 delayStart = 0.0f);

        [[nodiscard]] AmUInt32 GetParamCount() const override;

        [[nodiscard]] AmString GetParamName(AmUInt32 index) const override;

        [[nodiscard]] AmUInt32 GetParamType(AmUInt32 index) const override;

        [[nodiscard]] AmReal32 GetParamMax(AmUInt32 index) const override;

        [[nodiscard]] AmReal32 GetParamMin(AmUInt32 index) const override;

        FilterInstance* CreateInstance() override;

        void DestroyInstance(FilterInstance* instance) override;

    protected:
        AmReal32 _delay;
        AmReal32 _decay;
        AmReal32 _delayStart; // Set this to 0 to produce echo
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_DSP_FILTERS_DELAY_FILTER_H
