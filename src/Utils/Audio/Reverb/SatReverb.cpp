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

#include <Utils/Audio/Reverb/SatReverb.h>

namespace SparkyStudios::Audio::Amplitude::Internal
{
    SatReverb::SatReverb(AmUInt64 sampleRate)
        : BaseReverb(sampleRate)
    {
        AmReal32 combTimes[4] = { 778, 901, 1011, 1123 };
        SetCombTimes(combTimes, 4);

        AmReal32 combWeights[4] = { 0.805, 0.827, 0.783, 0.764 };
        SetCombWeights(combWeights, 4);

        AmReal32 allPassTimes[3] = { 125, 42, 12 };
        SetAllPassTimes(allPassTimes, 3);

        AmReal32 allPassWeights[3] = { 0.7, 0.7, 0.7 };
        SetAllPassWeights(allPassWeights, 3);
    }

    void SatReverb::Process(AmReal32 x, AmReal32& l, AmReal32& r)
    {
        // Structure created by Chowning (1971) : https://ccrma.stanford.edu/~jos/pasp/Example_Schroeder_Reverberators.html
        // 4 parallel combs -> 3 serial all pass
        x = ParallelComb(x, 0, 4);
        AmReal32 y = SerialAllPass(x, 0, 3);

        l = +y;
        r = -y;
    }

    void SatReverb::Process(const AudioBuffer& input, AudioBuffer& output)
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
