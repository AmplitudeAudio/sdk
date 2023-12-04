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

#ifndef SS_AMPLITUDE_AUDIO_DELAY_FILTER_H
#define SS_AMPLITUDE_AUDIO_DELAY_FILTER_H

#include <SparkyStudios/Audio/Amplitude/Sound/Filter.h>

namespace SparkyStudios::Audio::Amplitude
{
    class DelayFilter;

    class DelayFilterInstance : public FilterInstance
    {
    public:
        explicit DelayFilterInstance(DelayFilter* parent);
        ~DelayFilterInstance() override;

        void Process(
            AmAudioSampleBuffer buffer, AmUInt64 frames, AmUInt64 bufferSize, AmUInt16 channels, AmUInt32 sampleRate) override;

        AmAudioSample ProcessSample(AmAudioSample sample, AmUInt16 channel, AmUInt32 sampleRate) override;

    private:
        void InitBuffer(AmUInt16 channels, AmUInt32 sampleRate);

        AmReal32Buffer _buffer;
        AmUInt32 _bufferLength;
        AmUInt32 _bufferMaxLength;
        AmUInt32 _bufferOffset;
        AmUInt32 _offset;
    };

    [[maybe_unused]] static class DelayFilter final : public Filter
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

        AmResult Init(AmReal32 delay, AmReal32 decay = 0.7f, AmReal32 delayStart = 0.0f);

        AmUInt32 GetParamCount() override;

        AmString GetParamName(AmUInt32 index) override;

        AmUInt32 GetParamType(AmUInt32 index) override;

        AmReal32 GetParamMax(AmUInt32 index) override;

        AmReal32 GetParamMin(AmUInt32 index) override;

        FilterInstance* CreateInstance() override;

        void DestroyInstance(FilterInstance* instance) override;

    protected:
        AmReal32 _delay;
        AmReal32 _decay;
        AmReal32 _delayStart; // Set this to false to produce echo
    } gDelayFilter; // NOLINT(cert-err58-cpp)
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_DELAY_FILTER_H
