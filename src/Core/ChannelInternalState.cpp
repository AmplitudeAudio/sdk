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

#include "collection_definition_generated.h"
#include "sound_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    bool ChannelInternalState::IsStream() const
    {
        if (_sound != nullptr)
            return _sound->IsStream();

        return false;
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
        _realChannel = RealChannel(this);
        _channelState = ChannelState::Stopped;
        _collection = nullptr;
        _fader = nullptr;
        _targetFadeOutState = ChannelState::Stopped;
        _entity = Entity();
        _sound = nullptr;
        _userGain = 0.0f;
        _gain = 0.0f;
        _location = AM_Vec3(0, 0, 0);
    }

    void ChannelInternalState::SetCollection(Collection* collection)
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

    void ChannelInternalState::SetSound(Sound* sound)
    {
        if (_sound && _sound->GetBus())
        {
            bus_node.remove();
        }
        _sound = sound;
        if (_sound && _sound->GetBus())
        {
            _sound->GetBus()->GetPlayingSoundList().push_front(*this);
        }
    }

    Sound* ChannelInternalState::GetSound() const
    {
        return _sound;
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
        if (_collection != nullptr)
        {
            return PlayCollection();
        }

        if (_sound != nullptr)
        {
            return PlaySound();
        }

        CallLogFunc("[ERROR] Cannot play a channel. Neither a sound nor a collection was defined.\n");
        return false;
    }

    bool ChannelInternalState::Playing() const
    {
        return _channelState == ChannelState::Playing;
    }

    bool ChannelInternalState::Stopped() const
    {
        return _channelState == ChannelState::Stopped;
    }

    bool ChannelInternalState::Paused() const
    {
        return _channelState == ChannelState::Paused;
    }

    void ChannelInternalState::Halt()
    {
        if (_realChannel.Valid())
        {
            _realChannel.Halt();
        }

        _channelState = ChannelState::Stopped;
    }

    void ChannelInternalState::Pause()
    {
        if (_realChannel.Valid())
        {
            _realChannel.Pause();
        }

        _channelState = ChannelState::Paused;
    }

    void ChannelInternalState::Resume()
    {
        if (_realChannel.Valid())
        {
            _realChannel.Resume();
        }

        _channelState = ChannelState::Playing;
    }

    void ChannelInternalState::FadeIn(AmTime duration)
    {
        if (Playing())
            return;

        if (_realChannel.Valid())
        {
            _fader->Set(0.0f, _gain, duration / kAmSecond);
            _fader->Start(Engine::GetInstance()->GetTotalTime());

            _realChannel.SetGain(0.0f);
            _realChannel.Resume();
        }

        _channelState = ChannelState::FadingIn;
    }

    void ChannelInternalState::FadeOut(AmTime duration, ChannelState targetState)
    {
        if (Stopped() || Paused())
            return;

        if (_realChannel.Valid())
        {
            _fader->Set(_gain, 0.0f, duration / kAmSecond);
            _fader->Start(Engine::GetInstance()->GetTotalTime());
        }

        _channelState = ChannelState::FadingOut;
        _targetFadeOutState = targetState;
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
        if (_channelState == ChannelState::FadingOut || _channelState == ChannelState::FadingIn)
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

        _realChannel._parentChannelState = this;
        _realChannel.ClearPlayedSounds();

        if (Playing())
        {
            // Resume playing the audio.
            _realChannel.Play(_sound->CreateInstance(_collection));
        }
        else if (Paused())
        {
            // The audio needs to be playing to pause it.
            _realChannel.Play(_sound->CreateInstance(_collection));
            _realChannel.Pause();
        }
    }

    float ChannelInternalState::Priority() const
    {
        if (_collection != nullptr)
        {
            return GetGain() * _collection->GetCollectionDefinition()->priority();
        }

        if (_sound != nullptr)
        {
            return GetGain() * _sound->GetSoundDefinition()->priority();
        }

        AMPLITUDE_ASSERT(false); // Should never fall in this case...
        return 0.0f;
    }

    void ChannelInternalState::AdvanceFrame([[maybe_unused]] AmTime deltaTime)
    {
        // Update attached entity if any
        if (_entity.Valid())
        {
            _entity.Update();
        }

        // Update the fading in animation if necessary
        if (_channelState == ChannelState::FadingIn)
        {
            if (_fader != nullptr && _fader->GetState() == AM_FADER_STATE_ACTIVE)
            {
                const float gain = _fader->GetFromTime(Engine::GetInstance()->GetTotalTime());

                if (_realChannel.Valid())
                {
                    _realChannel.SetGain(gain);
                }

                if (_gain - gain <= kEpsilon)
                {
                    _fader->SetState(AM_FADER_STATE_STOPPED);
                    // Fading in transition complete. Now we mark the channel as playing.
                    _channelState = ChannelState::Playing;
                }
            }
            else
            {
                // No fader is defined, no fading occurs
                if (_realChannel.Valid())
                {
                    _realChannel.SetGain(_gain);
                }

                _channelState = ChannelState::Playing;
            }
        }

        // Update the fading out animation if necessary
        if (_channelState == ChannelState::FadingOut)
        {
            if (_fader != nullptr && _fader->GetState() == AM_FADER_STATE_ACTIVE)
            {
                _gain = _fader->GetFromTime(Engine::GetInstance()->GetTotalTime());

                if (_realChannel.Valid())
                {
                    _realChannel.SetGain(_gain);
                }

                if (_gain == 0.0f)
                {
                    _fader->SetState(AM_FADER_STATE_STOPPED);
                    // Fading out transition complete. Now we can halt or pause the channel.
                    if (_targetFadeOutState == ChannelState::Stopped)
                    {
                        Halt();
                    }
                    else if (_targetFadeOutState == ChannelState::Paused)
                    {
                        Pause();
                    }
                }
            }
            else
            {
                // No fader is defined, no fading occurs
                if (_realChannel.Valid())
                {
                    _realChannel.SetGain(0.0f);
                }

                if (_targetFadeOutState == ChannelState::Stopped)
                {
                    Halt();
                }
                else if (_targetFadeOutState == ChannelState::Paused)
                {
                    Pause();
                }
            }
        }
    }

    void ChannelInternalState::UpdateState()
    {
        switch (_channelState)
        {
        case ChannelState::Paused:
        case ChannelState::Stopped:
            break;
        case ChannelState::FadingIn:
        case ChannelState::Playing:
            if (_realChannel.Valid() && !_realChannel.Playing())
            {
                _channelState = ChannelState::Stopped;
            }
            break;
        case ChannelState::FadingOut:
            if (!_realChannel.Valid() || !_realChannel.Playing())
            {
                _channelState = ChannelState::Stopped;
            }
            break;
        default:
            AMPLITUDE_ASSERT(false);
        }
    }

    bool ChannelInternalState::PlayCollection()
    {
        AMPLITUDE_ASSERT(_collection != nullptr);

        const CollectionDefinition* definition = _collection->GetCollectionDefinition();

        _sound = _entity.Valid() ? _collection->SelectFromEntity(_entity, _realChannel._playedSounds)
                                 : _collection->SelectFromWorld(_realChannel._playedSounds);

        _fader = Fader::Create(static_cast<Fader::FADER_ALGORITHM>(definition->fader()));

        _channelState = ChannelState::Playing;
        return !_realChannel.Valid() || _realChannel.Play(_sound->CreateInstance(_collection));
    }

    bool ChannelInternalState::PlaySound()
    {
        AMPLITUDE_ASSERT(_sound != nullptr);

        const SoundDefinition* definition = _sound->GetSoundDefinition();

        _fader = Fader::Create(static_cast<Fader::FADER_ALGORITHM>(definition->fader()));

        _channelState = ChannelState::Playing;
        return !_realChannel.Valid() || _realChannel.Play(_sound->CreateInstance());
    }
} // namespace SparkyStudios::Audio::Amplitude
