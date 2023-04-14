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
#include <Core/EngineInternalState.h>
#include <Core/EntityInternalState.h>

#include <Utils/Utils.h>
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

        _channelState = ChannelPlaybackState::Stopped;
        _switchContainer = nullptr;
        _collection = nullptr;
        _sound = nullptr;
        _fader = nullptr;
        _targetFadeOutState = ChannelPlaybackState::Stopped;
        _entity = Entity();
        _userGain = 0.0f;
        _gain = 0.0f;
        _location = AM_V3(0, 0, 0);
    }

    void ChannelInternalState::SetSwitchContainer(SwitchContainer* switchContainer)
    {
        if (_switchContainer && _switchContainer->GetBus().Valid())
        {
            bus_node.remove();
        }
        _switchContainer = switchContainer;
        if (_switchContainer && _switchContainer->GetBus().Valid())
        {
            _switchContainer->GetBus().GetState()->GetPlayingSoundList().push_front(*this);
        }
    }

    void ChannelInternalState::SetCollection(Collection* collection)
    {
        if (_collection && _collection->GetBus().Valid())
        {
            bus_node.remove();
        }
        _collection = collection;
        if (_collection && _collection->GetBus().Valid())
        {
            _collection->GetBus().GetState()->GetPlayingSoundList().push_front(*this);
        }
    }

    void ChannelInternalState::SetSound(Sound* sound)
    {
        if (_sound && _sound->GetBus().Valid())
        {
            bus_node.remove();
        }
        _sound = sound;
        if (_sound && _sound->GetBus().Valid())
        {
            _sound->GetBus().GetState()->GetPlayingSoundList().push_front(*this);
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
        return _channelState == ChannelPlaybackState::Playing;
    }

    bool ChannelInternalState::Stopped() const
    {
        return _channelState == ChannelPlaybackState::Stopped;
    }

    bool ChannelInternalState::Paused() const
    {
        return _channelState == ChannelPlaybackState::Paused;
    }

    void ChannelInternalState::Halt()
    {
        if (Stopped())
            return;

        if (Valid())
        {
            _realChannel.Halt();
        }

        _channelState = ChannelPlaybackState::Stopped;

        if (_entity.Valid() && _collection != nullptr)
        {
            _collection->ResetEntityScopeScheduler(_entity);
        }
    }

    void ChannelInternalState::Pause()
    {
        if (Paused())
            return;

        if (Valid())
        {
            _realChannel.Pause();
        }

        _channelState = ChannelPlaybackState::Paused;
    }

    void ChannelInternalState::Resume()
    {
        if (Playing())
            return;

        if (Valid())
        {
            _realChannel.Resume();
        }

        _channelState = ChannelPlaybackState::Playing;
    }

    void ChannelInternalState::FadeIn(AmTime duration)
    {
        if (Playing() || _channelState == ChannelPlaybackState::FadingIn)
            return;

        if (Valid())
        {
            _fader->Set(0.0f, _gain, duration / kAmSecond);
            _fader->Start(Engine::GetInstance()->GetTotalTime());

            _realChannel.SetGain(0.0f);
            _realChannel.Resume();
        }

        _channelState = ChannelPlaybackState::FadingIn;
    }

    void ChannelInternalState::FadeOut(AmTime duration, ChannelPlaybackState targetState)
    {
        if (Stopped() || Paused() || _channelState == ChannelPlaybackState::FadingOut)
            return;

        if (Valid())
        {
            // If the sound is muted, no need to fade out
            if (_gain == 0.0f)
                return Halt();

            _realChannel.SetGain(_gain);

            _fader->Set(_gain, 0.0f, duration / kAmSecond);
            _fader->Start(Engine::GetInstance()->GetTotalTime());
        }

        _channelState = ChannelPlaybackState::FadingOut;
        _targetFadeOutState = targetState;
    }

    void ChannelInternalState::SetPan(const AmVec2& pan)
    {
        _pan = pan;

        if (Valid())
        {
            _realChannel.SetPan(pan);
        }
    }

    void ChannelInternalState::SetGain(const float gain)
    {
        if (_channelState == ChannelPlaybackState::FadingOut || _channelState == ChannelPlaybackState::FadingIn ||
            _channelState == ChannelPlaybackState::SwitchingState)
            // Do not update gain when fading...
            return;

        _gain = gain;

        if (Valid())
        {
            _realChannel.SetGain(gain);
        }
    }

    void ChannelInternalState::SetPitch(AmReal32 pitch)
    {
        _pitch = pitch;

        if (Valid())
        {
            _realChannel.SetPitch(pitch);
        }
    }

    AmReal32 ChannelInternalState::GetPitch() const
    {
        return _pitch;
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
        // Skip paused and stopped channels
        if (_channelState == ChannelPlaybackState::Paused || _channelState == ChannelPlaybackState::Stopped)
            return;

        // Update Doppler factors
        if (_entity.Valid())
        {
            for (auto&& listener : amEngine->GetState()->listener_list)
            {
                if (listener.GetId() == kAmInvalidObjectId)
                    continue;

                _dopplerFactors[listener.GetId()] = ComputeDopplerFactor(
                    _entity.GetLocation() - listener.GetLocation(), _entity.GetVelocity(), listener.GetVelocity(),
                    amEngine->GetSoundSpeed(), amEngine->GetDopplerFactor());
            }
        }

        // Update sounds if playing a switch container
        // TODO: This part should probably be optimized
        if (_switchContainer != nullptr && _channelState != ChannelPlaybackState::FadingIn &&
            _channelState != ChannelPlaybackState::FadingOut)
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

                _channelState = ChannelPlaybackState::SwitchingState;
            }

            if (_channelState == ChannelPlaybackState::SwitchingState)
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
                    for (auto&& _activeSound : _realChannel._activeSounds)
                    {
                        if (_activeSound.second->GetSettings().m_id == item.m_id)
                        {
                            layer = _activeSound.first;
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
                    _channelState = ChannelPlaybackState::Playing;
                    _previousSwitchContainerStateId = _playingSwitchContainerStateId;
                }
            }
        }

        // Update the fading in animation if necessary
        if (_channelState == ChannelPlaybackState::FadingIn)
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
                    _channelState = ChannelPlaybackState::Playing;
                }
            }
            else
            {
                // No fader is defined, no fading occurs
                if (IsReal())
                {
                    _realChannel.SetGain(_gain);
                }

                _channelState = ChannelPlaybackState::Playing;
            }
        }

        // Update the fading out animation if necessary
        if (_channelState == ChannelPlaybackState::FadingOut)
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
                    if (_targetFadeOutState == ChannelPlaybackState::Stopped)
                    {
                        Halt();
                    }
                    else if (_targetFadeOutState == ChannelPlaybackState::Paused)
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

                if (_targetFadeOutState == ChannelPlaybackState::Stopped)
                {
                    Halt();
                }
                else if (_targetFadeOutState == ChannelPlaybackState::Paused)
                {
                    Pause();
                }
            }
        }
    }

    void ChannelInternalState::SetObstruction(AmReal32 obstruction)
    {
        _realChannel.SetObstruction(obstruction);
    }

    void ChannelInternalState::SetOcclusion(AmReal32 occlusion)
    {
        _realChannel.SetOcclusion(occlusion);
    }

    AmReal32 ChannelInternalState::GetDopplerFactor(AmListenerID listener) const
    {
        return _dopplerFactors.find(listener) != _dopplerFactors.end() ? _dopplerFactors.at(listener) : 1.0f;
    }

    void ChannelInternalState::UpdateState()
    {
        switch (_channelState)
        {
        case ChannelPlaybackState::SwitchingState:
        case ChannelPlaybackState::Paused:
        case ChannelPlaybackState::Stopped:
            break;
        case ChannelPlaybackState::FadingIn:
        case ChannelPlaybackState::Playing:
            if (IsReal() && !_realChannel.Playing())
            {
                _channelState = ChannelPlaybackState::Stopped;
            }
            break;
        case ChannelPlaybackState::FadingOut:
            if (!IsReal() || !_realChannel.Playing())
            {
                _channelState = ChannelPlaybackState::Stopped;
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
        _channelState = ChannelPlaybackState::Playing;

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

        _channelState = ChannelPlaybackState::Playing;
        return !IsReal() || _realChannel.Play(sound->CreateInstance(_collection));
    }

    bool ChannelInternalState::PlaySound()
    {
        AMPLITUDE_ASSERT(_sound != nullptr);

        const SoundDefinition* definition = _sound->GetSoundDefinition();

        _fader = Fader::Create(static_cast<Fader::FADER_ALGORITHM>(definition->fader()));

        _channelState = ChannelPlaybackState::Playing;
        return !IsReal() || _realChannel.Play(_sound->CreateInstance());
    }
} // namespace SparkyStudios::Audio::Amplitude
