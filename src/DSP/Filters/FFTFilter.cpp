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
#include <SparkyStudios/Audio/Amplitude/DSP/FFT.h>
#include <SparkyStudios/Audio/Amplitude/Math/Utils.h>

#include <DSP/Filters/FFTFilter.h>
#include <Utils/Utils.h>

#define STFT_WINDOW_SIZE 256 // must be power of two
#define STFT_WINDOW_HALF 128

namespace
{
    using namespace SparkyStudios::Audio::Amplitude;

    // Writes STFT_WINDOW_HALF samples into a ring buffer of STFT_WINDOW_SIZE
    // capacity at the given position, handling the wrap-around.
    void RingWrite(AmReal32Buffer fifo, AmUInt32 position, const AmReal32* samples)
    {
        const AmUInt32 first = std::min<AmUInt32>(STFT_WINDOW_HALF, STFT_WINDOW_SIZE - position);
        std::memcpy(fifo + position, samples, first * sizeof(AmReal32));
        if (first < STFT_WINDOW_HALF)
            std::memcpy(fifo, samples + first, (STFT_WINDOW_HALF - first) * sizeof(AmReal32));
    }
} // namespace

namespace SparkyStudios::Audio::Amplitude
{
    FFTFilter::FFTFilter(const std::string& name)
        : Filter(name)
    {}

    std::shared_ptr<FilterInstance> FFTFilter::CreateInstance()
    {
        return ampoolshared(eMemoryPoolKind_Filtering, FFTFilterInstance, this);
    }

    FFTFilterInstance::FFTFilterInstance(FFTFilter* parent)
        : FilterInstance(parent)
    {
        InitializeFFT();
        Initialize(1);
    }

    FFTFilterInstance::~FFTFilterInstance()
    {
        ampoolfree(eMemoryPoolKind_Filtering, _temp);
        _temp = nullptr;

        if (_sumPhase != nullptr)
        {
            ampoolfree(eMemoryPoolKind_Filtering, _sumPhase);
            _sumPhase = nullptr;
        }

        if (_lastPhase != nullptr)
        {
            ampoolfree(eMemoryPoolKind_Filtering, _lastPhase);
            _lastPhase = nullptr;
        }

        if (_window != nullptr)
        {
            ampoolfree(eMemoryPoolKind_Filtering, _window);
            _window = nullptr;
        }

        if (_inHistory != nullptr)
        {
            ampoolfree(eMemoryPoolKind_Filtering, _inHistory);
            _inHistory = nullptr;
        }

        if (_ola != nullptr)
        {
            ampoolfree(eMemoryPoolKind_Filtering, _ola);
            _ola = nullptr;
        }

        if (_wetFifo != nullptr)
        {
            ampoolfree(eMemoryPoolKind_Filtering, _wetFifo);
            _wetFifo = nullptr;
        }

        if (_dryFifo != nullptr)
        {
            ampoolfree(eMemoryPoolKind_Filtering, _dryFifo);
            _dryFifo = nullptr;
        }

        if (_carry != nullptr)
        {
            ampoolfree(eMemoryPoolKind_Filtering, _carry);
            _carry = nullptr;
        }

        if (_scratch != nullptr)
        {
            ampoolfree(eMemoryPoolKind_Filtering, _scratch);
            _scratch = nullptr;
        }

        _scratchCapacity = 0;
    }

    void FFTFilterInstance::InitializeFFT()
    {
        constexpr AmUInt32 n = STFT_WINDOW_SIZE;
        constexpr AmUInt32 h = STFT_WINDOW_HALF;

        _temp = static_cast<AmReal32Buffer>(ampoolmalloc(eMemoryPoolKind_Filtering, n * sizeof(AmReal32)));

        // Cache the FFT plan and the complex scratch buffer
        _fft.Initialize(n);
        _sc.Resize(FFT::GetOutputSize(n), true);

        // Phase accumulators are indexed by (bin + channel * STFT_WINDOW_SIZE)
        const AmSize phaseSize = n * kAmMaxSupportedChannelCount * sizeof(AmReal32);
        _sumPhase = static_cast<AmReal32Buffer>(ampoolmalloc(eMemoryPoolKind_Filtering, phaseSize));
        _lastPhase = static_cast<AmReal32Buffer>(ampoolmalloc(eMemoryPoolKind_Filtering, phaseSize));

        std::memset(_sumPhase, 0, phaseSize);
        std::memset(_lastPhase, 0, phaseSize);

        // Streaming overlap-add state, per channel.
        _window = static_cast<AmReal32Buffer>(ampoolmalloc(eMemoryPoolKind_Filtering, n * sizeof(AmReal32)));
        _inHistory =
            static_cast<AmReal32Buffer>(ampoolmalloc(eMemoryPoolKind_Filtering, h * kAmMaxSupportedChannelCount * sizeof(AmReal32)));
        _ola = static_cast<AmReal32Buffer>(ampoolmalloc(eMemoryPoolKind_Filtering, n * kAmMaxSupportedChannelCount * sizeof(AmReal32)));
        _wetFifo = static_cast<AmReal32Buffer>(ampoolmalloc(eMemoryPoolKind_Filtering, n * kAmMaxSupportedChannelCount * sizeof(AmReal32)));
        _dryFifo = static_cast<AmReal32Buffer>(ampoolmalloc(eMemoryPoolKind_Filtering, n * kAmMaxSupportedChannelCount * sizeof(AmReal32)));
        _carry = static_cast<AmReal32Buffer>(ampoolmalloc(eMemoryPoolKind_Filtering, h * kAmMaxSupportedChannelCount * sizeof(AmReal32)));

        // Triangular COLA synthesis window: w(i) + w(i + H) == 1 for 50% overlap.
        for (AmUInt32 i = 0; i < n; ++i)
            _window[i] = 1.0f - std::abs(2.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(n) - 1.0f);

        std::memset(_inHistory, 0, h * kAmMaxSupportedChannelCount * sizeof(AmReal32));
        std::memset(_ola, 0, n * kAmMaxSupportedChannelCount * sizeof(AmReal32));
        std::memset(_wetFifo, 0, n * kAmMaxSupportedChannelCount * sizeof(AmReal32));
        std::memset(_dryFifo, 0, n * kAmMaxSupportedChannelCount * sizeof(AmReal32));
        std::memset(_carry, 0, h * kAmMaxSupportedChannelCount * sizeof(AmReal32));

        // Prime both FIFOs with one hop of silence. Together with the first hop's
        // zero half-window emission this yields the fixed 256-sample latency.
        for (AmUInt32 c = 0; c < kAmMaxSupportedChannelCount; ++c)
            _fifoCount[c] = h;
    }

    void FFTFilterInstance::Process(const AudioBuffer& in, AudioBuffer& out, AmUInt64 frames, AmUInt32 sampleRate)
    {
        AMPLITUDE_ASSERT(_sumPhase != nullptr && _lastPhase != nullptr);
        AMPLITUDE_ASSERT(in.GetChannelCount() <= kAmMaxSupportedChannelCount);

        FilterInstance::Process(in, out, frames, sampleRate);
    }

    void FFTFilterInstance::ProcessChannel(const AudioBuffer& in, AudioBuffer& out, AmUInt16 channel, AmUInt64 frames, AmUInt32 sampleRate)
    {
        AMPLITUDE_ASSERT(channel < kAmMaxSupportedChannelCount);

        const auto& inChannel = in[channel];
        auto& outChannel = out[channel];

        const AmReal32* input = inChannel.begin();

        if (input == outChannel.begin() && frames > 0)
        {
            if (_scratchCapacity < frames)
            {
                if (_scratch != nullptr)
                {
                    ampoolfree(eMemoryPoolKind_Filtering, _scratch);
                    _scratch = nullptr;
                }

                _scratch = static_cast<AmReal32Buffer>(ampoolmalloc(eMemoryPoolKind_Filtering, frames * sizeof(AmReal32)));
                _scratchCapacity = frames;
            }

            std::memcpy(_scratch, input, frames * sizeof(AmReal32));
            input = _scratch;
        }

        AmReal32* inHistory = _inHistory + channel * STFT_WINDOW_HALF;
        AmReal32* ola = _ola + channel * STFT_WINDOW_SIZE;
        AmReal32* wetFifo = _wetFifo + channel * STFT_WINDOW_SIZE;
        AmReal32* dryFifo = _dryFifo + channel * STFT_WINDOW_SIZE;
        AmReal32* carry = _carry + channel * STFT_WINDOW_HALF;

        AmUInt32& fifoHead = _fifoHead[channel];
        AmUInt32& fifoCount = _fifoCount[channel];
        AmUInt32& carryCount = _carryCount[channel];

        AmUInt64 inPos = 0;

        for (AmUInt64 outPos = 0; outPos < frames; ++outPos)
        {
            if (fifoCount == 0)
            {
                // Complete the pending hop with new input. The FIFO invariant
                // (fifoCount + carryCount == STFT_WINDOW_HALF at call boundaries)
                // guarantees enough input remains for a full hop.
                const AmUInt32 needed = STFT_WINDOW_HALF - carryCount;

                for (AmUInt32 i = 0; i < needed; ++i, ++inPos)
                    carry[carryCount + i] = input[inPos];

                // Analysis frame: previous half-window followed by the new samples.
                std::memcpy(_temp, inHistory, STFT_WINDOW_HALF * sizeof(AmReal32));
                std::memcpy(_temp + STFT_WINDOW_HALF, carry, STFT_WINDOW_HALF * sizeof(AmReal32));

                _fft.Forward(_temp, _sc);
                ProcessFFTChannel(_sc, channel, STFT_WINDOW_HALF, in.GetChannelCount(), sampleRate);
                _fft.Backward(_temp, _sc);

                // Overlap-add with the COLA synthesis window.
                for (AmUInt32 i = 0; i < STFT_WINDOW_SIZE; ++i)
                    ola[i] += _temp[i] * _window[i];

                // Emit the finalized half-window, and the time-aligned dry samples
                // (the input half-window processed one hop ago).
                RingWrite(wetFifo, fifoHead, ola);
                RingWrite(dryFifo, fifoHead, inHistory);
                fifoCount += STFT_WINDOW_HALF;

                std::memmove(ola, ola + STFT_WINDOW_HALF, STFT_WINDOW_HALF * sizeof(AmReal32));
                std::memset(ola + STFT_WINDOW_HALF, 0, STFT_WINDOW_HALF * sizeof(AmReal32));

                std::memcpy(inHistory, carry, STFT_WINDOW_HALF * sizeof(AmReal32));
                carryCount = 0;
            }

            const AmReal32 dry = dryFifo[fifoHead];
            const AmReal32 wet = wetFifo[fifoHead];
            fifoHead = (fifoHead + 1) % STFT_WINDOW_SIZE;
            --fifoCount;

            outChannel[outPos] = static_cast<AmAudioSample>(dry + (wet - dry) * m_parameters[0]);
        }

        // Stash the remaining input for the next call. Any carry left over from
        // a previous sub-hop call is still pending consumption, so append.
        if (inPos < frames)
        {
            const AmUInt32 remaining = static_cast<AmUInt32>(frames - inPos);
            for (AmUInt32 i = 0; i < remaining; ++i, ++inPos)
                carry[carryCount + i] = input[inPos];
            carryCount += remaining;
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
        // Expected per-hop phase advance for bin s: 2*pi*s*H/N.
        const AmReal32 expect = 2.0f * AM_PI32 * (static_cast<AmReal32>(STFT_WINDOW_HALF) / static_cast<AmReal32>(STFT_WINDOW_SIZE));
        const AmReal32 freqPerBin = static_cast<AmReal32>(sampleRate) / static_cast<AmReal32>(STFT_WINDOW_SIZE);

        for (AmUInt32 s = 0; s < samples; s++)
        {
            // get true frequency from analysis arrays
            const AmReal32 pha = fft.im()[s];

            // compute phase difference
            AmReal32 freq = pha - _lastPhase[s + channel * STFT_WINDOW_SIZE];
            _lastPhase[s + channel * STFT_WINDOW_SIZE] = pha;

            // subtract expected phase difference
            freq -= static_cast<AmReal32>(s) * expect;

            // map delta phase into +/- Pi interval (truncation toward zero, as
            // in the reference phase vocoder: floor would mis-wrap values in
            // (-Pi, 0) by an extra 2*Pi, i.e. a +2-bin true-frequency error)
            auto qpd = static_cast<AmInt32>(freq / AM_PI32);

            if (qpd >= 0)
                qpd += qpd & 1;
            else
                qpd -= qpd & 1;

            freq -= AM_PI32 * static_cast<AmReal32>(qpd);

            // get deviation from bin frequency, in bins: dphi per bin is 2*pi*H/N
            freq = freq * (static_cast<AmReal32>(STFT_WINDOW_SIZE) / (2.0f * AM_PI32 * static_cast<AmReal32>(STFT_WINDOW_HALF)));

            // compute the k-th partials' true frequency
            freq = s * freqPerBin + freq * freqPerBin;

            // store magnitude and true frequency in analysis arrays
            fft.im()[s] = freq;
        }
    }

    void FFTFilterInstance::MagFreq2MagPhase(SplitComplex& fft, AmUInt32 samples, AmUInt32 sampleRate, AmUInt16 channel)
    {
        // Expected per-hop phase advance for bin s: 2*pi*s*H/N.
        const AmReal32 expect = 2.0f * AM_PI32 * (static_cast<AmReal32>(STFT_WINDOW_HALF) / static_cast<AmReal32>(STFT_WINDOW_SIZE));
        const AmReal32 freqPerBin = static_cast<AmReal32>(sampleRate) / static_cast<AmReal32>(STFT_WINDOW_SIZE);

        for (AmUInt32 s = 0; s < samples; s++)
        {
            // get true frequency from synthesis arrays
            const AmReal32 freq = fft.im()[s];

            AmReal32 pha = freq;

            // subtract bin mid-frequency
            pha -= static_cast<AmReal32>(s) * freqPerBin;

            // get bin deviation from freq deviation
            pha /= freqPerBin;

            // convert the bin deviation into a per-hop phase advance
            pha *= 2.0f * AM_PI32 * (static_cast<AmReal32>(STFT_WINDOW_HALF) / static_cast<AmReal32>(STFT_WINDOW_SIZE));

            // add the overlap phase advance back in
            pha += static_cast<AmReal32>(s) * expect;

            _sumPhase[s + channel * STFT_WINDOW_SIZE] += pha;
            fft.im()[s] = _sumPhase[s + channel * STFT_WINDOW_SIZE];
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

        // Identity transform

        MagPhase2Comp(fft, frames);
    }
} // namespace SparkyStudios::Audio::Amplitude
