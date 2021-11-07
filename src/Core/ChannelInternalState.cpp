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
#include "switch_container_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
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
        _realChannel._channelLayersId.clear();
        _realChannel._activeSounds.clear();
        _realChannel._playedSounds.clear();
        _realChannel._stream.clear();
        _realChannel._loop.clear();
        _realChannel._gain.clear();

        _channelState = ChannelState::Stopped;
        _switchContainer = nullptr;
        _collection = nullptr;
        _sound = nullptr;
        _fader = nullptr;
        _targetFadeOutState = ChannelState::Stopped;
        _entity = Entity();
        _userGain = 0.0f;
        _gain = 0.0f;
        _location = AM_Vec3(0, 0, 0);
    }

    void ChannelInternalState::SetSwitchContainer(SwitchContainer* switchContainer)
    {
        if (_switchContainer && _switchContainer->GetBus())
        {
            bus_node.remove();
        }
        _switchContainer = switchContainer;
        if (_switchContainer && _switchContainer->GetBus())
        {
            _switchContainer->GetBus()->GetPlayingSoundList().push_front(*this);
        }
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
        if (_switchContainer != nullptr)
        {
            return PlaySwitchContainer();
        }

        if (_collection != nullptr)
        {
            return PlayCollection();
        }

        if (_sound != nullptr)
        {
            return PlaySound();
        }

        CallLogFunc("[ERROR] Cannot play a channel. Neither a sound, a collection, nor a switch container was defined.\n");
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
        if (Valid())
        {
            _realChannel.Halt();
        }

        _channelState = ChannelState::Stopped;

        if (_entity.Valid() && _collection != nullptr)
        {
            _collection->ResetEntityScopeScheduler(_entity);
        }
    }

    void ChannelInternalState::Pause()
    {
        if (Valid())
        {
            _realChannel.Pause();
        }

        _channelState = ChannelState::Paused;
    }

    void ChannelInternalState::Resume()
    {
        if (Valid())
        {
            _realChannel.Resume();
        }

        _channelState = ChannelState::Playing;
    }

    void ChannelInternalState::FadeIn(AmTime duration)
    {
        if (Playing())
            return;

        if (Valid())
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

        if (Valid())
        {
            _realChannel.SetGain(_gain);

            _fader->Set(_gain, 0.0f, duration / kAmSecond);
            _fader->Start(Engine::GetInstance()->GetTotalTime());
        }

        _channelState = ChannelState::FadingOut;
        _targetFadeOutState = targetState;
    }

    void ChannelInternalState::SetPan(const hmm_vec2& pan)
    {
        _pan = pan;
        if (Valid())
        {
            _realChannel.SetPan(pan);
        }
    }

    void ChannelInternalState::SetGain(const float gain)
    {
        if (_channelState == ChannelState::FadingOut || _channelState == ChannelState::FadingIn ||
            _channelState == ChannelState::SwitchingState)
            // Do not update gain when fading...
            return;

        _gain = gain;
        if (Valid())
        {
            _realChannel.SetGain(gain);
        }
    }

    void ChannelInternalState::Devirtualize(ChannelInternalState* other)
    {
        AMPLITUDE_ASSERT(!_realChannel.Valid());
        AMPLITUDE_ASSERT(other->_realChannel.Valid());

        other->_realChannel.Pause();

        // Transfer the real channel id to this channel.
        std::swap(_realChannel._channelId, other->_realChannel._channelId);

        if (Playing())
        {
            // Resume playing the audio.
            if (!_realChannel._channelLayersId.empty())
            {
                Play();
            }
            else
            {
                _realChannel.Resume();
            }
        }
        else if (Paused())
        {
            Resume();
        }
    }

    float ChannelInternalState::Priority() const
    {
        if (_switchContainer != nullptr)
        {
            return GetGain() * _switchContainer->GetPriority().GetValue();
        }

        if (_collection != nullptr)
        {
            return GetGain() * _collection->GetPriority().GetValue();
        }

        if (_sound != nullptr)
        {
            return GetGain() * _sound->GetPriority().GetValue();
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

        if (_channelState == ChannelState::Paused || _channelState == ChannelState::Stopped)
            return;

        // Update sounds if playing a switch container
        // TODO: This part should probably be optimized
        if (_switchContainer != nullptr && _channelState != ChannelState::FadingIn && _channelState != ChannelState::FadingOut)
        {
            const SwitchContainerDefinition* definition = _switchContainer->GetSwitchContainerDefinition();
            if (_switch->GetState().m_id != kAmInvalidObjectId && _switch->GetState().m_id != _playingSwitchContainerStateId &&
                definition->update_behavior() == SwitchContainerUpdateBehavior_UpdateOnChange)
            {
                const std::vector<SwitchContainerItem>& previousItems = _switchContainer->GetSoundObjects(_playingSwitchContainerStateId);
                const std::vector<SwitchContainerItem>& nextItems = _switchContainer->GetSoundObjects(_switch->GetState().m_id);

                for (const auto& item : previousItems)
                {
                    bool shouldSkip = false;
                    for (const auto& next : nextItems)
                    {
                        if (next.m_id == item.m_id)
                        {
                            shouldSkip = item.m_continueBetweenStates;
                        }
                    }

                    if (shouldSkip)
                    {
                        continue;
                    }

                    Fader* out = _switchContainer->GetFaderOut(item.m_id);
                    out->Set(_gain, 0.0f);
                    out->Start(Engine::GetInstance()->GetTotalTime());
                }

                for (const auto& item : nextItems)
                {
                    bool shouldSkip = false;
                    for (const auto& previous : previousItems)
                    {
                        if (previous.m_id == item.m_id)
                        {
                            shouldSkip = item.m_continueBetweenStates;
                        }
                    }

                    if (shouldSkip)
                    {
                        continue;
                    }

                    Fader* in = _switchContainer->GetFaderIn(item.m_id);
                    in->Set(0.0f, _gain);
                    in->Start(Engine::GetInstance()->GetTotalTime());
                }

                _previousSwitchContainerStateId = _playingSwitchContainerStateId;
                PlaySwitchContainerStateUpdate(previousItems, nextItems);
                _playingSwitchContainerStateId = _switch->GetState().m_id;

                _channelState = ChannelState::SwitchingState;
            }

            if (_channelState == ChannelState::SwitchingState)
            {
                const std::vector<SwitchContainerItem>& previousItems = _switchContainer->GetSoundObjects(_previousSwitchContainerStateId);
                const std::vector<SwitchContainerItem>& nextItems = _switchContainer->GetSoundObjects(_playingSwitchContainerStateId);

                bool isAtLeastOneFadeInRunning = false;
                bool isAtLeastOneFadeOutRunning = false;

                for (const auto& item : previousItems)
                {
                    bool shouldSkip = false;
                    for (const auto& next : nextItems)
                    {
                        if (next.m_id == item.m_id)
                        {
                            shouldSkip = item.m_continueBetweenStates;
                        }
                    }

                    if (shouldSkip)
                    {
                        continue;
                    }

                    AmUInt32 layer = 0;
                    for (auto it = _realChannel._activeSounds.begin(); it != _realChannel._activeSounds.end(); ++it)
                    {
                        if (it->second->GetSettings().m_id == item.m_id)
                        {
                            layer = it->first;
                            break;
                        }
                    }

                    if (layer == 0)
                    {
                        continue;
                    }

                    Fader* out = _switchContainer->GetFaderOut(item.m_id);
                    if (out->GetState() == AM_FADER_STATE_STOPPED)
                    {
                        continue;
                    }

                    const float gain = out->GetFromTime(Engine::GetInstance()->GetTotalTime());
                    isAtLeastOneFadeOutRunning = true;

                    if (IsReal())
                    {
                        _realChannel.SetGain(gain, layer);
                    }

                    if (gain == 0.0f)
                    {
                        out->SetState(AM_FADER_STATE_STOPPED);
                        // Fading in transition complete. Now we can destroy the channel layer
                        _realChannel.Destroy(layer);
                    }
                }

                for (const auto& item : nextItems)
                {
                    bool shouldSkip = false;
                    for (const auto& previous : previousItems)
                    {
                        if (previous.m_id == item.m_id)
                        {
                            shouldSkip = item.m_continueBetweenStates;
                        }
                    }

                    if (shouldSkip)
                    {
                        continue;
                    }

                    AmUInt32 layer = 0;
                    for (auto it = _realChannel._activeSounds.rbegin(); it != _realChannel._activeSounds.rend(); ++it)
                    {
                        if (it->second->GetSettings().m_id == item.m_id)
                        {
                            layer = it->first;
                            break;
                        }
                    }

                    if (layer == 0)
                    {
                        continue;
                    }

                    Fader* in = _switchContainer->GetFaderIn(item.m_id);
                    if (in->GetState() == AM_FADER_STATE_STOPPED)
                    {
                        continue;
                    }

                    const float gain = in->GetFromTime(Engine::GetInstance()->GetTotalTime());
                    isAtLeastOneFadeInRunning = true;

                    if (IsReal())
                    {
                        _realChannel.SetGain(gain, layer);
                    }

                    if (_gain - gain <= kEpsilon)
                    {
                        in->SetState(AM_FADER_STATE_STOPPED);
                    }
                }

                if (!isAtLeastOneFadeInRunning && !isAtLeastOneFadeOutRunning)
                {
                    _channelState = ChannelState::Playing;
                    _previousSwitchContainerStateId = _playingSwitchContainerStateId;
                }
            }
        }

        // Update the fading in animation if necessary
        if (_channelState == ChannelState::FadingIn)
        {
            if (_fader != nullptr && _fader->GetState() == AM_FADER_STATE_ACTIVE)
            {
                const float gain = _fader->GetFromTime(Engine::GetInstance()->GetTotalTime());

                if (IsReal())
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
                if (IsReal())
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

                if (IsReal())
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
                if (IsReal())
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
        case ChannelState::SwitchingState:
        case ChannelState::Paused:
        case ChannelState::Stopped:
            break;
        case ChannelState::FadingIn:
        case ChannelState::Playing:
            if (IsReal() && !_realChannel.Playing())
            {
                _channelState = ChannelState::Stopped;
            }
            break;
        case ChannelState::FadingOut:
            if (!IsReal() || !_realChannel.Playing())
            {
                _channelState = ChannelState::Stopped;
            }
            break;
        default:
            AMPLITUDE_ASSERT(false);
        }
    }

    bool ChannelInternalState::PlaySwitchContainerStateUpdate(
        const std::vector<SwitchContainerItem>& previous, const std::vector<SwitchContainerItem>& next)
    {
        Engine* engine = Engine::GetInstance();
        const SwitchContainerDefinition* definition = _switchContainer->GetSwitchContainerDefinition();

        std::vector<SoundInstance*> instances;
        for (const auto& item : next)
        {
            bool shouldSkip = false;
            for (const auto& prev : previous)
            {
                if (prev.m_id == item.m_id)
                {
                    shouldSkip = item.m_continueBetweenStates;
                }
            }

            if (shouldSkip)
            {
                continue;
            }

            Sound* sound = nullptr;

            if (Collection* collection = engine->GetCollectionHandle(item.m_id); collection != nullptr)
            {
                sound = _entity.Valid() ? collection->SelectFromEntity(_entity, _realChannel._playedSounds)
                                        : collection->SelectFromWorld(_realChannel._playedSounds);
            }
            else
            {
                sound = engine->GetSoundHandle(item.m_id);
            }

            if (!sound)
            {
                CallLogFunc("[ERROR] Unable to find a sound object with id: %u", item.m_id);
                return false;
            }

            SoundInstanceSettings settings;
            settings.m_id = item.m_id;
            settings.m_kind = SoundKind::Switched;
            settings.m_busID = definition->bus();
            settings.m_attenuationID = definition->attenuation();
            settings.m_spatialization = definition->spatialization();
            settings.m_priority = _switchContainer->GetPriority();
            settings.m_gain = item.m_gain;
            settings.m_loop = sound->IsLoop();
            settings.m_loopCount = sound->GetSoundDefinition()->loop()->loop_count();

            instances.push_back(new SoundInstance(sound, settings, _switchContainer->GetEffect()));
        }

        return _realChannel.Play(instances);
    }

    bool ChannelInternalState::PlaySwitchContainer()
    {
        AMPLITUDE_ASSERT(_switchContainer != nullptr);

        Engine* engine = Engine::GetInstance();
        const SwitchContainerDefinition* definition = _switchContainer->GetSwitchContainerDefinition();

        _switch = _switchContainer->GetSwitch();
        _fader = Fader::Create(static_cast<Fader::FADER_ALGORITHM>(definition->fader()));
        _channelState = ChannelState::Playing;

        if (IsReal())
        {
            const SwitchState& state = _switch->GetState();
            _playingSwitchContainerStateId = state.m_id != kAmInvalidObjectId ? state.m_id : definition->default_switch_state();
            const std::vector<SwitchContainerItem>& items = _switchContainer->GetSoundObjects(_playingSwitchContainerStateId);

            return PlaySwitchContainerStateUpdate({}, items);
        }

        return true;
    }

    bool ChannelInternalState::PlayCollection()
    {
        AMPLITUDE_ASSERT(_collection != nullptr);

        const CollectionDefinition* definition = _collection->GetCollectionDefinition();

        Sound* sound = _entity.Valid() ? _collection->SelectFromEntity(_entity, _realChannel._playedSounds)
                                       : _collection->SelectFromWorld(_realChannel._playedSounds);

        _fader = Fader::Create(static_cast<Fader::FADER_ALGORITHM>(definition->fader()));

        _channelState = ChannelState::Playing;
        return IsReal() ? _realChannel.Play(sound->CreateInstance(_collection)) : true;
    }

    bool ChannelInternalState::PlaySound()
    {
        AMPLITUDE_ASSERT(_sound != nullptr);

        const SoundDefinition* definition = _sound->GetSoundDefinition();

        _fader = Fader::Create(static_cast<Fader::FADER_ALGORITHM>(definition->fader()));

        _channelState = ChannelState::Playing;
        return IsReal() ? _realChannel.Play(_sound->CreateInstance()) : true;
    }
} // namespace SparkyStudios::Audio::Amplitude
