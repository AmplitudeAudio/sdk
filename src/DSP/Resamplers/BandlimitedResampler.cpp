// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#include <algorithm>
#include <cmath>
#include <limits>

#include <DSP/Resamplers/BandlimitedResampler.h>

namespace SparkyStudios::Audio::Amplitude
{
    namespace
    {
        // The accumulator holds ratio * 2^32 per output: these bounds keep it far from overflow.
        constexpr AmReal64 kMinRatio = 1.0 / 65536.0;
        constexpr AmReal64 kMaxRatio = 65536.0;

        // Catmull-Rom, the 4-point Hermite kernel.
        AmReal32 CubicWeight(AmReal64 d)
        {
            if (d < 1.0)
                return static_cast<AmReal32>((1.5 * d - 2.5) * d * d + 1.0);

            if (d < 2.0)
                return static_cast<AmReal32>(((-0.5 * d + 2.5) * d - 4.0) * d + 2.0);

            return 0.0f;
        }
    } // namespace

    std::shared_ptr<const BandlimitedKernel> GetPresetKernel(eResamplerPreset preset)
    {
        switch (preset)
        {
        case eResamplerPreset::Sinc:
            {
                static const auto kernel = std::make_shared<const BandlimitedKernel>(kSincKernel);
                return kernel;
            }
        case eResamplerPreset::SincBest:
            {
                static const auto kernel = std::make_shared<const BandlimitedKernel>(kSincBestKernel);
                return kernel;
            }
        default:
            return nullptr;
        }
    }

    BandlimitedResamplerInstance::BandlimitedResamplerInstance(eResamplerPreset preset, std::shared_ptr<const BandlimitedKernel> kernel)
        : _preset(preset)
        , _kernel(std::move(kernel))
    {
        AMPLITUDE_ASSERT((_kernel != nullptr) == (preset == eResamplerPreset::Sinc || preset == eResamplerPreset::SincBest));
        SetRatio(1.0);
    }

    AmUInt64 BandlimitedResamplerInstance::MaxReachFrames() const
    {
        switch (_preset)
        {
        case eResamplerPreset::Linear:
            return 1;
        case eResamplerPreset::Cubic:
            return 2;
        default:
            return static_cast<AmUInt64>(_kernel->GetZeroCrossings()) * static_cast<AmUInt64>(kMaxKernelStretch) + 1;
        }
    }

    void BandlimitedResamplerInstance::Initialize(AmUInt16 channelCount, AmUInt32 sampleRateIn, AmUInt32 sampleRateOut)
    {
        AMPLITUDE_ASSERT(channelCount > 0 && channelCount <= kAmMaxSupportedChannelCount);

        _channels = static_cast<AmUInt16>(std::clamp<AmUInt32>(channelCount, 1, kAmMaxSupportedChannelCount));
        _rateIn = AM_MAX(sampleRateIn, 1U);
        _rateOut = AM_MAX(sampleRateOut, 1U);

        const AmUInt64 reach = MaxReachFrames();
        _historyFrames = reach + 1;
        _history = AudioBuffer(_historyFrames, _channels);
        _weights.assign(2 * reach + 2, 0.0f);
        _gather.assign(2 * reach + 2, 0.0f);

        SetRatio(static_cast<AmReal64>(_rateIn) / static_cast<AmReal64>(_rateOut));
        Reset();
    }

    void BandlimitedResamplerInstance::SetRatio(AmReal64 inputPerOutput)
    {
        if (!std::isfinite(inputPerOutput) || inputPerOutput <= 0.0)
            inputPerOutput = 1.0;

        _ratio = std::clamp(inputPerOutput, kMinRatio, kMaxRatio);
        _step = AM_MAX(static_cast<AmUInt64>(std::llround(_ratio * static_cast<AmReal64>(kOne))), 1ULL);

        switch (_preset)
        {
        case eResamplerPreset::Linear:
            _stretch = 1.0;
            _reach = kOne;
            break;
        case eResamplerPreset::Cubic:
            _stretch = 1.0;
            _reach = 2 * kOne;
            break;
        default:
            _stretch = std::clamp(_ratio, 1.0, kMaxKernelStretch);
            _reach = static_cast<AmUInt64>(
                std::llround(static_cast<AmReal64>(_kernel->GetZeroCrossings()) * _stretch * static_cast<AmReal64>(kOne)));
            _tableScale = static_cast<AmReal64>(_kernel->GetPhases()) / _stretch;
            break;
        }

        // A kernel stretched by s cuts at 1/s of Nyquist and has a DC gain of s.
        _gain = static_cast<AmReal32>(1.0 / _stretch);
    }

    AmUInt64 BandlimitedResamplerInstance::CurrentReach() const
    {
        return _step == kOne && _frac == 0 ? 0 : _reach;
    }

    AmUInt64 BandlimitedResamplerInstance::GetInputFramesNeeded(AmUInt64 outputFrameCount) const
    {
        if (outputFrameCount == 0)
            return 0;

        // (outputFrameCount - 1) * _step is 32.32 frames: cap the request where that product would wrap.
        const AmUInt64 maxCount = (std::numeric_limits<AmUInt64>::max() / _step) + 1;
        outputFrameCount = AM_MIN(outputFrameCount, maxCount);

        const AmUInt64 reach = CurrentReach();
        const AmUInt64 last = _frac + (outputFrameCount - 1) * _step;
        const AmUInt64 top = reach == 0 ? (last >> 32) : ((last + reach - 1) >> 32);
        const AmUInt64 consumedAfter = (last + _step) >> 32;

        return AM_MAX(top + 1, consumedAfter);
    }

    bool BandlimitedResamplerInstance::Process(const AudioBuffer& input, AmUInt64& inputFrames, AudioBuffer& output, AmUInt64& outputFrames)
    {
        AMPLITUDE_ASSERT(input.GetChannelCount() == _channels && output.GetChannelCount() == _channels);

        const AmUInt64 available = AM_MIN(inputFrames, static_cast<AmUInt64>(input.GetFrameCount()));
        const AmUInt64 wanted = AM_MIN(outputFrames, static_cast<AmUInt64>(output.GetFrameCount()));
        const AmUInt64 reach = CurrentReach();

        AmUInt64 time = _frac;
        AmUInt64 produced = 0;
        while (produced < wanted)
        {
            const AmUInt64 top = reach == 0 ? (time >> 32) : ((time + reach - 1) >> 32);
            if (top >= available || ((time + _step) >> 32) > available)
                break;

            RenderFrame(input, output, time, reach, produced);
            time += _step;
            ++produced;
        }

        const AmUInt64 consumed = time >> 32;
        PushHistory(input, consumed);
        _frac = time & (kOne - 1);

        inputFrames = consumed;
        outputFrames = produced;
        return true;
    }

    AmReal32 BandlimitedResamplerInstance::Weight(AmReal64 distance) const
    {
        switch (_preset)
        {
        case eResamplerPreset::Linear:
            return distance < 1.0 ? static_cast<AmReal32>(1.0 - distance) : 0.0f;
        case eResamplerPreset::Cubic:
            return CubicWeight(distance);
        default:
            {
                const AmReal64 p = distance * _tableScale;
                const auto i = static_cast<AmUInt64>(p);
                if (i >= _kernel->GetTableSize())
                    return 0.0f;

                const AmReal32* table = _kernel->GetTable();
                const auto f = static_cast<AmReal32>(p - static_cast<AmReal64>(i));
                return (table[i] + f * (table[i + 1] - table[i])) * _gain;
            }
        }
    }

    void BandlimitedResamplerInstance::RenderFrame(
        const AudioBuffer& input, AudioBuffer& output, AmUInt64 time, AmUInt64 reach, AmUInt64 index)
    {
        const auto signedTime = static_cast<AmInt64>(time);
        const AmInt64 first = reach == 0 ? (signedTime >> 32) : ((signedTime - static_cast<AmInt64>(reach)) >> 32) + 1;
        const AmInt64 last = reach == 0 ? (signedTime >> 32) : static_cast<AmInt64>((time + reach - 1) >> 32);
        const auto count = static_cast<AmUInt64>(last - first + 1);
        AMPLITUDE_ASSERT(count <= _weights.size() && -first <= static_cast<AmInt64>(_historyFrames));

        // The weights depend only on each frame's distance to the output time: computed once, in fixed point so the
        // result does not depend on how the input is split across calls, and applied to every channel.
        for (AmUInt64 k = 0; k < count; ++k)
        {
            const AmInt64 distance = (first + static_cast<AmInt64>(k)) * static_cast<AmInt64>(kOne) - signedTime;
            _weights[k] = Weight(std::abs(static_cast<AmReal64>(distance)) / static_cast<AmReal64>(kOne));
        }

        for (AmUInt16 c = 0; c < _channels; ++c)
        {
            const AmReal32* taps = nullptr;
            if (first >= 0)
            {
                taps = input[c].begin() + first;
            }
            else
            {
                // Only the first outputs of a call reach back into the history: gather their taps contiguously.
                for (AmUInt64 k = 0; k < count; ++k)
                {
                    const AmInt64 frame = first + static_cast<AmInt64>(k);
                    _gather[k] = frame < 0 ? _history[c][static_cast<AmSize>(static_cast<AmInt64>(_historyFrames) + frame)]
                                           : input[c][static_cast<AmSize>(frame)];
                }

                taps = _gather.data();
            }

            AmReal32 sum = 0.0f;
            for (AmUInt64 k = 0; k < count; ++k)
                sum += _weights[k] * taps[k];

            output[c][index] = sum;
        }
    }

    void BandlimitedResamplerInstance::PushHistory(const AudioBuffer& input, AmUInt64 consumed)
    {
        if (consumed == 0)
            return;

        for (AmUInt16 c = 0; c < _channels; ++c)
        {
            AmReal32* history = _history[c].begin();
            const AmReal32* in = input[c].begin();

            if (consumed >= _historyFrames)
            {
                std::copy_n(in + (consumed - _historyFrames), _historyFrames, history);
                continue;
            }

            std::copy(history + consumed, history + _historyFrames, history);
            std::copy_n(in, consumed, history + (_historyFrames - consumed));
        }
    }

    void BandlimitedResamplerInstance::Reset()
    {
        _frac = 0;
        _history.Clear();
    }

    void BandlimitedResamplerInstance::Clear()
    {
        Reset();
        _history = AudioBuffer();
        _historyFrames = 0;
        _weights.clear();
        _gather.clear();
        _channels = 0;
        _rateIn = 0;
        _rateOut = 0;
    }

    BandlimitedResampler::BandlimitedResampler(AmString name, eResamplerPreset preset)
        : Resampler(std::move(name))
        , _preset(preset)
    {}

    std::shared_ptr<ResamplerInstance> BandlimitedResampler::CreateInstance()
    {
        return ampoolshared(eMemoryPoolKind_Filtering, BandlimitedResamplerInstance, _preset, GetPresetKernel(_preset));
    }
} // namespace SparkyStudios::Audio::Amplitude
