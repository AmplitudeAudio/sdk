// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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
#include <cstdint>
#include <cstring>

#include <DSP/Gain.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    constexpr AmReal32 kGainThreshold = 0.001f;

    void Gain::ApplyReplaceConstantGain(
        AmReal32 gain, const AudioBufferChannel& in, AmSize inOffset, AudioBufferChannel& out, AmSize outOffset, AmSize frames)
    {
        AMPLITUDE_ASSERT(in.size() >= inOffset + frames);
        AMPLITUDE_ASSERT(out.size() >= outOffset + frames);

        const AmReal32* src = in.begin() + inOffset;
        AmReal32* dst = out.begin() + outOffset;

        if (IsZero(gain))
            std::fill_n(dst, frames, 0.0f);
        else if (IsOne(gain))
        {
            if (src != dst)
                std::memmove(dst, src, frames * sizeof(AmReal32));
        }
        else
            ScalarMultiply(src, dst, gain, frames);
    }

    void Gain::ApplyAccumulateConstantGain(
        AmReal32 gain, const AudioBufferChannel& in, AmSize inOffset, AudioBufferChannel& out, AmSize outOffset, AmSize frames)
    {
        AMPLITUDE_ASSERT(in.size() >= inOffset + frames);
        AMPLITUDE_ASSERT(out.size() >= outOffset + frames);

        if (IsZero(gain))
            return;

        ScalarMultiplyAccumulate(in.begin() + inOffset, out.begin() + outOffset, gain, frames);
    }

    void Gain::ApplyReplaceLinearGain(
        AmReal32 startGain,
        AmReal32 endGain,
        const AudioBufferChannel& in,
        AmSize inOffset,
        AudioBufferChannel& out,
        AmSize outOffset,
        AmSize frames)
    {
        AMPLITUDE_ASSERT(in.size() >= inOffset + frames);
        AMPLITUDE_ASSERT(out.size() >= outOffset + frames);

        const AmReal32 invFrames = 1.0f / static_cast<AmReal32>(frames);
        const AmReal32 gainDelta = endGain - startGain;

        AmSize j = 0;

#if defined(AM_SIMD_INTRINSICS)
        constexpr AmSize blockSize = GetSimdBlockSize();
        const AmSize simdEnd = (frames / blockSize) * blockSize;

        if (simdEnd > 0)
        {
            alignas(AM_SIMD_ALIGNMENT) AmReal32 gainValues[blockSize];
            for (AmSize i = 0; i < blockSize; ++i)
                gainValues[i] = startGain + gainDelta * (invFrames * static_cast<AmReal32>(i));

            auto gainVec = xsimd::load_aligned<simd_arch>(gainValues);
            const auto stepVec = simd_batch(gainDelta * invFrames * static_cast<AmReal32>(blockSize));

            for (; j < simdEnd; j += blockSize)
            {
                const auto inVec = xsimd::load_aligned<simd_arch>(&in[j + inOffset]);
                xsimd::store_aligned<simd_arch>(&out[j + outOffset], inVec * gainVec);
                gainVec += stepVec;
            }
        }
#endif // AM_SIMD_INTRINSICS

        for (; j < frames; ++j)
            out[j + outOffset] = in[j + inOffset] * (startGain + gainDelta * (invFrames * static_cast<AmReal32>(j)));
    }

    void Gain::ApplyAccumulateLinearGain(
        AmReal32 startGain,
        AmReal32 endGain,
        const AudioBufferChannel& in,
        AmSize inOffset,
        AudioBufferChannel& out,
        AmSize outOffset,
        AmSize frames)
    {
        AMPLITUDE_ASSERT(in.size() >= inOffset + frames);
        AMPLITUDE_ASSERT(out.size() >= outOffset + frames);

        const AmReal32 invFrames = 1.0f / static_cast<AmReal32>(frames);
        const AmReal32 gainDelta = endGain - startGain;

        AmSize j = 0;

#if defined(AM_SIMD_INTRINSICS)
        constexpr AmSize blockSize = GetSimdBlockSize();
        const AmSize simdEnd = (frames / blockSize) * blockSize;

        if (simdEnd > 0)
        {
            alignas(AM_SIMD_ALIGNMENT) AmReal32 gainValues[blockSize];
            for (AmSize i = 0; i < blockSize; ++i)
                gainValues[i] = startGain + gainDelta * (invFrames * static_cast<AmReal32>(i));

            auto gainVec = xsimd::load_aligned<simd_arch>(gainValues);
            const auto stepVec = simd_batch(gainDelta * invFrames * static_cast<AmReal32>(blockSize));

            for (; j < simdEnd; j += blockSize)
            {
                const auto inVec = xsimd::load_aligned<simd_arch>(&in[j + inOffset]);
                const auto outVec = xsimd::load_aligned<simd_arch>(&out[j + outOffset]);
                xsimd::store_aligned<simd_arch>(&out[j + outOffset], xsimd::fma(inVec, gainVec, outVec));
                gainVec += stepVec;
            }
        }
#endif // AM_SIMD_INTRINSICS

        for (; j < frames; ++j)
            out[j + outOffset] += in[j + inOffset] * (startGain + gainDelta * (invFrames * static_cast<AmReal32>(j)));
    }

    void Gain::ApplyReplaceGain(
        Curve gainCurve, const AudioBufferChannel& in, AmSize inOffset, AudioBufferChannel& out, AmSize outOffset, AmSize frames)
    {
        AMPLITUDE_ASSERT(in.size() >= inOffset + frames);
        AMPLITUDE_ASSERT(out.size() >= outOffset + frames);

        const AmReal32 step = 1.0f / frames;

        for (AmSize j = 0; j < frames; ++j)
            out[j + outOffset] = in[j + inOffset] * gainCurve.Get(step * j);
    }

    void Gain::ApplyAccumulateGain(
        Curve gainCurve, const AudioBufferChannel& in, AmSize inOffset, AudioBufferChannel& out, AmSize outOffset, AmSize frames)
    {
        AMPLITUDE_ASSERT(in.size() >= inOffset + frames);
        AMPLITUDE_ASSERT(out.size() >= outOffset + frames);

        const AmReal32 step = 1.0f / frames;

        for (AmSize j = 0; j < frames; ++j)
            out[j + outOffset] += in[j + inOffset] * gainCurve.Get(step * j);
    }

    bool Gain::IsZero(AmReal32 gain)
    {
        return AM_ABS(gain) < kGainThreshold;
    }

    bool Gain::IsOne(AmReal32 gain)
    {
        const AmReal32 k = 1.0f - gain;
        return AM_ABS(k) < kGainThreshold;
    }

    AmVector2 Gain::CalculateStereoPannedGain(AmReal32 gain, AmVector3 sourcePosition, AmMatrix4 listenerViewMatrix)
    {
        if (IsZero(gain))
            return kVector2Zero;

        const auto& listenerSpaceSourcePosition =
            Transform(listenerViewMatrix, { sourcePosition.x, sourcePosition.y, sourcePosition.z, 1.0f });
        if (SquaredLength(listenerSpaceSourcePosition.xyz) <= kEpsilon)
            return CalculateStereoPannedGain(gain, 0);

        const AmVector3 direction = Normalize(listenerSpaceSourcePosition.xyz);

        return CalculateStereoPannedGain(gain, SphericalPosition::ForHRTF(direction));
    }

    AmVector2 Gain::CalculateStereoPannedGain(AmReal32 gain, AmReal32 pan)
    {
        if (IsZero(gain))
            return kVector2Zero;

        // Clamp pan to its valid range of -1.0f to 1.0f inclusive
        pan = AM_CLAMP(pan, -1.0f, 1.0f);

        // Convert gain and pan to left and right gain
        // This formula is explained in the following paper:
        // http://www.rs-met.com/documents/tutorials/PanRules.pdf
        const AmReal32 p = static_cast<AmReal32>(M_PI) * (pan + 1.0f) / 4.0f;
        const AmReal32 left = std::cos(p);
        const AmReal32 right = std::sin(p);

        return { left * left * gain, right * right * gain };
    }

    AmVector2 Gain::CalculateStereoPannedGain(AmReal32 gain, SphericalPosition sourcePosition)
    {
        if (IsZero(gain))
            return kVector2Zero;

        const AmReal32 cosTheta = std::cos(sourcePosition.GetElevation());

        return { 0.5f * (1.0f + std::cos((AM_DegToRad * -90.0f) - sourcePosition.GetAzimuth()) * cosTheta) * gain,
                 0.5f * (1.0f + std::cos((AM_DegToRad * +90.0f) - sourcePosition.GetAzimuth()) * cosTheta) * gain };
    }

    namespace
    {
        AM_INLINE bool IsSimdAligned(const AmReal32* ptr)
        {
#if defined(AM_SIMD_INTRINSICS)
            return reinterpret_cast<std::uintptr_t>(ptr) % AM_SIMD_ALIGNMENT == 0;
#else
            return true;
#endif // AM_SIMD_INTRINSICS
        }
    } // namespace

    GainProcessor::GainProcessor()
        : _currentGain(0.0f)
        , _targetGain(0.0f)
        , _step(0.0f)
        , _remainingFrames(0)
        , _minRampFrames(kDefaultGainRampMinFrames)
        , _isInitialized(false)
    {}

    GainProcessor::GainProcessor(AmReal32 initialGain)
        : GainProcessor()
    {
        Reset(initialGain);
    }

    void GainProcessor::ApplyGain(
        AmReal32 gain,
        const AudioBufferChannel& in,
        AmSize inOffset,
        AudioBufferChannel& out,
        AmSize outOffset,
        AmSize frames,
        bool accumulate)
    {
        AMPLITUDE_ASSERT(inOffset + frames <= in.size());
        AMPLITUDE_ASSERT(outOffset + frames <= out.size());

        if (frames == 0)
            return;

        const AmSize rampFrames = BeginBlock(gain, frames);
        const AmReal32* src = in.begin() + inOffset;
        AmReal32* dst = out.begin() + outOffset;

        if (rampFrames > 0)
            LinearGainRamp(_currentGain, _step, src, dst, rampFrames, accumulate);

        EndRamp(rampFrames);

        if (rampFrames < frames)
            ConstantGain(_currentGain, src + rampFrames, dst + rampFrames, frames - rampFrames, accumulate);
    }

    void GainProcessor::Advance(AmReal32 gain, AmSize frames)
    {
        if (frames == 0)
            return;

        EndRamp(BeginBlock(gain, frames));
    }

    void GainProcessor::Reset(AmReal32 gain)
    {
        if (!std::isfinite(gain))
            gain = 0.0f;

        _currentGain = gain;
        _targetGain = gain;
        _step = 0.0f;
        _remainingFrames = 0;
        _isInitialized = true;
    }

    void GainProcessor::SetGain(AmReal32 gain)
    {
        Reset(gain);
    }

    void GainProcessor::Invalidate()
    {
        _currentGain = 0.0f;
        _targetGain = 0.0f;
        _step = 0.0f;
        _remainingFrames = 0;
        _isInitialized = false;
    }

    void GainProcessor::SetMinRampFrames(AmSize frames)
    {
        _minRampFrames = AM_MAX(frames, AmSize(1));
    }

    AmSize GainProcessor::BeginBlock(AmReal32 gain, AmSize frames)
    {
        if (!std::isfinite(gain))
            gain = 0.0f;

        if (!_isInitialized)
        {
            Reset(gain);
            return 0;
        }

        if (std::abs(gain - _targetGain) > kEpsilon)
        {
            AmSize length = AM_MAX(frames, _minRampFrames);

#if defined(AM_SIMD_INTRINSICS)
            length = AM_VALUE_ALIGN(length, GetSimdBlockSize());
#endif // AM_SIMD_INTRINSICS

            _targetGain = gain;
            _remainingFrames = length;
            _step = (_targetGain - _currentGain) / static_cast<AmReal32>(length);
        }

        return AM_MIN(frames, _remainingFrames);
    }

    void GainProcessor::EndRamp(AmSize rampFrames)
    {
        if (rampFrames == 0)
            return;

        _remainingFrames -= rampFrames;

        if (_remainingFrames == 0)
            _currentGain = _targetGain; // Land exactly on the target, no float drift.
        else
            _currentGain += _step * static_cast<AmReal32>(rampFrames);
    }

    void GainProcessor::LinearGainRamp(AmReal32 startGain, AmReal32 step, const AmReal32* in, AmReal32* out, AmSize frames, bool accumulate)
    {
        AMPLITUDE_ASSERT(in != nullptr && out != nullptr);

        AmReal32 currentGain = startGain;
        AmSize frame = 0;

#if defined(AM_SIMD_INTRINSICS)
        constexpr AmSize blockSize = GetSimdBlockSize();
        const AmSize simdEnd = IsSimdAligned(in) && IsSimdAligned(out) ? (frames / blockSize) * blockSize : 0;

        if (simdEnd > 0)
        {
            // Build initial gain vector: {g, g+s, g+2s, g+3s, ...}
            alignas(AM_SIMD_ALIGNMENT) AmReal32 gainValues[blockSize];
            for (AmSize i = 0; i < blockSize; ++i)
                gainValues[i] = currentGain + step * static_cast<AmReal32>(i);

            auto gainVec = xsimd::load_aligned<simd_arch>(gainValues);
            const auto stepVec = simd_batch(step * static_cast<AmReal32>(blockSize));

            if (accumulate)
            {
                for (; frame < simdEnd; frame += blockSize)
                {
                    const auto inVec = xsimd::load_aligned<simd_arch>(in + frame);
                    const auto outVec = xsimd::load_aligned<simd_arch>(out + frame);
                    xsimd::store_aligned<simd_arch>(out + frame, xsimd::fma(inVec, gainVec, outVec));
                    gainVec += stepVec;
                }
            }
            else
            {
                for (; frame < simdEnd; frame += blockSize)
                {
                    const auto inVec = xsimd::load_aligned<simd_arch>(in + frame);
                    xsimd::store_aligned<simd_arch>(out + frame, inVec * gainVec);
                    gainVec += stepVec;
                }
            }

            currentGain = startGain + step * static_cast<AmReal32>(simdEnd);
        }
#endif // AM_SIMD_INTRINSICS

        if (accumulate)
        {
            for (; frame < frames; ++frame)
            {
                out[frame] += currentGain * in[frame];
                currentGain += step;
            }
        }
        else
        {
            for (; frame < frames; ++frame)
            {
                out[frame] = currentGain * in[frame];
                currentGain += step;
            }
        }
    }

    void GainProcessor::ConstantGain(AmReal32 gain, const AmReal32* in, AmReal32* out, AmSize frames, bool accumulate)
    {
        if (Gain::IsZero(gain))
        {
            if (!accumulate)
                std::fill_n(out, frames, 0.0f);

            return;
        }

        const bool aligned = IsSimdAligned(in) && IsSimdAligned(out);

        if (accumulate)
        {
            if (aligned)
                ScalarMultiplyAccumulate(in, out, gain, frames);
            else
                for (AmSize i = 0; i < frames; ++i)
                    out[i] += in[i] * gain;

            return;
        }

        if (Gain::IsOne(gain))
        {
            if (in != out)
                std::memmove(out, in, frames * sizeof(AmReal32));
        }
        else if (aligned)
            ScalarMultiply(in, out, gain, frames);
        else
            for (AmSize i = 0; i < frames; ++i)
                out[i] = in[i] * gain;
    }
} // namespace SparkyStudios::Audio::Amplitude
