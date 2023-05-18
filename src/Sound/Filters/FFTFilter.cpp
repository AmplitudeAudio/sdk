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

#include <Sound/Filters/FFTFilter.h>

#define STFT_WINDOW_SIZE 256 // must be power of two
#define STFT_WINDOW_HALF (STFT_WINDOW_SIZE / 2)
#define STFT_WINDOW_TWICE (STFT_WINDOW_SIZE * 2)

namespace SparkyStudios::Audio::Amplitude
{
    FFTFilter::FFTFilter(const std::string& name)
        : Filter(name)
    {}

    FilterInstance* FFTFilter::CreateInstance()
    {
        return amnew(FFTFilterInstance, this);
    }

    void FFTFilter::DestroyInstance(FilterInstance* instance)
    {
        amdelete(FFTFilterInstance, (FFTFilterInstance*)instance);
    }

    FFTFilterInstance::FFTFilterInstance(FFTFilter* parent)
        : FilterInstance(parent)
    {
        InitFFT();
        Init(1);
    }

    FFTFilterInstance::~FFTFilterInstance()
    {
        pffftd_aligned_free(_temp);

        amMemory->Free(MemoryPoolKind::Filtering, _inputBuffer);
        amMemory->Free(MemoryPoolKind::Filtering, _mixBuffer);
        amMemory->Free(MemoryPoolKind::Filtering, _lastPhase);
        amMemory->Free(MemoryPoolKind::Filtering, _sumPhase);
    }

    void FFTFilterInstance::InitFFT()
    {
        _temp = static_cast<AmReal64*>(pffftd_aligned_malloc(STFT_WINDOW_SIZE));

        _inputBuffer = nullptr;
        _mixBuffer = nullptr;
        _lastPhase = nullptr;
        _sumPhase = nullptr;

        for (AmUInt16 i = 0; i < AM_MAX_CHANNELS; i++)
        {
            _inputOffset[i] = STFT_WINDOW_SIZE;
            _mixOffset[i] = STFT_WINDOW_HALF;
            _readOffset[i] = 0;
        }
    }

    void FFTFilterInstance::ProcessChannel(
        AmAudioSampleBuffer buffer, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate, bool isInterleaved)
    {
        // Chicken-egg problem: we don't know channel count before this.
        // Could allocate max_channels but that would potentially waste a lot of memory.
        if (_inputBuffer == nullptr)
        {
            _inputBuffer =
                static_cast<AmReal64Buffer>(amMemory->Malloc(MemoryPoolKind::Filtering, STFT_WINDOW_TWICE * channels * sizeof(AmReal64)));
            _mixBuffer =
                static_cast<AmReal64Buffer>(amMemory->Malloc(MemoryPoolKind::Filtering, STFT_WINDOW_TWICE * channels * sizeof(AmReal64)));
            _lastPhase =
                static_cast<AmReal64Buffer>(amMemory->Malloc(MemoryPoolKind::Filtering, STFT_WINDOW_SIZE * channels * sizeof(AmReal64)));
            _sumPhase =
                static_cast<AmReal64Buffer>(amMemory->Malloc(MemoryPoolKind::Filtering, STFT_WINDOW_SIZE * channels * sizeof(AmReal64)));

            std::memset(_inputBuffer, 0, sizeof(AmReal64) * STFT_WINDOW_TWICE * channels);
            std::memset(_mixBuffer, 0, sizeof(AmReal64) * STFT_WINDOW_TWICE * channels);
            std::memset(_lastPhase, 0, sizeof(AmReal64) * STFT_WINDOW_SIZE * channels);
            std::memset(_sumPhase, 0, sizeof(AmReal64) * STFT_WINDOW_SIZE * channels);
        }

        AmUInt32 offset = 0;
        AmUInt32 channelOffset = STFT_WINDOW_TWICE * channel;
        AmUInt32 inputOffset = _inputOffset[channel];
        AmUInt32 mixOffset = _mixOffset[channel];
        AmUInt32 readOffset = _readOffset[channel];
        AmUInt64 maxFrames = isInterleaved ? frames * channels : frames;

        while (offset < maxFrames)
        {
            AmUInt32 samples = STFT_WINDOW_HALF - (inputOffset & (STFT_WINDOW_HALF - 1));

            if (isInterleaved)
                samples *= channels;

            if (offset + samples > maxFrames)
                samples = frames - offset;

            for (AmUInt32 i = channel; i < samples; i += isInterleaved ? channels : 1)
            {
                _inputBuffer[channelOffset + ((inputOffset + STFT_WINDOW_HALF) & (STFT_WINDOW_TWICE - 1))] = buffer[offset + i];
                _mixBuffer[channelOffset + ((inputOffset + STFT_WINDOW_HALF) & (STFT_WINDOW_TWICE - 1))] = 0;
                inputOffset++;
            }

            if ((inputOffset & (STFT_WINDOW_HALF - 1)) == 0)
            {
                for (AmUInt32 i = 0; i < STFT_WINDOW_SIZE; i++)
                    _temp[i] =
                        _inputBuffer[channelOffset + ((inputOffset + STFT_WINDOW_TWICE - STFT_WINDOW_HALF + i) & (STFT_WINDOW_TWICE - 1))];

                _pffft_setup = pffftd_new_setup(STFT_WINDOW_HALF, PFFFT_COMPLEX);

                pffftd_transform_ordered(_pffft_setup, _temp, _temp, nullptr, PFFFT_FORWARD);

                // do magic
                ProcessFFTChannel(_temp, channel, STFT_WINDOW_HALF, channels, sampleRate);

                pffftd_transform_ordered(_pffft_setup, _temp, _temp, nullptr, PFFFT_BACKWARD);

                pffftd_destroy_setup(_pffft_setup);

                for (AmUInt32 i = 0; i < STFT_WINDOW_SIZE; i++)
                {
                    _temp[i] *= 1.0 / static_cast<AmReal64>(STFT_WINDOW_HALF);

                    _mixBuffer[channelOffset + (mixOffset & (STFT_WINDOW_TWICE - 1))] += _temp[i] *
                        (static_cast<AmReal64>(STFT_WINDOW_HALF) - std::abs(static_cast<AmReal64>(STFT_WINDOW_HALF - i))) *
                        (1.0 / static_cast<AmReal64>(STFT_WINDOW_HALF));

                    mixOffset++;
                }

                mixOffset -= STFT_WINDOW_HALF;
            }

            for (AmUInt32 i = channel; i < samples; i += isInterleaved ? channels : 1)
            {
                const AmReal32 x = buffer[offset + i];
                /* */ AmReal32 y = static_cast<AmReal32>(_mixBuffer[channelOffset + (readOffset & (STFT_WINDOW_TWICE - 1))]);

                y = x + (y - x) * m_parameters[0];
                y = AM_CLAMP_AUDIO_SAMPLE(y);

                buffer[offset + i] = static_cast<AmAudioSample>(y);

                readOffset++;
            }

            offset += samples;
        }

        _inputOffset[channel] = inputOffset;
        _readOffset[channel] = readOffset;
        _mixOffset[channel] = mixOffset;
    }

    void FFTFilterInstance::Comp2MagPhase(AmReal64Buffer buffer, AmUInt32 samples)
    {
        for (AmUInt32 i = 0; i < samples; i++)
        {
            AmReal64 re = buffer[i * 2];
            AmReal64 im = buffer[i * 2 + 1];

            buffer[i * 2] = std::sqrt(re * re + im * im) * 2.0;
            buffer[i * 2 + 1] = std::atan2(im, re);
        }
    }

    void FFTFilterInstance::MagPhase2MagFreq(AmReal64Buffer buffer, AmUInt32 samples, AmUInt32 sampleRate, AmUInt16 channel)
    {
        const AmReal64 stepSize = static_cast<AmReal64>(samples) / sampleRate;
        const AmReal64 expect = (stepSize / samples) * 2.0f * M_PI;
        const AmReal64 freqPerBin = static_cast<AmReal64>(sampleRate) / samples;

        for (AmUInt32 i = 0; i < samples; i++)
        {
            // get true frequency from synthesis arrays
            AmReal64 pha = buffer[i * 2 + 1];

            // compute phase difference
            AmReal64 freq = pha - _lastPhase[i + channel * STFT_WINDOW_SIZE];
            _lastPhase[i + channel * STFT_WINDOW_SIZE] = pha;

            /* subtract expected phase difference */
            freq -= static_cast<AmReal64>(i) * expect;

            /* map delta phase into +/- Pi interval */
            auto qpd = static_cast<AmInt32>(std::floor(freq / static_cast<AmReal64>(M_PI)));
            if (qpd >= 0)
                qpd += qpd & 1;
            else
                qpd -= qpd & 1;

            freq -= M_PI * qpd;

            /* get deviation from bin frequency from the +/- Pi interval */
            freq = samples * freq / (2.0 * M_PI);

            /* compute the k-th partials' true frequency */
            freq = static_cast<AmReal64>(i) * freqPerBin + freq * freqPerBin;

            /* store magnitude and true frequency in analysis arrays */
            buffer[i * 2 + 1] = freq;
        }
    }

    void FFTFilterInstance::MagFreq2MagPhase(AmReal64Buffer buffer, AmUInt32 samples, AmUInt32 sampleRate, AmUInt16 channel)
    {
        const AmReal64 stepSize = static_cast<AmReal64>(samples) / sampleRate;
        const AmReal64 expect = (stepSize / samples) * 2.0f * static_cast<AmReal64>(M_PI);
        const AmReal64 freqPerBin = static_cast<AmReal64>(sampleRate) / samples;

        for (AmUInt32 i = 0; i < samples; i++)
        {
            // get true frequency from synthesis arrays
            AmReal64 freq = buffer[i * 2 + 1];

            // subtract bin mid-frequency
            freq -= static_cast<AmReal64>(i) * freqPerBin;

            // get bin deviation from freq deviation
            freq /= freqPerBin;

            // take osamp into account
            freq = (freq / samples) * M_PI * 2.0;

            // add the overlap phase advance back in
            freq += static_cast<AmReal64>(i) * expect;

            // accumulate delta phase to get bin phase

            _sumPhase[i + channel * STFT_WINDOW_SIZE] += freq;
            buffer[i * 2 + 1] = _sumPhase[i + channel * STFT_WINDOW_SIZE];
        }
    }

    void FFTFilterInstance::MagPhase2Comp(AmReal64Buffer buffer, AmUInt32 samples)
    {
        for (AmUInt32 i = 0; i < samples; i++)
        {
            const AmReal64 mag = buffer[i * 2];
            const AmReal64 pha = buffer[i * 2 + 1];

            buffer[i * 2] = std::cos(pha) * mag;
            buffer[i * 2 + 1] = std::sin(pha) * mag;
        }
    }

    void FFTFilterInstance::ProcessFFTChannel(
        AmReal64Buffer buffer, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate)
    {
        Comp2MagPhase(buffer, frames);
        MagPhase2MagFreq(buffer, frames, sampleRate, channel);

        AmReal64 t[STFT_WINDOW_TWICE];
        std::memcpy(t, buffer, sizeof(AmReal64) * frames);
        std::memset(buffer, 0, sizeof(AmReal64) * frames * 2);

        for (AmUInt32 i = 0; i < frames / 4; i++)
        {
            AmUInt32 d = i * 2;
            if (d < frames / 4)
            {
                buffer[d * 2] += t[i * 2];
                buffer[d * 2 + 1] = t[i * 2 + 1] * 2;
            }
        }

        MagFreq2MagPhase(buffer, frames, sampleRate, channel);
        MagPhase2Comp(buffer, frames);
    }
} // namespace SparkyStudios::Audio::Amplitude
