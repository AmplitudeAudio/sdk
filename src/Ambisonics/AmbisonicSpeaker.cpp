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

#include <Ambisonics/AmbisonicSpeaker.h>
#include <Ambisonics/BFormat.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    AmbisonicSpeaker::AmbisonicSpeaker()
    {
        Configure(1, true);
        Refresh();
    }

    AmbisonicSpeaker::~AmbisonicSpeaker() = default;

    bool AmbisonicSpeaker::Configure(AmUInt32 order, bool is3D)
    {
        if (!AmbisonicEntity::Configure(order, is3D))
            return false;

        SetOrderWeight(0, std::sqrt(2.0f));
        return true;
    }

    void AmbisonicSpeaker::Refresh()
    {
        AmbisonicEntity::Refresh();
    }

    void AmbisonicSpeaker::Process(BFormat* input, AmUInt32 frameCount, AudioBufferChannel& output)
    {
        AMPLITUDE_ASSERT(output.size() == frameCount);
        output.clear();

        for (AmUInt32 c = 0; c < m_channelCount; ++c)
        {
            auto& channel = input->GetBufferChannel(c);
            ScalarMultiplyAccumulate(channel.begin(), output.begin(), m_coefficients[c], frameCount);
        }
    }
} // namespace SparkyStudios::Audio::Amplitude