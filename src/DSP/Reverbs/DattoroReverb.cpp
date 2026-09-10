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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/DSP/Reverbs/DattoroReverb.h>

#include <Utils/Audio/Reverb/DattoroReverb.h>

namespace SparkyStudios::Audio::Amplitude
{
    DattoroReverbInstance::DattoroReverbInstance(DattoroReverb* parent)
        : ReverbInstance(parent)
        , _reverb(std::make_unique<Internal::DattoroReverb>(48000))
        , _sampleRate(48000)
    {}

    DattoroReverbInstance::~DattoroReverbInstance() = default;

    void DattoroReverbInstance::Initialize(AmUInt32 sampleRate)
    {
        _sampleRate = sampleRate;
        _reverb = std::make_unique<Internal::DattoroReverb>(sampleRate);
    }

    void DattoroReverbInstance::Reset()
    {
        if (_reverb != nullptr)
            _reverb->Mute();
    }

    void DattoroReverbInstance::SetRoomParameters(AmReal32 roomSize, AmReal32 absorption)
    {
        if (_reverb != nullptr)
            _reverb->SetRoomParameters(roomSize, absorption);
    }

    void DattoroReverbInstance::Configure(AmUInt64 frames)
    {}

    void DattoroReverbInstance::Process(const AudioBuffer& in, AudioBuffer& out, AmUInt64 frames, AmUInt32 sampleRate)
    {
        if (_reverb == nullptr || frames == 0 || in.GetChannelCount() == 0 || out.GetChannelCount() == 0)
            return;

        for (AmUInt64 f = 0; f < frames; ++f)
        {
            const AmReal32 x = in.GetChannelCount() > 1 ? 0.5f * (in[0][f] + in[1][f]) : in[0][f];
            AmReal32 l = 0.0f;
            AmReal32 r = 0.0f;

            _reverb->Process(x, l, r);

            if (out.GetChannelCount() > 1)
            {
                out[0][f] = l;
                out[1][f] = r;
            }
            else
            {
                out[0][f] = 0.5f * (l + r);
            }
        }
    }

    DattoroReverb::DattoroReverb()
        : Reverb("Dattorro")
    {}

    std::shared_ptr<ReverbInstance> DattoroReverb::CreateInstance()
    {
        return ampoolshared(eMemoryPoolKind_Filtering, DattoroReverbInstance, this);
    }
} // namespace SparkyStudios::Audio::Amplitude
