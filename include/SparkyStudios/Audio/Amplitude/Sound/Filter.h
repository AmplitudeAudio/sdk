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

#ifndef SS_AMPLITUDE_AUDIO_FILTER_H
#define SS_AMPLITUDE_AUDIO_FILTER_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    class FilterInstance;

    class Filter
    {
        friend class FilterInstance;

    public:
        enum PARAM_TYPE
        {
            PARAM_FLOAT = 0,
            PARAM_INT,
            PARAM_BOOL
        };

        Filter() = default;
        virtual ~Filter() = default;

        virtual AmUInt32 GetParamCount();

        virtual AmString GetParamName(AmUInt32 index);

        virtual AmUInt32 GetParamType(AmUInt32 index);

        virtual AmReal32 GetParamMax(AmUInt32 index);

        virtual AmReal32 GetParamMin(AmUInt32 index);

        virtual FilterInstance* CreateInstance() = 0;
    };

    class FilterInstance
    {
    public:
        explicit FilterInstance(Filter* parent);
        virtual ~FilterInstance();

        AmResult Init(AmUInt32 numParams);

        virtual void AdvanceFrame(AmTime delta_time);

        virtual void Process(AmAudioSampleBuffer buffer, AmUInt64 frames, AmUInt64 bufferSize, AmUInt16 channels, AmUInt32 sampleRate);
        virtual void ProcessInterleaved(AmAudioSampleBuffer buffer, AmUInt64 frames, AmUInt64 bufferSize, AmUInt16 channels, AmUInt32 sampleRate);

        virtual void ProcessChannel(
            AmAudioSampleBuffer buffer, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate, bool isInterleaved);

        virtual AmAudioSample ProcessSample(AmAudioSample sample, AmUInt16 channel, AmUInt32 sampleRate);

        virtual AmReal32 GetFilterParameter(AmUInt32 attributeId);

        virtual void SetFilterParameter(AmUInt32 attributeId, AmReal32 value);

    protected:
        class Filter* m_parent;

        AmUInt32 m_numParams;
        AmUInt32 m_numParamsChanged;
        AmReal32Buffer m_parameters;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_FILTER_H
