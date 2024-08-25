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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Math/FFT.h>

#include <DSP/Filters/FFTFilter.h>

#define STFT_WINDOW_SIZE 256 // must be power of two
#define STFT_WINDOW_HALF 128
#define STFT_WINDOW_TWICE 512

namespace SparkyStudios::Audio::Amplitude
{
    // Create a hamming window of STFT_WINDOW_SIZE samples in buffer
    AM_API_PRIVATE void hamming(AmReal32Buffer buffer)
    {
        for (int i = 0; i < STFT_WINDOW_SIZE; i++)
            buffer[i] = 0.54 - (0.46f * std::cos(2.0 * M_PI * (i / ((STFT_WINDOW_SIZE - 1) * 1.0))));
    }

    FFTFilter::FFTFilter(const std::string& name)
        : Filter(name)
    {}

    FilterInstance* FFTFilter::CreateInstance()
    {
        return ampoolnew(MemoryPoolKind::Filtering, FFTFilterInstance, this);
    }

    void FFTFilter::DestroyInstance(FilterInstance* instance)
    {
        ampooldelete(MemoryPoolKind::Filtering, FFTFilterInstance, (FFTFilterInstance*)instance);
    }

    FFTFilterInstance::FFTFilterInstance(FFTFilter* parent)
        : FilterInstance(parent)
    {
        InitializeFFT();
        Initialize(1);
    }

    FFTFilterInstance::~FFTFilterInstance()
    {
        ampoolfree(MemoryPoolKind::Filtering, _temp);
    }

    void FFTFilterInstance::InitializeFFT()
    {
        _temp = static_cast<AmReal32Buffer>(ampoolmalloc(MemoryPoolKind::Filtering, STFT_WINDOW_SIZE * sizeof(AmReal32)));
    }

    void FFTFilterInstance::ProcessChannel(const AudioBuffer& in, AudioBuffer& out, AmUInt16 channel, AmUInt64 frames, AmUInt32 sampleRate)
    {
        const auto& inChannel = in[channel];
        auto& outChannel = out[channel];

        AmUInt32 offset = 0;

        while (offset < frames)
        {
            AmUInt32 framesToProcess = frames - offset;
            if (framesToProcess > STFT_WINDOW_SIZE)
                framesToProcess = STFT_WINDOW_SIZE;

            for (AmUInt32 i = 0; i < framesToProcess; i++)
                _temp[i] = inChannel[i + offset];

            if (framesToProcess < STFT_WINDOW_SIZE)
                std::memset(_temp + framesToProcess, 0, sizeof(AmReal32) * (STFT_WINDOW_SIZE - framesToProcess));

            {
                FFT fft;
                SplitComplex sc;

                fft.Initialize(STFT_WINDOW_SIZE);

                fft.Forward(_temp, sc);

                ProcessFFTChannel(sc, channel, STFT_WINDOW_HALF, in.GetChannelCount(), sampleRate);

                fft.Backward(_temp, sc);
            }

            for (AmUInt32 i = 0; i < framesToProcess; i++)
            {
                const AmReal32 x = inChannel[i + offset];
                /* */ AmReal32 y = _temp[i];

                y = x + (y - x) * m_parameters[0];

                outChannel[i + offset] = static_cast<AmAudioSample>(y);
            }

            offset += framesToProcess;
        }
    }

    void FFTFilterInstance::Comp2MagPhase(SplitComplex& fft, AmUInt32 samples)
    {
        for (AmUInt32 s = 0; s < samples; s++)
        {
            const AmReal32 r = fft.re()[s];
            const AmReal32 i = fft.im()[s];

            fft.re()[s] = std::sqrt(r * r + i * i);
            fft.im()[s] = std::atan2(i, r);
        }
    }

    void FFTFilterInstance::MagPhase2MagFreq(SplitComplex& fft, AmUInt32 samples, AmUInt32 sampleRate, AmUInt16 channel)
    {
        const AmReal32 stepSize = static_cast<AmReal32>(samples) / static_cast<AmReal32>(sampleRate);
        const AmReal32 expect = (stepSize / static_cast<AmReal32>(samples)) * 2.0f * static_cast<AmReal32>(M_PI);
        const AmReal32 freqPerBin = static_cast<AmReal32>(sampleRate) / static_cast<AmReal32>(samples);

        for (AmUInt32 s = 0; s < samples; s++)
        {
            // get true frequency from synthesis arrays
            const AmReal32 pha = fft.im()[s];

            AmReal32 freq = pha;

            // subtract expected phase difference
            freq -= static_cast<AmReal32>(s) * expect;

            // map delta phase into +/- Pi interval
            auto qpd = static_cast<AmInt32>(std::floor(freq / static_cast<AmReal32>(M_PI)));

            if (qpd >= 0)
                qpd += qpd & 1;
            else
                qpd -= qpd & 1;

            freq -= static_cast<AmReal32>(M_PI) * static_cast<AmReal32>(qpd);

            // get deviation from bin frequency from the +/- Pi interval
            freq = static_cast<AmReal32>(samples) * freq / (2.0f * static_cast<AmReal32>(M_PI));

            // compute the k-th partials' true frequency
            freq = static_cast<AmReal32>(s) * freqPerBin + freq * freqPerBin;

            // store magnitude and true frequency in analysis arrays
            fft.im()[s] = freq;
        }
    }

    void FFTFilterInstance::MagFreq2MagPhase(SplitComplex& fft, AmUInt32 samples, AmUInt32 sampleRate, AmUInt16 channel)
    {
        const AmReal32 stepSize = static_cast<AmReal32>(samples) / static_cast<AmReal32>(sampleRate);
        const AmReal32 expect = (stepSize / static_cast<AmReal32>(samples)) * 2.0f * static_cast<AmReal32>(M_PI);
        const AmReal32 freqPerBin = static_cast<AmReal32>(sampleRate) / static_cast<AmReal32>(samples);

        for (AmUInt32 s = 0; s < samples; s++)
        {
            // get true frequency from synthesis arrays
            const AmReal32 freq = fft.im()[s];

            AmReal32 pha = freq;

            // subtract bin mid-frequency
            pha -= static_cast<AmReal32>(s) * freqPerBin;

            // get bin deviation from freq deviation
            pha /= freqPerBin;

            // take osamp into account
            pha = (pha / static_cast<AmReal32>(samples)) * static_cast<AmReal32>(M_PI) * 2.0f;

            // add the overlap phase advance back in
            pha += static_cast<AmReal32>(s) * expect;

            fft.im()[s] = pha;
        }
    }

    void FFTFilterInstance::MagPhase2Comp(SplitComplex& fft, AmUInt32 samples)
    {
        for (AmUInt32 s = 0; s < samples; s++)
        {
            const AmReal32 mag = fft.re()[s];
            const AmReal32 pha = fft.im()[s];

            fft.re()[s] = std::cos(pha) * mag;
            fft.im()[s] = std::sin(pha) * mag;
        }
    }

    void FFTFilterInstance::ProcessFFTChannel(SplitComplex& fft, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate)
    {
        Comp2MagPhase(fft, frames);
        MagPhase2MagFreq(fft, frames, sampleRate, channel);

        // Identity transform

        MagFreq2MagPhase(fft, frames, sampleRate, channel);
        MagPhase2Comp(fft, frames);
    }
} // namespace SparkyStudios::Audio::Amplitude
