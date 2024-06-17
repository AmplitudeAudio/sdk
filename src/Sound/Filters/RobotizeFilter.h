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

#ifndef SS_AMPLITUDE_AUDIO_ROBOTIZE_FILTER_H
#define SS_AMPLITUDE_AUDIO_ROBOTIZE_FILTER_H

#include <SparkyStudios/Audio/Amplitude/Sound/Filter.h>

namespace SparkyStudios::Audio::Amplitude
{
    class RobotizeFilter;

    class RobotizeFilterInstance : public FilterInstance
    {
    public:
        explicit RobotizeFilterInstance(RobotizeFilter* parent);

        void AdvanceFrame(AmTime deltaTime) override;

        void ProcessChannel(AmAudioSampleBuffer buffer, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate) override;

    private:
        static AmReal32 GenerateWaveform(AmInt32 waveform, AmReal32 p);

        AmTime _duration;
    };

    class RobotizeFilter final : public Filter
    {
        friend class RobotizeFilterInstance;

    public:
        enum ATTRIBUTES
        {
            ATTRIBUTE_WET = 0,
            ATTRIBUTE_FREQUENCY,
            ATTRIBUTE_WAVEFORM,
            ATTRIBUTE_LAST
        };

        enum WAVEFORM
        {
            WAVE_SQUARE = 0,
            WAVE_SAW,
            WAVE_SIN,
            WAVE_TRIANGLE,
            WAVE_BOUNCE,
            WAVE_JAWS,
            WAVE_HUMPS,
            WAVE_FSQUARE,
            WAVE_FSAW,
            WAVE_LAST
        };

        RobotizeFilter();

        AmResult Init(AmReal32 frequency, AmInt32 waveform);

        [[nodiscard]] AmUInt32 GetParamCount() const override;

        [[nodiscard]] AmString GetParamName(AmUInt32 index) const override;

        [[nodiscard]] AmUInt32 GetParamType(AmUInt32 index) const override;

        [[nodiscard]] AmReal32 GetParamMax(AmUInt32 index) const override;

        [[nodiscard]] AmReal32 GetParamMin(AmUInt32 index) const override;

        FilterInstance* CreateInstance() override;

        void DestroyInstance(FilterInstance* instance) override;

    protected:
        AmReal32 m_frequency;
        AmInt32 m_waveform;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_ROBOTIZE_FILTER_H
