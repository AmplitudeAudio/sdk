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

#ifndef SS_AMPLITUDE_AUDIO_WAVESHARPERFILTER_H
#define SS_AMPLITUDE_AUDIO_WAVESHARPERFILTER_H

#include <SparkyStudios/Audio/Amplitude/Sound/Filter.h>

namespace SparkyStudios::Audio::Amplitude
{
    class WaveShaperFilterInstance;

    class WaveShaperFilter : public Filter
    {
        friend class WaveShaperFilterInstance;

    public:
        enum ATTRIBUTE
        {
            ATTRIBUTE_WET = 0,
            ATTRIBUTE_AMOUNT,
            ATTRIBUTE_LAST
        };

        WaveShaperFilter();
        ~WaveShaperFilter() override = default;

        AmResult Init(AmReal32 amount);

        AmUInt32 GetParamCount() override;

        AmString GetParamName(AmUInt32 index) override;

        AmUInt32 GetParamType(AmUInt32 index) override;

        AmReal32 GetParamMax(AmUInt32 index) override;

        AmReal32 GetParamMin(AmUInt32 index) override;

        FilterInstance* CreateInstance() override;

    private:
        AmReal32 _amount;
    };

    class WaveShaperFilterInstance : public FilterInstance
    {
    public:
        explicit WaveShaperFilterInstance(WaveShaperFilter* parent);
        ~WaveShaperFilterInstance() override = default;

        AmInt16 ProcessSample(AmInt16 sample, AmUInt16 channel, AmUInt32 sampleRate) override;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_WAVESHARPERFILTER_H