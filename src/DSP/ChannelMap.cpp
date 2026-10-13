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

#include <DSP/ChannelMap.h>
#include <DSP/SpeakerPanner.h>

namespace SparkyStudios::Audio::Amplitude
{
    ChannelMatrix BuildChannelMap(const ChannelLayout& from, const ChannelLayout& to)
    {
        if (from == to)
            return ChannelLayout::GetFoldDownMatrix(from, to);

        ChannelMatrix matrix;
        matrix.inputCount = from.GetChannelCount();
        matrix.outputCount = to.GetChannelCount();

        if (from.GetType() == eChannelLayout_Mono)
        {
            if (const AmInt32 centre = to.FindSpeaker(eSpeaker_FrontCenter); centre >= 0)
            {
                matrix.gains[centre][0] = 1.0f;
            }
            else
            {
                matrix.gains[to.FindSpeaker(eSpeaker_FrontLeft)][0] = kAmMinus3dB;
                matrix.gains[to.FindSpeaker(eSpeaker_FrontRight)][0] = kAmMinus3dB;
            }

            return matrix;
        }

        if (to.GetChannelCount() < from.GetChannelCount())
            return ChannelLayout::GetFoldDownMatrix(from, to);

        for (AmUInt16 i = 0; i < from.GetChannelCount(); ++i)
        {
            if (from.IsLfe(i))
            {
                if (const AmInt32 lfe = to.GetLfeChannel(); lfe >= 0)
                    matrix.gains[lfe][i] = 1.0f;

                continue;
            }

            AmInt32 nearest = -1;
            AmReal32 nearestDistance = 360.0f;
            for (AmUInt16 o = 0; o < to.GetChannelCount(); ++o)
            {
                if (to.IsLfe(o))
                    continue;

                const AmReal32 distance = std::abs(WrapDegrees(to.GetAzimuth(o) - from.GetAzimuth(i)));
                if (distance < nearestDistance)
                {
                    nearest = o;
                    nearestDistance = distance;
                }
            }

            matrix.gains[nearest][i] = 1.0f;
        }

        return matrix;
    }

    void ApplyChannelMap(const ChannelMatrix& matrix, const AudioBuffer& input, AudioBuffer& output, AmUInt64 frames)
    {
        const AmUInt16 inputs = std::min<AmUInt16>(matrix.inputCount, static_cast<AmUInt16>(input.GetChannelCount()));

        for (AmUInt16 o = 0; o < output.GetChannelCount(); ++o)
        {
            auto& out = output.GetChannel(o);
            std::fill_n(out.begin(), frames, 0.0f);

            if (o >= matrix.outputCount)
                continue;

            for (AmUInt16 i = 0; i < inputs; ++i)
            {
                const AmReal32 gain = matrix.gains[o][i];
                if (gain == 0.0f)
                    continue;

                const auto& in = input.GetChannel(i);
                for (AmUInt64 f = 0; f < frames; ++f)
                    out[f] += gain * in[f];
            }
        }
    }
} // namespace SparkyStudios::Audio::Amplitude
