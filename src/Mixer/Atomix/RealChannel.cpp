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

#include <SparkyStudios/Audio/Amplitude/Core/Log.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Collection.h>

#include <Core/ChannelInternalState.h>
#include <Core/EngineInternalState.h>

#include "RealChannel.h"

#include "collection_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    static constexpr unsigned int kInvalidChannelId = 0;

    RealChannel::RealChannel()
        : RealChannel(nullptr)
    {}

    RealChannel::RealChannel(ChannelInternalState* parent)
        : _channelId(kInvalidChannelId)
        , _stream(false)
        , _loop(false)
        , _pan(0.0f)
        , _gain(1.0f)
        , _mixer(nullptr)
        , _activeSound(nullptr)
        , _parentChannelState(parent)
    {}

    void RealChannel::Initialize(int i)
    {
        _channelId = i;
        _mixer = static_cast<atomix_mixer*>(Engine::GetInstance()->GetState()->mixer.GetUserData());
    }

    void RealChannel::MarkAsPlayed(const Sound* sound)
    {
        _playedSounds.push_back(sound->GetId());
    }

    bool RealChannel::AllSoundsHasPlayed() const
    {
        bool result = true;
        for (auto&& sound : _parentChannelState->GetCollection()->GetAudioSamples())
        {
            if (auto foundIt = std::find(_playedSounds.begin(), _playedSounds.end(), sound); foundIt != _playedSounds.end())
                continue;

            result = false;
            break;
        }
        return result;
    }

    void RealChannel::ClearPlayedSounds()
    {
        _playedSounds.clear();
    }

    bool RealChannel::Valid() const
    {
        return _channelId != kInvalidChannelId && _mixer != nullptr && _parentChannelState != nullptr;
    }

    bool RealChannel::Play(SoundInstance* sound)
    {
        AMPLITUDE_ASSERT(sound != nullptr);

        _activeSound = sound;
        _activeSound->SetChannel(this);
        _activeSound->Load();

        if (!_activeSound->GetUserData())
        {
            _channelId = kInvalidChannelId;
            CallLogFunc("[ERROR] The sound was not loaded successfully.");
            return false;
        }

        _loop = _activeSound->GetSound()->IsLoop();
        _stream = _activeSound->GetSound()->IsStream();

        const AmUInt8 loops = _loop ? ATOMIX_LOOP : ATOMIX_PLAY;

        _channelId = atomixMixerPlay(_mixer, static_cast<atomix_sound*>(_activeSound->GetUserData()), loops, _gain, _pan);

        // Check if playing the sound was successful, and display the error if it was not.
        const bool success = _channelId != kInvalidChannelId;
        if (!success)
        {
            _channelId = kInvalidChannelId;
            CallLogFunc("[ERROR] Could not play sound " AM_OS_CHAR_FMT "\n", _activeSound->GetSound()->GetFilename());
        }

        return success;
    }

    bool RealChannel::Playing() const
    {
        AMPLITUDE_ASSERT(Valid());
        const AmUInt32 state = atomixMixerGetState(_mixer, _channelId);

        if (const auto* collection = _parentChannelState->GetCollection(); collection == nullptr)
        {
            return !_loop && state == ATOMIX_PLAY  || _loop && state == ATOMIX_LOOP;
        }
        else
        {
            const CollectionPlayMode mode = collection->GetCollectionDefinition()->play_mode();

            return mode == CollectionPlayMode_PlayOne && !_loop ? state == ATOMIX_PLAY
                : mode == CollectionPlayMode_PlayOne && _loop   ? state == ATOMIX_LOOP
                                                                : _channelId != kInvalidChannelId;
        }
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
    }

    void RealChannel::Resume()
    {
        AMPLITUDE_ASSERT(Valid());
        atomixMixerSetState(_mixer, _channelId, _loop ? ATOMIX_LOOP : ATOMIX_PLAY);
    }

    void RealChannel::SetPan(const hmm_vec2& pan)
    {
        AMPLITUDE_ASSERT(Valid());
        atomixMixerSetGainPan(_mixer, _channelId, _gain, pan.X);
        _pan = pan.X;
    }
} // namespace SparkyStudios::Audio::Amplitude
