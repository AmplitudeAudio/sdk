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

#ifndef SS_AMPLITUDE_AUDIO_FFT_FILTER_H
#define SS_AMPLITUDE_AUDIO_FFT_FILTER_H

#include <SparkyStudios/Audio/Amplitude/Sound/Filter.h>

#include <Utils/pffft/pffft.h>

namespace SparkyStudios::Audio::Amplitude
{
    class FFTFilterInstance;

    class FFTFilter : public Filter
    {
        friend class FFTFilterInstance;

    public:
        FFTFilter() = default;
        ~FFTFilter() override = default;

        FilterInstance* CreateInstance() override;
    };

    class FFTFilterInstance : public FilterInstance
    {
    public:
        explicit FFTFilterInstance(FFTFilter* parent);
        ~FFTFilterInstance() override;

        void ProcessChannel(
            AmInt16Buffer buffer, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate, bool isInterleaved) override;

        virtual void ProcessFFTChannel(AmReal32Buffer buffer, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate);

        void Comp2MagPhase(AmReal32Buffer buffer, AmUInt32 samples);
        void MagPhase2MagFreq(AmReal32Buffer buffer, AmUInt32 samples, AmUInt32 sampleRate, AmUInt16 channel);
        void MagFreq2MagPhase(AmReal32Buffer buffer, AmUInt32 samples, AmUInt32 sampleRate, AmUInt16 channel);

        static void MagPhase2Comp(AmReal32Buffer buffer, AmUInt32 samples);

        void InitFFT();

    private:
        PFFFT_Setup* _pffft_setup;

        AmReal32Buffer _temp;
        AmReal32Buffer _inputBuffer;
        AmReal32Buffer _mixBuffer;
        AmReal32Buffer _lastPhase;
        AmReal32Buffer _sumPhase;

        AmUInt32 _inputOffset[AM_MAX_CHANNELS];
        AmUInt32 _mixOffset[AM_MAX_CHANNELS];
        AmUInt32 _readOffset[AM_MAX_CHANNELS];
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_FFT_FILTER_H
