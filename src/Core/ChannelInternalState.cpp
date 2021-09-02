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

#ifdef _WIN32
#if !defined(_USE_MATH_DEFINES)
#define _USE_MATH_DEFINES
#endif // !defined(_USE_MATH_DEFINES)
#endif // _WIN32

#include <algorithm>
#include <cmath>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/BusInternalState.h>
#include <Core/ChannelInternalState.h>
#include <Core/EntityInternalState.h>
#include <Utils/intrusive_list.h>

#include "sound_collection_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    bool ChannelInternalState::IsStream() const
    {
        return _collection->GetSoundCollectionDefinition()->stream() != 0;
    }

    // Removes this channel state from all lists.
    void ChannelInternalState::Remove()
    {
        free_node.remove();
        priority_node.remove();
        bus_node.remove();
        entity_node.remove();
    }

    void ChannelInternalState::Reset()
    {
        _realChannel = RealChannel();
        _channelState = ChannelStateStopped;
        _collection = nullptr;
        _fader = nullptr;
        _entity = Entity();
        _sound = nullptr;
        _userGain = 0.0f;
        _gain = 0.0f;
        _location = AM_Vec3(0, 0, 0);
    }

    void ChannelInternalState::SetSoundCollection(SoundCollection* collection)
    {
        if (_collection && _collection->GetBus())
        {
            bus_node.remove();
        }
        _collection = collection;
        if (_collection && _collection->GetBus())
        {
            _collection->GetBus()->GetPlayingSoundList().push_front(*this);
        }
    }

    void ChannelInternalState::SetEntity(const Entity& entity)
    {
        if (_entity.Valid())
        {
            entity_node.remove();
        }
        _entity = entity;
        if (_entity.Valid())
        {
            _entity.GetState()->GetPlayingSoundList().push_front(*this);
        }
    }

    bool ChannelInternalState::Play()
    {
        if (_collection == nullptr)
        {
            CallLogFunc("[Debug] Cannot play a channel. No sound collection defined.\n");
            return false;
        }

        const SoundCollectionDefinition* definition = _collection->GetSoundCollectionDefinition();

        std::vector<const Sound*> toSkip = _realChannel.Valid() ? _realChannel._playedSounds : std::vector<const Sound*>();
        _sound = _entity.Valid() ? _collection->SelectFromEntity(_entity, toSkip) : _collection->SelectFromWorld(toSkip);

        _fader = Fader::Create(static_cast<Fader::FADER_ALGORITHM>(definition->fader()));

        _channelState = ChannelStatePlaying;
        return !_realChannel.Valid() || _realChannel.Play(_collection, _sound);
    }

    bool ChannelInternalState::Playing() const
    {
        return _channelState == ChannelStatePlaying;
    }

    bool ChannelInternalState::Stopped() const
    {
        return _channelState == ChannelStateStopped;
    }

    bool ChannelInternalState::Paused() const
    {
        return _channelState == ChannelStatePaused;
    }

    void ChannelInternalState::Halt()
    {
        if (_realChannel.Valid())
        {
            _realChannel.Halt();
        }

        _channelState = ChannelStateStopped;
    }

    void ChannelInternalState::Pause()
    {
        if (_realChannel.Valid())
        {
            _realChannel.Pause();
        }

        _channelState = ChannelStatePaused;
    }

    void ChannelInternalState::Resume()
    {
        if (_realChannel.Valid())
        {
            _realChannel.Resume();
        }

        _channelState = ChannelStatePlaying;
    }

    void ChannelInternalState::FadeOut(AmTime duration)
    {
        if (_realChannel.Valid())
        {
            _fader->Set(_gain, 0.0f, duration / kAmSecond);
            _fader->Start(Engine::GetInstance()->GetTotalTime());
        }

        _channelState = ChannelStateFadingOut;
    }

    void ChannelInternalState::SetPan(const hmm_vec2& pan)
    {
        _pan = pan;
        if (_realChannel.Valid())
        {
            _realChannel.SetPan(pan);
        }
    }

    void ChannelInternalState::SetGain(const float gain)
    {
        if (_channelState == ChannelStateFadingOut)
            // Do not update gain when fading...
            return;

        _gain = gain;
        if (_realChannel.Valid())
        {
            _realChannel.SetGain(gain);
        }
    }

    void ChannelInternalState::Devirtualize(ChannelInternalState* other)
    {
        AMPLITUDE_ASSERT(!_realChannel.Valid());
        AMPLITUDE_ASSERT(other->_realChannel.Valid());

        // Transfer the real channel id to this channel.
        std::swap(_realChannel, other->_realChannel);

        if (Playing())
        {
            // Resume playing the audio.
            _realChannel.Play(_collection, _sound);
        }
        else if (Paused())
        {
            // The audio needs to be playing to pause it.
            _realChannel.Play(_collection, _sound);
            _realChannel.Pause();
        }
    }

    float ChannelInternalState::Priority() const
    {
        AMPLITUDE_ASSERT(_collection);
        return GetGain() * _collection->GetSoundCollectionDefinition()->priority();
    }

    void ChannelInternalState::AdvanceFrame(AmTime delta_time)
    {
        // Update attached entity if any
        if (_entity.Valid())
        {
            _entity.Update();
        }

        // Update the fading out animation if necessary
        if (_channelState == ChannelStateFadingOut && _fader != nullptr)
        {
            _gain = _fader->GetFromTime(Engine::GetInstance()->GetTotalTime());

            if (_realChannel.Valid())
            {
                _realChannel.SetGain(_gain);
            }

            if (_gain == 0.0f)
            {
                // Fading out transition complete. Now we can halt the channel.
                Halt();
            }
        }
    }

    void ChannelInternalState::UpdateState()
    {
        switch (_channelState)
        {
        case ChannelStatePaused:
        case ChannelStateStopped:
            break;
        case ChannelStatePlaying:
            if (_realChannel.Valid() && !_realChannel.Playing())
            {
                _channelState = ChannelStateStopped;
            }
            break;
        case ChannelStateFadingOut:
            if (!_realChannel.Valid() || !_realChannel.Playing())
            {
                _channelState = ChannelStateStopped;
            }
            break;
        default:
            AMPLITUDE_ASSERT(false);
        }
    }
} // namespace SparkyStudios::Audio::Amplitude
