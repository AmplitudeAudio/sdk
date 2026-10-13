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

#include <SparkyStudios/Audio/Amplitude/DSP/ChannelLayout.h>

namespace SparkyStudios::Audio::Amplitude
{
    namespace
    {
        ChannelMatrix Identity(AmUInt16 count)
        {
            ChannelMatrix matrix;
            matrix.inputCount = matrix.outputCount = count;
            for (AmUInt16 c = 0; c < count; ++c)
                matrix.gains[c][c] = 1.0f;

            return matrix;
        }

        /// @p second applied after @p first.
        ChannelMatrix Compose(const ChannelMatrix& second, const ChannelMatrix& first)
        {
            ChannelMatrix matrix;
            matrix.inputCount = first.inputCount;
            matrix.outputCount = second.outputCount;

            for (AmUInt16 o = 0; o < second.outputCount; ++o)
                for (AmUInt16 i = 0; i < first.inputCount; ++i)
                    for (AmUInt16 k = 0; k < first.outputCount; ++k)
                        matrix.gains[o][i] += second.gains[o][k] * first.gains[k][i];

            return matrix;
        }

        ChannelMatrix Make(AmUInt16 inputs, AmUInt16 outputs)
        {
            ChannelMatrix matrix;
            matrix.inputCount = inputs;
            matrix.outputCount = outputs;
            return matrix;
        }

        /// 7.1 (L R C LFE Lb Rb Ls Rs) to 5.1 (L R C LFE Ls Rs): each surround takes its side and back speakers.
        ChannelMatrix Fold71To51()
        {
            ChannelMatrix m = Make(8, 6);
            m.gains[0][0] = m.gains[1][1] = m.gains[2][2] = m.gains[3][3] = 1.0f;
            m.gains[4][4] = m.gains[4][6] = kAmMinus3dB;
            m.gains[5][5] = m.gains[5][7] = kAmMinus3dB;
            return m;
        }

        /// 5.1 to quad (L R Lb Rb): the centre into the front pair, the surrounds to the rears, the LFE dropped.
        ChannelMatrix Fold51ToQuad()
        {
            ChannelMatrix m = Make(6, 4);
            m.gains[0][0] = m.gains[1][1] = 1.0f;
            m.gains[0][2] = m.gains[1][2] = kAmMinus3dB;
            m.gains[2][4] = m.gains[3][5] = 1.0f;
            return m;
        }

        /// ITU-R BS.775: L' = L + 0.707 C + 0.707 Ls, R' = R + 0.707 C + 0.707 Rs, the LFE dropped.
        ChannelMatrix Fold51ToStereo()
        {
            ChannelMatrix m = Make(6, 2);
            m.gains[0][0] = m.gains[1][1] = 1.0f;
            m.gains[0][2] = m.gains[1][2] = kAmMinus3dB;
            m.gains[0][4] = m.gains[1][5] = kAmMinus3dB;
            return m;
        }

        ChannelMatrix FoldQuadToStereo()
        {
            ChannelMatrix m = Make(4, 2);
            m.gains[0][0] = m.gains[1][1] = 1.0f;
            m.gains[0][2] = m.gains[1][3] = kAmMinus3dB;
            return m;
        }

        ChannelMatrix FoldStereoToMono()
        {
            ChannelMatrix m = Make(2, 1);
            m.gains[0][0] = m.gains[0][1] = kAmMinus3dB;
            return m;
        }
    } // namespace

    ChannelLayout::ChannelLayout(eChannelLayout type, std::initializer_list<SpeakerPosition> speakers)
        : _type(type)
        , _channelCount(static_cast<AmUInt16>(speakers.size()))
    {
        AmUInt16 c = 0;
        for (const SpeakerPosition& position : speakers)
        {
            _speakers[c] = position.speaker;
            _azimuths[c] = position.azimuth;
            ++c;
        }
    }

    const ChannelLayout& ChannelLayout::Get(eChannelLayout layout)
    {
        static const std::array<ChannelLayout, eChannelLayout_COUNT> kLayouts{
            ChannelLayout(eChannelLayout_Mono, { { eSpeaker_FrontCenter, 0.0f } }),
            ChannelLayout(eChannelLayout_Stereo, { { eSpeaker_FrontLeft, 30.0f }, { eSpeaker_FrontRight, -30.0f } }),
            ChannelLayout(
                eChannelLayout_Quad,
                { { eSpeaker_FrontLeft, 45.0f }, { eSpeaker_FrontRight, -45.0f }, { eSpeaker_BackLeft, 135.0f }, { eSpeaker_BackRight, -135.0f } }),
            ChannelLayout(
                eChannelLayout_Surround_5_1,
                { { eSpeaker_FrontLeft, 30.0f },
                  { eSpeaker_FrontRight, -30.0f },
                  { eSpeaker_FrontCenter, 0.0f },
                  { eSpeaker_LowFrequency, 0.0f },
                  { eSpeaker_SideLeft, 110.0f },
                  { eSpeaker_SideRight, -110.0f } }),
            ChannelLayout(
                eChannelLayout_Surround_7_1,
                { { eSpeaker_FrontLeft, 30.0f },
                  { eSpeaker_FrontRight, -30.0f },
                  { eSpeaker_FrontCenter, 0.0f },
                  { eSpeaker_LowFrequency, 0.0f },
                  { eSpeaker_BackLeft, 150.0f },
                  { eSpeaker_BackRight, -150.0f },
                  { eSpeaker_SideLeft, 90.0f },
                  { eSpeaker_SideRight, -90.0f } }),
        };

        AMPLITUDE_ASSERT(layout < eChannelLayout_COUNT);
        return kLayouts[AM_MIN(static_cast<AmSize>(layout), kLayouts.size() - 1)];
    }

    const ChannelLayout* ChannelLayout::FromChannelCount(AmUInt16 channelCount)
    {
        switch (channelCount)
        {
        case 1:
            return &Get(eChannelLayout_Mono);
        case 2:
            return &Get(eChannelLayout_Stereo);
        case 4:
            return &Get(eChannelLayout_Quad);
        case 6:
            return &Get(eChannelLayout_Surround_5_1);
        case 8:
            return &Get(eChannelLayout_Surround_7_1);
        default:
            return nullptr;
        }
    }

    const ChannelLayout& ChannelLayout::LargestFitting(AmUInt16 channelCount)
    {
        if (channelCount >= 8)
            return Get(eChannelLayout_Surround_7_1);
        if (channelCount >= 6)
            return Get(eChannelLayout_Surround_5_1);
        if (channelCount >= 4)
            return Get(eChannelLayout_Quad);
        if (channelCount == 1)
            return Get(eChannelLayout_Mono);

        return Get(eChannelLayout_Stereo);
    }

    ChannelMatrix ChannelLayout::GetFoldDownMatrix(const ChannelLayout& from, const ChannelLayout& to)
    {
        if (from == to)
            return Identity(from.GetChannelCount());

        if (to.GetChannelCount() > from.GetChannelCount())
            return {};

        // Walk the chain 7.1 -> 5.1 -> (quad | stereo) -> mono and stop at the target.
        ChannelMatrix matrix = Identity(from.GetChannelCount());
        eChannelLayout current = from.GetType();

        if (current == eChannelLayout_Surround_7_1)
        {
            matrix = Compose(Fold71To51(), matrix);
            current = eChannelLayout_Surround_5_1;
        }

        if (current == to.GetType())
            return matrix;

        if (current == eChannelLayout_Surround_5_1)
        {
            if (to.GetType() == eChannelLayout_Quad)
                return Compose(Fold51ToQuad(), matrix);

            matrix = Compose(Fold51ToStereo(), matrix);
            current = eChannelLayout_Stereo;
        }
        else if (current == eChannelLayout_Quad)
        {
            matrix = Compose(FoldQuadToStereo(), matrix);
            current = eChannelLayout_Stereo;
        }

        if (current == to.GetType())
            return matrix;

        return Compose(FoldStereoToMono(), matrix);
    }

    eSpeaker ChannelLayout::GetSpeaker(AmUInt16 channel) const
    {
        AMPLITUDE_ASSERT(channel < _channelCount);
        return _speakers[AM_MIN(channel, static_cast<AmUInt16>(kAmMaxLayoutChannelCount - 1))];
    }

    AmReal32 ChannelLayout::GetAzimuth(AmUInt16 channel) const
    {
        AMPLITUDE_ASSERT(channel < _channelCount);
        return _azimuths[AM_MIN(channel, static_cast<AmUInt16>(kAmMaxLayoutChannelCount - 1))];
    }

    bool ChannelLayout::IsLfe(AmUInt16 channel) const
    {
        return channel < _channelCount && _speakers[channel] == eSpeaker_LowFrequency;
    }

    AmInt32 ChannelLayout::GetLfeChannel() const
    {
        return FindSpeaker(eSpeaker_LowFrequency);
    }

    AmInt32 ChannelLayout::FindSpeaker(eSpeaker speaker) const
    {
        for (AmUInt16 c = 0; c < _channelCount; ++c)
            if (_speakers[c] == speaker)
                return c;

        return -1;
    }

    AmReal32 ChannelLayout::GetNativeWidth() const
    {
        AmReal32 widest = 0.0f;
        for (AmUInt16 c = 0; c < _channelCount; ++c)
            if (!IsLfe(c))
                widest = std::max(widest, std::abs(_azimuths[c]));

        return 2.0f * widest;
    }
} // namespace SparkyStudios::Audio::Amplitude
