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

#ifndef SS_AMPLITUDE_AUDIO_FLANGERFILTER_H
#define SS_AMPLITUDE_AUDIO_FLANGERFILTER_H

#include <SparkyStudios/Audio/Amplitude/Sound/Filter.h>

namespace SparkyStudios::Audio::Amplitude
{
    class FlangerFilter;

    class FlangerFilterInstance : public FilterInstance
    {
    public:
        explicit FlangerFilterInstance(FlangerFilter* parent);
        ~FlangerFilterInstance() override;

        void Process(AmAudioSampleBuffer buffer, AmUInt64 frames, AmUInt64 bufferSize, AmUInt16 channels, AmUInt32 sampleRate) override;

        void ProcessChannel(AmAudioSampleBuffer buffer, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate) override;

    private:
        void InitBuffer(AmUInt16 channels, AmUInt32 sampleRate);

        AmReal32Buffer _buffer;
        AmUInt32 _bufferLength;
        AmUInt32 _bufferOffset;
        AmUInt32 _offset;
        AmReal64 _index;
    };

    class FlangerFilter final : public Filter
    {
        friend class FlangerFilterInstance;

    public:
        enum ATTRIBUTE
        {
            ATTRIBUTE_WET = 0,
            ATTRIBUTE_DELAY,
            ATTRIBUTE_FREQUENCY,
            ATTRIBUTE_LAST
        };

        FlangerFilter();
        ~FlangerFilter() override = default;

        AmResult Init(AmReal32 delay, AmReal32 frequency);

        [[nodiscard]] AmUInt32 GetParamCount() const override;

        [[nodiscard]] AmString GetParamName(AmUInt32 index) const override;

        [[nodiscard]] AmUInt32 GetParamType(AmUInt32 index) const override;

        [[nodiscard]] AmReal32 GetParamMax(AmUInt32 index) const override;

        [[nodiscard]] AmReal32 GetParamMin(AmUInt32 index) const override;

        FilterInstance* CreateInstance() override;

        void DestroyInstance(FilterInstance* instance) override;

    private:
        AmReal32 _delay;
        AmReal32 _frequency;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_FLANGERFILTER_H