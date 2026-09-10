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

#include <Utils/Audio/Reverb/RoomReverb.h>

namespace SparkyStudios::Audio::Amplitude
{
    RoomReverb::RoomReverb(AmUInt64 sampleRate)
        : BaseReverb(sampleRate)
    {
        constexpr AmUInt32 kNumCombs = 16;
        constexpr AmUInt32 kNumPass = 8;
        constexpr AmUInt32 kStereoSpread = 23;

        AmReal32 combTimes[kNumCombs] = { 1557, 1617, 1491, 1422, 1277, 1356, 1188, 1116 };
        for (int i = kNumCombs / 2; i < kNumCombs; i++)
            combTimes[i] = combTimes[i - (kNumCombs / 2)] + kStereoSpread;

        SetCombTimes(combTimes, kNumCombs);

        AmReal32 combGains[kNumCombs];
        AmReal32 cutoff[kNumCombs];
        for (int i = 0; i < kNumCombs; i++)
        {
            combGains[i] = 0.84f;
            cutoff[i] = 0.20f;
        }

        SetCombWeights(combGains, kNumCombs);
        SetLowPassCombCutOff(cutoff, kNumCombs);

        AmReal32 passTimes[kNumPass] = { 225, 556, 441, 341 };
        for (int i = kNumPass / 2; i < kNumPass; i++)
            passTimes[i] = passTimes[i - (kNumPass / 2)] + kStereoSpread;

        SetAllPassTimes(passTimes, kNumPass);

        AmReal32 passGains[kNumPass] = { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };
        SetAllPassWeights(passGains, kNumPass);
    }

    void RoomReverb::SetRoomSize(AmReal32 roomSize)
    {
        roomSize = AM_CLAMP(roomSize, 0.0f, 1.0f);
        SetCombWeightsAll((roomSize * 0.28f) + 0.70f);
    }

    void RoomReverb::SetRoomAbsorption(AmReal32 roomAbsorption)
    {
        roomAbsorption = AM_CLAMP(roomAbsorption, 0.0f, 1.0f);
        SetLowPassCombCutOff(roomAbsorption);
    }

    void RoomReverb::Process(AmReal32 x, AmReal32& l, AmReal32& r)
    {
        AmReal32 l1 = ParallelComb(x, 0, 8) - (8.0f * x);
        l = SerialAllPass(l1, 0, 4);

        AmReal32 r1 = ParallelComb(x, 8, 8) - (8.0f * x);
        r = SerialAllPass(r1, 4, 4);
    }

    void RoomReverb::Process(const AudioBuffer& input, AudioBuffer& output)
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

} // namespace SparkyStudios::Audio::Amplitude
