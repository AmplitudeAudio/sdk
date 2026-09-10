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

#include <Utils/Audio/Reverb/DattoroReverb.h>

namespace SparkyStudios::Audio::Amplitude::Internal
{
    DattoroReverb::DattoroReverb(AmUInt64 sampleRate)
        : BaseReverb(sampleRate)
    {
        std::memset(_gains, 0, sizeof(AmReal32) * kNumDattaroGains);
        std::memset(_tapPos, 0, sizeof(AmUInt32) * kNumDattaroTaps);
        std::memset(_tap, 0, sizeof(AmReal32) * kNumDattaroTaps);
        std::memset(_fixedDelayLengths, 0, sizeof(AmUInt32) * kNumFixedDelays);
        std::memset(_times, 0, sizeof(AmUInt32) * 4);

        _sigL = 0.0;
        _sigR = 0.0;

        // original dattaro del tap lengths for 29.8 khz SR
        AmUInt32 orig[kNumDattaroTaps] = { 266, 2974, 1913, 1996, 1990, 187, 1066, 353, 3627, 1228, 2673, 2111, 335, 121 };

        // 1 ms at 29.8 khz SR;
        AmReal32 dms = 29.8;
        AmReal32 cms = static_cast<AmReal32>(sampleRate) / 1000.0f;
        for (AmUInt32 i = 0; i < kNumDattaroTaps; i++)
        {
            AmReal32 prevDelayLengthMs = static_cast<AmReal32>(orig[i]) / dms;
            AmUInt32 newDelayLength = std::floor(prevDelayLengthMs * cms);
            _tapPos[i] = newDelayLength;
        }

        // original dattaro delay lengths
        AmUInt32 originalDelays[kNumFixedDelays] = { 4217, 3163, 4453, 3720 };
        for (AmUInt32 i = 0; i < kNumFixedDelays; i++)
        {
            AmReal32 prevDelayLengthMs = static_cast<AmReal32>(originalDelays[i]) / dms;
            AmUInt32 newDelayLength = std::floor(prevDelayLengthMs * cms);
            _fixedDelayLengths[i] = newDelayLength;
        }

        // initial delay
        _fixedDelayLengths[4] = std::floor((3100.0f / dms) * cms);

        // set gains
        AmReal32 presetGains[kNumDattaroGains] = { 0.75f, 0.625f, 0.70f, 0.50f, 0.30f };
        for (AmUInt32 i = 0; i < kNumDattaroGains; i++)
            _gains[i] = presetGains[i];

        // set fb ap
        constexpr AmUInt32 kNumTap = 8;
        AmUInt32 originalTapLengths[kNumTap] = { 142, 107, 379, 277, 908, 2656, 672, 1800 };
        for (AmUInt32 i = 0; i < kNumTap; i++)
        {
            _delays[i].Init(sampleRate);

            AmReal32 prevLength = static_cast<AmReal32>(originalTapLengths[i]) / dms;
            AmUInt32 newLength = std::floor(prevLength * cms);
            _feedBackAllPass[i] = newLength;
        }
    }

    void DattoroReverb::Process(AmReal32 x, AmReal32& l, AmReal32& r)
    {
        // Reverb implementation from Jon Dattoro's design book:
        // https://ccrma.stanford.edu/~dattorro/EffectDesignPart1.pdf
        //
        // 1. The inital reflections are subject to more processing
        // 2. Interaction and feedback between two channels
        //    for great stereo reverb
        // 3. Low pass filters used throughout the structure
        //    to alter response
        // 4. Output is composed of taps from various points
        //    in the structure, rather than being taken
        //    off from the end. The way this is done really
        //    flattens the frequency response associated with combs
        //
        // All above params adapted from the paper and were subject to further tuning.

        AmReal32 a = _delays[4].OneTap(x, _fixedDelayLengths[4]);

        AmReal32 b = _arrayLowPass[0].LowPass(a, 0.8f);
        AmReal32 c = SerialAllPass(b, 0, 2, _gains[0]);
        AmReal32 d = SerialAllPass(c, 2, 2, _gains[1]);
        const AmReal32 oldSigL = _sigL;
        const AmReal32 inL = d + _gains[4] * _sigR;
        _sigR = d + _gains[4] * oldSigL;
        _sigL = _arrayAllPass[4].AllPass(inL, _feedBackAllPass[4], _gains[2]);
        _sigL = _delays[0].OneTap(_sigL, _fixedDelayLengths[0]);
        _tap[0] = _delays[0].GetTap(_tapPos[0]);
        _tap[1] = _delays[0].GetTap(_tapPos[1]);
        _tap[11] = _delays[0].GetTap(_tapPos[11]);
        _sigL = _arrayLowPass[1].LowPass(_sigL, 0.4f);
        _sigL = _arrayAllPass[5].AllPass(_sigL, _feedBackAllPass[5], _gains[3]);
        _tap[2] = _arrayAllPass[5].GetTap(_tapPos[2]);
        _tap[12] = _arrayAllPass[5].GetTap(_tapPos[12]);
        _sigL = _delays[1].OneTap(_sigL, _fixedDelayLengths[1]);
        _tap[3] = _delays[1].GetTap(_tapPos[3]);
        _tap[13] = _delays[1].GetTap(_tapPos[13]);

        _sigR = _arrayAllPass[6].AllPass(_sigR, _feedBackAllPass[6], _gains[2]);
        _sigR = _delays[2].OneTap(_sigR, _fixedDelayLengths[2]);
        _tap[4] = _delays[2].GetTap(_tapPos[4]);
        _tap[7] = _delays[2].GetTap(_tapPos[7]);
        _tap[8] = _delays[2].GetTap(_tapPos[8]);
        _sigR = _arrayLowPass[2].LowPass(_sigR, 0.4f);
        _sigR = _arrayAllPass[7].AllPass(_sigR, _feedBackAllPass[7], _gains[3]);
        _tap[5] = _arrayAllPass[7].GetTap(_tapPos[5]);
        _tap[9] = _arrayAllPass[7].GetTap(_tapPos[9]);
        _sigR = _delays[3].OneTap(_sigR, _fixedDelayLengths[3]);
        _tap[6] = _delays[3].GetTap(_tapPos[6]);
        _tap[10] = _delays[3].GetTap(_tapPos[10]);

        l = _tap[0] + _tap[1] - _tap[2] + _tap[3] - _tap[4] - _tap[5] - _tap[6];
        r = _tap[7] + _tap[8] - _tap[9] + _tap[10] - _tap[11] - _tap[12] - _tap[13];
    }

    void DattoroReverb::Process(const AudioBuffer& input, AudioBuffer& output)
    {
        AMPLITUDE_ASSERT(input.GetChannelCount() == kAmMonoChannelCount);
        AMPLITUDE_ASSERT(output.GetChannelCount() == kAmStereoChannelCount);
        AMPLITUDE_ASSERT(input.GetFrameCount() == output.GetFrameCount());

        const auto& inChannel = input[0];
        /* */ auto& outLChannel = output[0];
        /* */ auto& outRChannel = output[1];

        for (AmUInt32 i = 0, m = inChannel.size(); i < m; i++)
            Process(inChannel[i], outLChannel[i], outRChannel[i]);
    }

    void DattoroReverb::SetRoomParameters(AmReal32 roomSize, AmReal32 absorption)
    {
        _gains[4] = AM_CLAMP(roomSize, 0.0f, 1.0f);
    }

    void DattoroReverb::Mute()
    {
        BaseReverb::Mute();

        for (auto& delay : _delays)
            delay.Mute();

        std::memset(_tap, 0, sizeof(AmReal32) * kNumDattaroTaps);
        _sigL = 0.0f;
        _sigR = 0.0f;
    }
} // namespace SparkyStudios::Audio::Amplitude
