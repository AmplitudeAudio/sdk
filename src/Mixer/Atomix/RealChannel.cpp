// Copyright (c) 2021-present Sparky Studios. All rights reserved.
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

#include <cassert>
#include <cmath>
#include <cstdlib>

#include <SparkyStudios/Audio/Amplitude/Core/Log.h>
#include <SparkyStudios/Audio/Amplitude/Sound/SoundCollection.h>

#include <Core/EngineInternalState.h>

#include "RealChannel.h"

#include "sound_collection_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    static const unsigned int kLoopForever = ATOMIX_LOOP;
    static const unsigned int kPlayOnce = ATOMIX_PLAY;
    static const unsigned int kInvalidChannelId = 0;

    RealChannel::RealChannel()
        : _channelId(kInvalidChannelId)
        , _stream(false)
        , _loop(false)
        , _pan(0.0f)
        , _gain(1.0f)
    {}

    void RealChannel::Initialize(int i)
    {
        _channelId = i;
        _mixer = static_cast<atomix_mixer*>(Engine::GetInstance()->GetState()->mixer.GetUserData());
    }

    bool RealChannel::Valid() const
    {
        return _channelId != kInvalidChannelId && _mixer != nullptr;
    }

    bool RealChannel::Play(SoundCollection* collection, Sound* sound)
    {
        if (!sound || !collection)
            return false;

        if (!sound->GetUserData())
        {
            CallLogFunc("The sound was not loaded successfully.");
            return false;
        }

        AMPLITUDE_ASSERT(Valid());
        const SoundCollectionDefinition* def = collection->GetSoundCollectionDefinition();
        _loop = def->loop();
        _stream = def->stream();

        AmUInt32 loops = _loop ? kLoopForever : kPlayOnce;

        _channelId = atomixMixerPlay(_mixer, static_cast<atomix_sound*>(sound->GetUserData()), loops, def->gain(), 0.0f);
        _gain = def->gain();

        // Check if playing the sound was successful, and display the error if it was
        // not.
        bool success = _channelId != kInvalidChannelId;
        if (!success)
        {
            CallLogFunc("Could not play sound %s\n", sound->GetFilename().c_str());
        }

        return success;
    }

    bool RealChannel::Playing() const
    {
        AMPLITUDE_ASSERT(Valid());
        unsigned int state = atomixMixerGetState(_mixer, _channelId);
        return state == ATOMIX_PLAY || state == ATOMIX_LOOP;
    }

    bool RealChannel::Paused() const
    {
        AMPLITUDE_ASSERT(Valid());
        return atomixMixerGetState(_mixer, _channelId) == ATOMIX_HALT;
    }

    void RealChannel::SetGain(const float gain)
    {
        AMPLITUDE_ASSERT(Valid());

        atomixMixerSetGainPan(_mixer, _channelId, gain, _pan);
        _gain = gain;
    }

    float RealChannel::GetGain() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _gain;
    }

    void RealChannel::Halt()
    {
        AMPLITUDE_ASSERT(Valid());
        atomixMixerSetState(_mixer, _channelId, ATOMIX_STOP);
    }

    void RealChannel::Pause()
    {
        AMPLITUDE_ASSERT(Valid());
        atomixMixerSetState(_mixer, _channelId, ATOMIX_HALT);
        // Mix_Pause(_channelId);
    }

    void RealChannel::Resume()
    {
        AMPLITUDE_ASSERT(Valid());
        atomixMixerSetState(_mixer, _channelId, _loop ? ATOMIX_LOOP : ATOMIX_PLAY);
        // Mix_Resume(_channelId);
    }

    void RealChannel::FadeOut(int milliseconds)
    {
        AMPLITUDE_ASSERT(Valid());
        atomixMixerFade(_mixer, milliseconds);
        // Mix_FadeOutChannel(_channelId, milliseconds);
    }

    void RealChannel::SetPan(const hmm_vec2& pan)
    {
        AMPLITUDE_ASSERT(Valid());
        atomixMixerSetGainPan(_mixer, _channelId, _gain, pan.X);
        _pan = pan.X;
    }

} // namespace SparkyStudios::Audio::Amplitude