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

#ifndef SS_AMPLITUDE_AUDIO_BASS_BOOST_FILTER_H
#define SS_AMPLITUDE_AUDIO_BASS_BOOST_FILTER_H

#include <Sound/Filters/FFTFilter.h>

namespace SparkyStudios::Audio::Amplitude
{
    class BassBoostFilter;

    class BassBoostFilterInstance : public FFTFilterInstance
    {
    public:
        explicit BassBoostFilterInstance(BassBoostFilter* parent);
        ~BassBoostFilterInstance() override = default;

        void ProcessFFTChannel(
            AmReal32Buffer re, AmReal32Buffer im, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate) override;
    };

    [[maybe_unused]] static class BassBoostFilter final : public FFTFilter
    {
        friend class BassBoostFilterInstance;

    public:
        enum ATTRIBUTE
        {
            ATTRIBUTE_WET = 0,
            ATTRIBUTE_BOOST,
            ATTRIBUTE_LAST
        };

        BassBoostFilter();

        AmResult Init(AmReal32 boost);

        AmUInt32 GetParamCount() override;

        AmString GetParamName(AmUInt32 index) override;

        AmUInt32 GetParamType(AmUInt32 index) override;

        AmReal32 GetParamMax(AmUInt32 index) override;

        AmReal32 GetParamMin(AmUInt32 index) override;

        FilterInstance* CreateInstance() override;

        void DestroyInstance(FilterInstance* instance) override;

    protected:
        AmReal32 m_boost;
    } gBassBoostFilter; // NOLINT(cert-err58-cpp)
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_BASS_BOOST_FILTER_H
