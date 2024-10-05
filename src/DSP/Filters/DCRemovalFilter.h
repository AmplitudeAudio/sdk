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

#ifndef _AM_IMPLEMENTATION_DSP_FILTERS_DC_REMOVAL_FILTER_H
#define _AM_IMPLEMENTATION_DSP_FILTERS_DC_REMOVAL_FILTER_H

#include <SparkyStudios/Audio/Amplitude/DSP/Filter.h>

namespace SparkyStudios::Audio::Amplitude
{
    class DCRemovalFilter;

    class DCRemovalFilterInstance : public FilterInstance
    {
    public:
        explicit DCRemovalFilterInstance(DCRemovalFilter* parent);
        ~DCRemovalFilterInstance() override;

        void Process(const AudioBuffer& in, AudioBuffer& out, AmUInt64 frames, AmUInt32 sampleRate) override;

    protected:
        AmAudioSample ProcessSample(AmAudioSample sample, AmUInt16 channel, AmUInt32 sampleRate) override;

    private:
        void InitializeBuffer(AmUInt16 channels, AmUInt32 sampleRate);

        AmAlignedReal32Buffer _buffer;
        AmAlignedReal32Buffer _totals;
        AmUInt64 _bufferLength;
        AmUInt64 _offset;
    };

    class DCRemovalFilter final : public Filter
    {
        friend class DCRemovalFilterInstance;

    public:
        enum ATTRIBUTE
        {
            ATTRIBUTE_WET = 0,
            ATTRIBUTE_LENGTH = 1,
            ATTRIBUTE_LAST
        };

        DCRemovalFilter();

        AmResult Initialize(AmReal32 length = 0.1f);

        FilterInstance* CreateInstance() override;

        void DestroyInstance(FilterInstance* instance) override;

    private:
        AmReal32 _length;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_DSP_FILTERS_DC_REMOVAL_FILTER_H