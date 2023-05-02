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

#include <Utils/pffft/pffft_double.h>

namespace SoLoud
{
    namespace FFT
    {
        // Perform 1024 unit FFT. Buffer must have 1024 floats, and will be overwritten
        void fft1024(float* aBuffer);

        // Perform 256 unit FFT. Buffer must have 256 floats, and will be overwritten
        void fft256(float* aBuffer);

        // Perform 256 unit IFFT. Buffer must have 256 floats, and will be overwritten
        void ifft256(float* aBuffer);

        // Generic (slower) power of two FFT. Buffer is overwritten.
        void fft(float* aBuffer, unsigned int aBufferLength);

        // Generic (slower) power of two IFFT. Buffer is overwritten.
        void ifft(float* aBuffer, unsigned int aBufferLength);
    }; // namespace FFT
}; // namespace SoLoud

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
            AmAudioSampleBuffer buffer, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate, bool isInterleaved)
            override;

        virtual void ProcessFFTChannel(AmReal64Buffer buffer, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate);

        void Comp2MagPhase(AmReal64Buffer buffer, AmUInt32 samples);
        void MagPhase2MagFreq(AmReal64Buffer buffer, AmUInt32 samples, AmUInt32 sampleRate, AmUInt16 channel);
        void MagFreq2MagPhase(AmReal64Buffer buffer, AmUInt32 samples, AmUInt32 sampleRate, AmUInt16 channel);

        static void MagPhase2Comp(AmReal64Buffer buffer, AmUInt32 samples);

        void InitFFT();

    private:
        PFFFTD_Setup* _pffft_setup;

        AmReal64Buffer _temp;
        AmReal64Buffer _inputBuffer;
        AmReal64Buffer _mixBuffer;
        AmReal64Buffer _lastPhase;
        AmReal64Buffer _sumPhase;

        AmUInt32 _inputOffset[AM_MAX_CHANNELS];
        AmUInt32 _mixOffset[AM_MAX_CHANNELS];
        AmUInt32 _readOffset[AM_MAX_CHANNELS];
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_FFT_FILTER_H
