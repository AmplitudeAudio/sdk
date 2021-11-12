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

#include <Sound/Filters/FFTFilter.h>
#include <Utils/Utils.h>

#define STFT_WINDOW_SIZE 256 // must be power of two
#define STFT_WINDOW_HALF (STFT_WINDOW_SIZE / 2)
#define STFT_WINDOW_TWICE (STFT_WINDOW_SIZE * 2)

namespace SparkyStudios::Audio::Amplitude
{
    FilterInstance* FFTFilter::CreateInstance()
    {
        return new FFTFilterInstance(this);
    }

    FFTFilterInstance::FFTFilterInstance(FFTFilter* parent)
        : FilterInstance(parent)
    {
        InitFFT();
        Init(1);
    }

    FFTFilterInstance::~FFTFilterInstance()
    {
        pffft_aligned_free(_temp);

        delete[] _inputBuffer;
        delete[] _mixBuffer;
        delete[] _lastPhase;
        delete[] _sumPhase;
    }

    void FFTFilterInstance::InitFFT()
    {
        _temp = static_cast<AmReal32*>(pffft_aligned_malloc(STFT_WINDOW_SIZE));

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
        AmInt16Buffer buffer, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate, bool isInterleaved)
    {
        // Chicken-egg problem: we don't know channel count before this.
        // Could allocate max_channels but that would potentially waste a lot of memory.
        if (_inputBuffer == nullptr)
        {
            _inputBuffer = new AmReal32[STFT_WINDOW_TWICE * channels];
            _mixBuffer = new AmReal32[STFT_WINDOW_TWICE * channels];
            _lastPhase = new AmReal32[STFT_WINDOW_SIZE * channels];
            _sumPhase = new AmReal32[STFT_WINDOW_SIZE * channels];

            memset(_inputBuffer, 0, sizeof(AmReal32) * STFT_WINDOW_TWICE * channels);
            memset(_mixBuffer, 0, sizeof(AmReal32) * STFT_WINDOW_TWICE * channels);
            memset(_lastPhase, 0, sizeof(AmReal32) * STFT_WINDOW_SIZE * channels);
            memset(_sumPhase, 0, sizeof(AmReal32) * STFT_WINDOW_SIZE * channels);
        }

        AmUInt32 offset = 0;
        AmUInt32 channelOffset = STFT_WINDOW_TWICE * channel;
        AmUInt32 inputOffset = _inputOffset[channel];
        AmUInt32 mixOffset = _mixOffset[channel];
        AmUInt32 readOffset = _readOffset[channel];

        while (offset < (isInterleaved ? frames * channels : frames))
        {
            AmUInt32 samples = STFT_WINDOW_HALF - (inputOffset & (STFT_WINDOW_HALF - 1));

            if ((offset + static_cast<AmUInt64>(isInterleaved ? samples * channels : samples)) >
                (isInterleaved ? frames * channels : frames))
                samples = frames - (isInterleaved ? offset / channels : offset);

            for (AmUInt32 i = 0; i < samples; i++)
            {
                const AmUInt64 s = isInterleaved ? i * channels + channel : i + channel * frames;

                _inputBuffer[channelOffset + ((inputOffset + STFT_WINDOW_HALF) & (STFT_WINDOW_TWICE - 1))] =
                    AmInt16ToReal32(buffer[offset + s]);

                _mixBuffer[channelOffset + ((inputOffset + STFT_WINDOW_HALF) & (STFT_WINDOW_TWICE - 1))] = 0;

                inputOffset++;
            }

            if ((inputOffset & (STFT_WINDOW_HALF - 1)) == 0)
            {
                for (AmUInt32 i = 0; i < STFT_WINDOW_SIZE; i++)
                {
                    _temp[i] =
                        _inputBuffer[channelOffset + ((inputOffset + STFT_WINDOW_TWICE - STFT_WINDOW_HALF + i) & (STFT_WINDOW_TWICE - 1))];
                }

                _pffft_setup = pffft_new_setup(STFT_WINDOW_HALF, PFFFT_COMPLEX);

                pffft_transform_ordered(_pffft_setup, _temp, _temp, nullptr, PFFFT_FORWARD);

                // do magic
                ProcessFFTChannel(_temp, channel, STFT_WINDOW_HALF, channels, sampleRate);

                pffft_transform_ordered(_pffft_setup, _temp, _temp, nullptr, PFFFT_BACKWARD);

                pffft_destroy_setup(_pffft_setup);

                for (AmUInt32 i = 0; i < STFT_WINDOW_SIZE; i++)
                {
                    _temp[i] *= 1.0f / STFT_WINDOW_HALF;

                    _mixBuffer[channelOffset + (mixOffset & (STFT_WINDOW_TWICE - 1))] += _temp[i] *
                        (static_cast<AmReal32>(STFT_WINDOW_HALF) - std::abs(static_cast<AmInt32>(STFT_WINDOW_HALF - i))) *
                        (1.0f / static_cast<AmReal32>(STFT_WINDOW_HALF));

                    mixOffset++;
                }

                mixOffset -= STFT_WINDOW_HALF;
            }

            for (AmUInt32 i = 0; i < samples; i++)
            {
                const AmUInt64 s = isInterleaved ? i * channels + channel : i + channel * frames;

                const AmInt32 x = buffer[offset + s];
                /* */ AmInt32 y = AmReal32ToInt16(_mixBuffer[channelOffset + (readOffset & (STFT_WINDOW_TWICE - 1))]);

                y = x + ((y - x) * AmFloatToFixedPoint(m_parameters[0]) >> kAmFixedPointShift);
                y = AM_CLAMP(y, INT16_MIN, INT16_MAX);

                buffer[offset + s] = static_cast<AmInt16>(y);

                readOffset++;
            }

            offset += isInterleaved ? samples * channels : samples;
        }

        _inputOffset[channel] = inputOffset;
        _readOffset[channel] = readOffset;
        _mixOffset[channel] = mixOffset;
    }

    void FFTFilterInstance::Comp2MagPhase(AmReal32Buffer buffer, AmUInt32 samples)
    {
        for (AmUInt32 i = 0; i < samples; i++)
        {
            AmReal32 re = buffer[i * 2];
            AmReal32 im = buffer[i * 2 + 1];

            buffer[i * 2] = std::sqrt(re * re + im * im) * 2.0f;
            buffer[i * 2 + 1] = std::atan2(im, re);
        }
    }

    void FFTFilterInstance::MagPhase2MagFreq(AmReal32Buffer buffer, AmUInt32 samples, AmUInt32 sampleRate, AmUInt16 channel)
    {
        const AmReal32 stepsize = static_cast<AmReal32>(samples) / sampleRate;
        const AmReal32 expect = (stepsize / samples) * 2.0f * M_PI;
        const AmReal32 freqPerBin = static_cast<AmReal32>(sampleRate) / samples;

        for (AmUInt32 i = 0; i < samples; i++)
        {
            AmReal32 mag = buffer[i * 2];
            AmReal32 pha = buffer[i * 2 + 1];

            /* compute phase difference */
            AmReal32 freq = pha - _lastPhase[i + channel * STFT_WINDOW_SIZE];
            _lastPhase[i + channel * STFT_WINDOW_SIZE] = pha;

            /* subtract expected phase difference */
            freq -= static_cast<AmReal32>(i) * expect;

            /* map delta phase into +/- Pi interval */
            AmInt32 qpd = static_cast<AmInt32>(std::floor(freq / M_PI));
            if (qpd >= 0)
                qpd += qpd & 1;
            else
                qpd -= qpd & 1;

            freq -= M_PI * qpd;

            /* get deviation from bin frequency from the +/- Pi interval */
            freq = samples * freq / (2.0f * M_PI);

            /* compute the k-th partials' true frequency */
            freq = static_cast<AmReal32>(i) * freqPerBin + freq * freqPerBin;

            /* store magnitude and true frequency in analysis arrays */
            buffer[i * 2 + 1] = freq;
        }
    }

    void FFTFilterInstance::MagFreq2MagPhase(AmReal32Buffer buffer, AmUInt32 samples, AmUInt32 sampleRate, AmUInt16 channel)
    {
        const AmReal32 stepsize = static_cast<AmReal32>(samples) / sampleRate;
        const AmReal32 expect = (stepsize / samples) * 2.0f * static_cast<AmReal32>(M_PI);
        const AmReal32 freqPerBin = static_cast<AmReal32>(sampleRate) / samples;

        for (AmUInt32 i = 0; i < samples; i++)
        {
            /* get magnitude and true frequency from synthesis arrays */
            AmReal32 mag = buffer[i * 2];
            AmReal32 freq = buffer[i * 2 + 1];

            /* subtract bin mid frequency */
            freq -= static_cast<AmReal32>(i) * freqPerBin;

            /* get bin deviation from freq deviation */
            freq /= freqPerBin;

            /* take osamp into account */
            freq = (freq / samples) * static_cast<AmReal32>(M_PI) * 2.0f;

            /* add the overlap phase advance back in */
            freq += static_cast<AmReal32>(i) * expect;

            /* accumulate delta phase to get bin phase */

            _sumPhase[i + channel * STFT_WINDOW_SIZE] += freq;
            buffer[i * 2 + 1] = _sumPhase[i + channel * STFT_WINDOW_SIZE];
        }
    }

    void FFTFilterInstance::MagPhase2Comp(AmReal32Buffer buffer, AmUInt32 samples)
    {
        for (AmUInt32 i = 0; i < samples; i++)
        {
            const AmReal32 mag = buffer[i * 2];
            const AmReal32 pha = buffer[i * 2 + 1];

            buffer[i * 2] = std::cos(pha) * mag;
            buffer[i * 2 + 1] = std::sin(pha) * mag;
        }
    }

    void FFTFilterInstance::ProcessFFTChannel(
        AmReal32Buffer buffer, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate)
    {
        Comp2MagPhase(buffer, frames);
        MagPhase2MagFreq(buffer, frames, sampleRate, channel);

        AmReal32 t[STFT_WINDOW_TWICE];
        memcpy(t, buffer, sizeof(AmReal32) * frames);
        memset(buffer, 0, sizeof(AmReal32) * frames * 2);

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
