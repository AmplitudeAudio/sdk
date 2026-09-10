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
#include <SparkyStudios/Audio/Amplitude/DSP/Reverbs/FreeverbReverb.h>

#include <Utils/Freeverb/ReverbModel.h>

#include <algorithm>

namespace SparkyStudios::Audio::Amplitude
{
    FreeverbReverbInstance::FreeverbReverbInstance(FreeverbReverb* parent)
        : ReverbInstance(parent)
        , _model(std::make_unique<Freeverb::ReverbModel>())
        , _scratchBuffer(kAmMaxSupportedFrameCount, 2)
    {
        _model->SetWidth(1.0f);
        _model->SetWet(1.0f);
        _model->SetDry(0.0f);
        _model->SetMode(0.0f);
    }

    FreeverbReverbInstance::~FreeverbReverbInstance() = default;

    void FreeverbReverbInstance::Initialize(AmUInt32 sampleRate)
    {
        if (_model != nullptr)
        {
            _model->SetSampleRate(sampleRate);
            _model->Mute();
        }

        if (_scratchBuffer.GetFrameCount() == 0)
            _scratchBuffer = AudioBuffer(kAmMaxSupportedFrameCount, 2);
    }

    void FreeverbReverbInstance::Reset()
    {
        if (_model != nullptr)
            _model->Mute();
    }

    void FreeverbReverbInstance::SetRoomParameters(AmReal32 roomSize, AmReal32 absorption)
    {
        if (_model != nullptr)
        {
            _model->SetRoomSize(AM_CLAMP(roomSize, 0.0f, 1.0f));
            _model->SetDamp(absorption);
        }
    }

    void FreeverbReverbInstance::Configure(AmUInt64 frames)
    {
        if (_scratchBuffer.GetFrameCount() < frames)
            _scratchBuffer = AudioBuffer(frames, 2);
    }

    void FreeverbReverbInstance::Process(const AudioBuffer& in, AudioBuffer& out, AmUInt64 frames, AmUInt32 sampleRate)
    {
        if (_model == nullptr || frames == 0 || in.GetChannelCount() == 0 || out.GetChannelCount() == 0)
            return;

        const AmReal32* inL = in[0].begin();
        const AmReal32* inR = in.GetChannelCount() > 1 ? in[1].begin() : inL;

        if (out.GetChannelCount() >= 2)
        {
            _model->ProcessReplace(inL, inR, out[0].begin(), out[1].begin(), frames, 1);
        }
        else
        {
            if (_scratchBuffer.IsEmpty())
                return;

            AmUInt64 remaining = frames;
            AmUInt64 offset = 0;
            const AmUInt64 chunkSize = _scratchBuffer.GetFrameCount();

            while (remaining > 0)
            {
                const AmUInt64 chunk = std::min(remaining, chunkSize);
                _model->ProcessReplace(inL + offset, inR + offset, _scratchBuffer[0].begin(), _scratchBuffer[1].begin(), chunk, 1);

                for (AmUInt64 f = 0; f < chunk; ++f)
                    out[0][offset + f] = 0.5f * (_scratchBuffer[0][f] + _scratchBuffer[1][f]);

                offset += chunk;
                remaining -= chunk;
            }
        }
    }

    FreeverbReverb::FreeverbReverb()
        : Reverb("Freeverb")
    {}

    std::shared_ptr<ReverbInstance> FreeverbReverb::CreateInstance()
    {
        return ampoolshared(eMemoryPoolKind_Filtering, FreeverbReverbInstance, this);
    }
} // namespace SparkyStudios::Audio::Amplitude
