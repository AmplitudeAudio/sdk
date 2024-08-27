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

#include <DSP/Gain.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    constexpr AmReal32 kGainThreshold = 0.001f;

    void Gain::ApplyReplaceConstantGain(
        AmReal32 gain, const AudioBuffer& in, AmSize inOffset, AudioBuffer& out, AmSize outOffset, AmSize frames)
    {
        AMPLITUDE_ASSERT(in.GetChannelCount() == out.GetChannelCount());
        AMPLITUDE_ASSERT(in.GetFrameCount() <= inOffset + frames);
        AMPLITUDE_ASSERT(out.GetFrameCount() <= outOffset + frames);

        for (AmSize i = 0, l = in.GetChannelCount(); i < l; ++i)
        {
            const auto& inChannel = in[i];
            auto& outChannel = out[i];

            ScalarMultiply(inChannel.begin() + inOffset, outChannel.begin() + outOffset, gain, frames);
        }
    }

    void Gain::ApplyAdditiveConstantGain(
        AmReal32 gain, const AudioBuffer& in, AmSize inOffset, AudioBuffer& out, AmSize outOffset, AmSize frames)
    {
        AMPLITUDE_ASSERT(in.GetChannelCount() == out.GetChannelCount());
        AMPLITUDE_ASSERT(in.GetFrameCount() <= inOffset + frames);
        AMPLITUDE_ASSERT(out.GetFrameCount() <= outOffset + frames);

        for (AmSize i = 0, l = in.GetChannelCount(); i < l; ++i)
        {
            const auto& inChannel = in[i];
            auto& outChannel = out[i];

            for (AmSize j = 0; j < frames; ++j)
            {
                outChannel[j + outOffset] += inChannel[j + inOffset] * gain;
            }
        }
    }

    void Gain::ApplyReplaceLinearGain(
        AmReal32 startGain, AmReal32 endGain, const AudioBuffer& in, AmSize inOffset, AudioBuffer& out, AmSize outOffset, AmSize frames)
    {
        AMPLITUDE_ASSERT(in.GetChannelCount() == out.GetChannelCount());
        AMPLITUDE_ASSERT(in.GetFrameCount() <= inOffset + frames);
        AMPLITUDE_ASSERT(out.GetFrameCount() <= outOffset + frames);

        const AmReal32 step = 1.0f / frames;

        for (AmSize i = 0, l = in.GetChannelCount(); i < l; ++i)
        {
            const auto& inChannel = in[i];
            auto& outChannel = out[i];

            for (AmSize j = 0; j < frames; ++j)
            {
                outChannel[j + outOffset] = inChannel[j + inOffset] * AM_Lerp(startGain, step * j, endGain);
            }
        }
    }

    void Gain::ApplyAdditiveLinearGain(
        AmReal32 startGain, AmReal32 endGain, const AudioBuffer& in, AmSize inOffset, AudioBuffer& out, AmSize outOffset, AmSize frames)
    {
        AMPLITUDE_ASSERT(in.GetChannelCount() == out.GetChannelCount());
        AMPLITUDE_ASSERT(in.GetFrameCount() <= inOffset + frames);
        AMPLITUDE_ASSERT(out.GetFrameCount() <= outOffset + frames);

        const AmReal32 step = 1.0f / frames;

        for (AmSize i = 0, l = in.GetChannelCount(); i < l; ++i)
        {
            const auto& inChannel = in[i];
            auto& outChannel = out[i];

            for (AmSize j = 0; j < frames; ++j)
            {
                outChannel[j + outOffset] += inChannel[j + inOffset] * AM_Lerp(startGain, step * j, endGain);
            }
        }
    }

    void Gain::ApplyReplaceGain(Curve gainCurve, const AudioBuffer& in, AmSize inOffset, AudioBuffer& out, AmSize outOffset, AmSize frames)
    {
        AMPLITUDE_ASSERT(in.GetChannelCount() == out.GetChannelCount());
        AMPLITUDE_ASSERT(in.GetFrameCount() <= inOffset + frames);
        AMPLITUDE_ASSERT(out.GetFrameCount() <= outOffset + frames);

        const AmReal32 step = 1.0f / frames;

        for (AmSize i = 0, l = in.GetChannelCount(); i < l; ++i)
        {
            const auto& inChannel = in[i];
            auto& outChannel = out[i];

            for (AmSize j = 0; j < frames; ++j)
            {
                outChannel[j + outOffset] = inChannel[j + inOffset] * gainCurve.Get(step * j);
            }
        }
    }

    void Gain::ApplyAdditiveGain(Curve gainCurve, const AudioBuffer& in, AmSize inOffset, AudioBuffer& out, AmSize outOffset, AmSize frames)
    {
        AMPLITUDE_ASSERT(in.GetChannelCount() == out.GetChannelCount());
        AMPLITUDE_ASSERT(in.GetFrameCount() <= inOffset + frames);
        AMPLITUDE_ASSERT(out.GetFrameCount() <= outOffset + frames);

        const AmReal32 step = 1.0f / frames;

        for (AmSize i = 0, l = in.GetChannelCount(); i < l; ++i)
        {
            const auto& inChannel = in[i];
            auto& outChannel = out[i];

            for (AmSize j = 0; j < frames; ++j)
            {
                outChannel[j + outOffset] += inChannel[j + inOffset] * gainCurve.Get(step * j);
            }
        }
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
} // namespace SparkyStudios::Audio::Amplitude
