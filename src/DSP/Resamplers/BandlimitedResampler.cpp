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
        SetRatioRamp(inputPerOutput, inputPerOutput, 0);
    }

    void BandlimitedResamplerInstance::SetRatioRamp(AmReal64 inputPerOutputStart, AmReal64 inputPerOutputEnd, AmUInt64 outputFrames)
    {
        const auto toStep = [](AmReal64 ratio)
        {
            if (!std::isfinite(ratio) || ratio <= 0.0)
                ratio = 1.0;

            ratio = std::clamp(ratio, kMinRatio, kMaxRatio);
            return AM_MAX(static_cast<AmUInt64>(std::llround(ratio * static_cast<AmReal64>(kOne))), 1ULL);
        };

        _rampStepStart = toStep(inputPerOutputStart);
        _rampStepEnd = toStep(inputPerOutputEnd);
        _rampFrames = outputFrames;
        _rampPos = 0;

        // The read-ahead and the read-ahead estimate are sized for the widest ratio of the ramp, so neither the
        // availability check nor GetInputFramesNeeded() can be caught short by a ramp that speeds the stream up.
        _maxStep = AM_MAX(_rampStepStart, _rampStepEnd);
        _ratio = 0.5 * (static_cast<AmReal64>(_rampStepStart) + static_cast<AmReal64>(_rampStepEnd)) / static_cast<AmReal64>(kOne);

        // ApplyRatio() writes the shared kernel state, so the two endpoints are sized one after the other into named
        // locals rather than as the arguments of one call, whose evaluation order is unspecified.
        const AmUInt64 reachStart = ApplyRatio(static_cast<AmReal64>(_rampStepStart) / static_cast<AmReal64>(kOne));
        const AmUInt64 reachEnd = ApplyRatio(static_cast<AmReal64>(_rampStepEnd) / static_cast<AmReal64>(kOne));
        _reach = AM_MAX(reachStart, reachEnd);
    }

    AmUInt64 BandlimitedResamplerInstance::ApplyRatio(AmReal64 ratio)
    {
        switch (_preset)
        {
        case eResamplerPreset::Linear:
            _stretch = 1.0;
            break;
        case eResamplerPreset::Cubic:
            _stretch = 1.0;
            break;
        default:
            _stretch = std::clamp(ratio, 1.0, kMaxKernelStretch);

            // Only the table-driven presets read the table scale; Linear and Cubic evaluate their kernel directly.
            _tableScale = static_cast<AmReal64>(_kernel->GetPhases()) / _stretch;
            break;
        }

        // A kernel stretched by s cuts at 1/s of Nyquist and has a DC gain of s.
        _gain = static_cast<AmReal32>(1.0 / _stretch);

        switch (_preset)
        {
        case eResamplerPreset::Linear:
            return kOne;
        case eResamplerPreset::Cubic:
            return 2 * kOne;
        default:
            return static_cast<AmUInt64>(
                std::llround(static_cast<AmReal64>(_kernel->GetZeroCrossings()) * _stretch * static_cast<AmReal64>(kOne)));
        }
    }

    AmUInt64 BandlimitedResamplerInstance::StepAt(AmUInt64 position) const
    {
        if (_rampFrames <= 1 || _rampStepStart == _rampStepEnd)
            return _rampStepEnd;

        const auto u = static_cast<AmReal64>(AM_MIN(position, _rampFrames - 1)) / static_cast<AmReal64>(_rampFrames - 1);

        // Smoothstep, so the step matches in value and in slope at both ends of the ramp: consecutive blocks join
        // without a kink, which is what a click at the boundary is made of.
        const auto s = static_cast<AmReal64>(static_cast<AmReal32>(u * u * (3.0f - 2.0f * u)));

        const auto step = static_cast<AmReal64>(_rampStepStart) * (1.0 - s) + static_cast<AmReal64>(_rampStepEnd) * s;
        return AM_MAX(static_cast<AmUInt64>(std::llround(step)), 1ULL);
    }

    AmUInt64 BandlimitedResamplerInstance::CurrentReach() const
    {
        // A ramp never collapses to the identity shortcut: its frames move off the centre at their own step, so even
        // the ones that land on a whole frame need their kernel.
        if (_rampStepStart != _rampStepEnd)
            return _reach;

        return _maxStep == kOne && _frac == 0 ? 0 : _reach;
    }

    AmUInt64 BandlimitedResamplerInstance::GetInputFramesNeeded(AmUInt64 outputFrameCount) const
    {
        if (outputFrameCount == 0)
            return 0;

        // (outputFrameCount - 1) * _maxStep is 32.32 frames: cap the request where that product would wrap.
        const AmUInt64 maxCount = (std::numeric_limits<AmUInt64>::max() / _maxStep) + 1;
        outputFrameCount = AM_MIN(outputFrameCount, maxCount);

        const AmUInt64 reach = CurrentReach();
        const AmUInt64 last = _frac + (outputFrameCount - 1) * _maxStep;
        const AmUInt64 top = reach == 0 ? (last >> 32) : ((last + reach - 1) >> 32);
        const AmUInt64 consumedAfter = (last + _maxStep) >> 32;

        return AM_MAX(top + 1, consumedAfter);
    }

    bool BandlimitedResamplerInstance::Process(const AudioBuffer& input, AmUInt64& inputFrames, AudioBuffer& output, AmUInt64& outputFrames)
    {
        AMPLITUDE_ASSERT(input.GetChannelCount() == _channels && output.GetChannelCount() == _channels);

        const AmUInt64 available = AM_MIN(inputFrames, static_cast<AmUInt64>(input.GetFrameCount()));
        const AmUInt64 wanted = AM_MIN(outputFrames, static_cast<AmUInt64>(output.GetFrameCount()));
        const AmUInt64 reach = CurrentReach();

        // Without a live ramp the ratio cannot change within the call, so the kernel stays sized by the ApplyRatio()
        // that SetRatio() or SetRatioRamp() already ran at the ratio StepAt() reports for every frame.
        const bool ramping = _rampStepStart != _rampStepEnd;

        AmUInt64 time = _frac;
        AmUInt64 produced = 0;
        while (produced < wanted)
        {
            // The kernel follows the ratio of this very output frame, so the cutoff and the gain track the pitch
            // through the ramp instead of holding the value the whole ramp was sized for.
            const AmUInt64 step = ramping ? StepAt(_rampPos + produced) : _rampStepEnd;

            const AmUInt64 top = reach == 0 ? (time >> 32) : ((time + reach - 1) >> 32);
            if (top >= available || ((time + step) >> 32) > available)
                break;

            if (ramping)
                AM_UNUSED(ApplyRatio(static_cast<AmReal64>(step) / static_cast<AmReal64>(kOne)));

            RenderFrame(input, output, time, reach, produced);
            time += step;
            ++produced;
        }

        const AmUInt64 consumed = time >> 32;
        PushHistory(input, consumed);
        _frac = time & (kOne - 1);
        _rampPos += produced;

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
        _rampPos = 0;
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
