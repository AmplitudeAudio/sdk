// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#ifndef _AM_SOUND_FILTERS_MONOPOLE_FILTER_H
#define _AM_SOUND_FILTERS_MONOPOLE_FILTER_H

#include <SparkyStudios/Audio/Amplitude/Sound/Filter.h>

namespace SparkyStudios::Audio::Amplitude
{
    class MonoPoleFilter;

    class MonoPoleFilterInstance : public FilterInstance
    {
    public:
        explicit MonoPoleFilterInstance(MonoPoleFilter* parent);
        ~MonoPoleFilterInstance() override = default;

        AmAudioSample ProcessSample(AmAudioSample sample, AmUInt16 channel, AmUInt32 sampleRate) override;

    private:
        AmAudioSample _previousSample = 0.0f;
    };

    class MonoPoleFilter : public Filter
    {
        friend class MonoPoleFilterInstance;

    public:
        enum ATTRIBUTE
        {
            ATTRIBUTE_WET = 0,
            ATTRIBUTE_COEFFICIENT,
            ATTRIBUTE_LAST
        };

        MonoPoleFilter();
        ~MonoPoleFilter() override = default;

        AmResult Init(AmReal32 coefficient);

        [[nodiscard]] AmUInt32 GetParamCount() const override;

        [[nodiscard]] AmString GetParamName(AmUInt32 index) const override;

        [[nodiscard]] AmUInt32 GetParamType(AmUInt32 index) const override;

        [[nodiscard]] AmReal32 GetParamMax(AmUInt32 index) const override;

        [[nodiscard]] AmReal32 GetParamMin(AmUInt32 index) const override;

        FilterInstance* CreateInstance() override;

        void DestroyInstance(FilterInstance* instance) override;

    private:
        AmReal32 _coefficient;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_SOUND_FILTERS_MONOPOLE_FILTER_H
