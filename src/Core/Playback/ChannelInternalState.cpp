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

#include <algorithm>
#include <atomic>
#include <ranges>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/Engine.h>
#include <Core/EntityInternalState.h>
#include <Core/Playback/BusInternalState.h>
#include <Core/Playback/ChannelInternalState.h>

#include <Utils/intrusive_list.h>
#include <Utils/Utils.h>

#include "collection_definition_generated.h"
#include "sound_definition_generated.h"
#include "switch_container_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    static std::atomic<AmUInt64> gChannelInstanceGeneration{ 1 };

    // Removes this channel state from all lists.
    void ChannelInternalState::Remove()
    {
        free_node.remove();
        priority_node.remove();
        bus_node.remove();
        entity_node.remove();
        room_node.remove();
        listener_node.remove();
    }

    void ChannelInternalState::Reset()
    {
        _realChannel._layers.clear();
        _realChannel._playedSounds.clear();

        _dopplerFactors.clear();
        _channelState = eChannelPlaybackState_Stopped;
        _switchContainer = nullptr;
        _collection = nullptr;
        _sound = nullptr;
        _fader = nullptr;
        _faderName = "";
        _targetFadeOutState = eChannelPlaybackState_Stopped;
        _fadeInEndTime = 0.0;
        _stopEventPending = false;
        _stopFired = false;
        _entity = Entity();
        _userGain = 0.0f;
        _gain = 0.0f;
        _realGain = 0.0f;
        _location = kVector3Zero;
        _channelStateId = 0;

        for (auto& listener : _eventsMap | std::views::values)
            listener = nullptr;

        _eventsMap.clear();

        ClearInstances();

        _instancingEnabled = false;
        _instancingMode = eChannelInstanceMode_Blended;
        _nextInstanceId = 1;

        _room = Room();
        _activeListener = Listener();
    }

    void ChannelInternalState::SetSwitchContainer(SwitchContainerImpl* switchContainer)
    {
        if (_switchContainer && _switchContainer->GetBus().Valid())
            bus_node.remove();

        if (_switchContainer != switchContainer)
        {
            _switchContainer = switchContainer;
            _priorityDirty = true;
        }

        if (_switchContainer && _switchContainer->GetBus().Valid())
            _switchContainer->GetBus().GetState()->GetPlayingSoundList().push_front(*this);
    }

    void ChannelInternalState::SetCollection(CollectionImpl* collection)
    {
        if (_collection && _collection->GetBus().Valid())
            bus_node.remove();

        if (_collection != collection)
        {
            _collection = collection;
            _priorityDirty = true;
        }

        if (_collection && _collection->GetBus().Valid())
            _collection->GetBus().GetState()->GetPlayingSoundList().push_front(*this);
    }

    void ChannelInternalState::SetSound(SoundImpl* sound)
    {
        if (_sound && _sound->GetBus().Valid())
            bus_node.remove();

        if (_sound != sound)
        {
            _sound = sound;
            _priorityDirty = true;
        }

        if (_sound && _sound->GetBus().Valid())
            _sound->GetBus().GetState()->GetPlayingSoundList().push_front(*this);
    }

    void ChannelInternalState::SetEntity(const Entity& entity)
    {
        if (entity.GetState() == _entity.GetState())
            return;

        if (_entity.Valid())
            entity_node.remove();

        _entity = entity;

        if (_entity.Valid())
            _entity.GetState()->GetPlayingSoundList().push_front(*this);
    }

    void ChannelInternalState::SetListener(const Listener& listener)
    {
        if (listener.GetState() == _activeListener.GetState())
            return;

        if (_activeListener.Valid())
            listener_node.remove();

        _activeListener = listener;

        if (_activeListener.Valid())
            _activeListener.GetState()->GetPlayingSoundList().push_front(*this);
    }

    void ChannelInternalState::SetRoom(const Room& room)
    {
        if (room.GetState() == _room.GetState())
            return;

        if (_room.Valid())
            room_node.remove();

        _room = room;

        if (_room.Valid())
            _room.GetState()->GetPlayingSoundList().push_front(*this);
    }

    bool ChannelInternalState::Play()
    {
        _stopFired = false;

        if (_switchContainer != nullptr)
            return PlaySwitchContainer();

        if (_collection != nullptr)
            return PlayCollection();

        if (_sound != nullptr)
            return PlaySound();

        amLogError("Cannot play a channel. Neither a sound, a collection, nor a switch container was defined.");
        return false;
    }

    bool ChannelInternalState::Playing() const
    {
        return _channelState == eChannelPlaybackState_Playing;
    }

    bool ChannelInternalState::Stopped() const
    {
        return _channelState == eChannelPlaybackState_Stopped;
    }

    bool ChannelInternalState::Paused() const
    {
        return _channelState == eChannelPlaybackState_Paused;
    }

    void ChannelInternalState::Halt()
    {
        if (Stopped())
            return;

        HaltInternal();

        if (_collection == nullptr)
            return;

        if (_entity.Valid())
            _collection->ResetEntityScopeScheduler(_entity);
        else
            _collection->ResetWorldScopeScheduler();
    }

    void ChannelInternalState::Pause()
    {
        if (Paused() || !Valid() || IsFadingOutToStopped())
            return;

        if (_realChannel.Pause())
        {
            _channelState = eChannelPlaybackState_Paused;
            TriggerOnNextFrame(eChannelEvent_Pause);
        }
    }

    void ChannelInternalState::Resume()
    {
        if (Playing() || !Valid() || IsFadingOutToStopped())
            return;

        if (_realChannel.Resume())
        {
            _channelState = eChannelPlaybackState_Playing;
            TriggerOnNextFrame(eChannelEvent_Resume);
        }
    }

    bool ChannelInternalState::SetPlaybackPosition(AmTime position)
    {
        if (!Valid())
            return false;

        if (_instancingEnabled && _instancingMode == eChannelInstanceMode_Separate)
        {
            amLogWarning(
                "Cannot seek channel " AM_ID_CHAR_FMT ". Separate instanced rendering uses divergent playback cursors.", _channelStateId);
            return false;
        }

        return _realChannel.Seek(position);
    }

    AmTime ChannelInternalState::GetPlaybackPosition() const
    {
        if (!Valid())
            return 0.0;

        if (_instancingEnabled && _instancingMode == eChannelInstanceMode_Separate)
        {
            amLogWarning(
                "Cannot query playback position for channel " AM_ID_CHAR_FMT
                ". Separate instanced rendering uses divergent playback cursors.",
                _channelStateId);
            return 0.0;
        }

        return _realChannel.GetPlaybackPosition();
    }

    void ChannelInternalState::FadeIn(AmTime duration)
    {
        if (Playing() || !Valid() || _channelState == eChannelPlaybackState_FadingIn || IsFadingOutToStopped())
            return;

        _realChannel.SetGain(_gain);
        _realGain = _gain;

        if (!_realChannel.ResumeWithFade(duration))
            return;

        _channelState = eChannelPlaybackState_FadingIn;
        _fadeInEndTime = amEngine->GetTotalTime() + AM_MAX(duration, kDeclickFade);
        TriggerOnNextFrame(eChannelEvent_Resume);
    }

    void ChannelInternalState::FadeOut(AmTime duration, eChannelPlaybackState targetState)
    {
        if (Stopped() || Paused())
            return;

        // A stop fade overriding a pause fade: layers may already be paused, and a paused voice finishes without
        // posting FadedOut{Stopped}. Checked before the guard below changes _channelState.
        const bool overridingPauseFade = _channelState == eChannelPlaybackState_FadingOut &&
            _targetFadeOutState == eChannelPlaybackState_Paused && targetState == eChannelPlaybackState_Stopped;

        // A stop fade in progress is never interrupted (by another stop fade or by a pause fade); a pause fade in
        // progress may be overridden by a stop fade, which lets the voice restart its fade from the current gain.
        if (_channelState == eChannelPlaybackState_FadingOut &&
            (_targetFadeOutState == eChannelPlaybackState_Stopped || targetState == eChannelPlaybackState_Paused))
            return;

        if (_realGain <= kEpsilon || !Valid())
        {
            if (targetState == eChannelPlaybackState_Stopped)
                return Halt();

            if (targetState == eChannelPlaybackState_Paused)
                return Pause();

            return;
        }

        const eVoiceCommandKind kind = targetState == eChannelPlaybackState_Stopped ? eVoiceCommandKind::Stop : eVoiceCommandKind::Pause;
        if (!_realChannel.FadeOut(duration, kind))
            return;

        if (overridingPauseFade)
            // Clear the stale paused flags so RealChannel::Playing() reads the voices' own state: the channel stays
            // FadingOut until every voice is forgotten, and SettleStopped() covers a paused voice that finishes without
            // FadedOut{Stopped}. The layers are not marked stopping: Playing() would skip them and settle the channel a
            // whole fade early.
            _realChannel.ClearAllLayersPaused();

        _channelState = eChannelPlaybackState_FadingOut;
        _targetFadeOutState = targetState;
        _stopEventPending = targetState == eChannelPlaybackState_Stopped;
    }

    void ChannelInternalState::SetGain(const AmReal32 gain)
    {
        if (_channelState == eChannelPlaybackState_FadingOut || _channelState == eChannelPlaybackState_FadingIn ||
            _channelState == eChannelPlaybackState_SwitchingState)
            // Do not update gain when fading...
            return;

        if (_gain != gain)
        {
            _gain = gain;
            _priorityDirty = true;
        }

        _realGain = gain;

        if (!Valid())
            return;

        _realChannel.SetGain(gain);
    }

    void ChannelInternalState::SetPitch(AmReal32 pitch)
    {
        _pitch = pitch;

        if (!Valid())
            return;

        _realChannel.SetPitch(pitch);
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
            if (!_realChannel._layers.empty())
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

    AmReal32 ChannelInternalState::Priority() const
    {
        if (_priorityDirty)
        {
            if (_switchContainer != nullptr)
            {
                _cachedPriority = GetGain() * _switchContainer->GetPriority().GetValue();
            }
            else if (_collection != nullptr)
            {
                _cachedPriority = GetGain() * _collection->GetPriority().GetValue();
            }
            else if (_sound != nullptr)
            {
                _cachedPriority = GetGain() * _sound->GetPriority().GetValue();
            }
            else
            {
                AMPLITUDE_ASSERT(false); // Should never fall in this case...
                return 0.0f;
            }

            _priorityDirty = false;
        }

        return _cachedPriority;
    }

    void ChannelInternalState::AdvanceFrame([[maybe_unused]] AmTime deltaTime)
    {
        // Skip paused and stopped channels
        if (_channelState == eChannelPlaybackState_Paused || _channelState == eChannelPlaybackState_Stopped)
            return;

        const AmTime currentTime = amEngine->GetTotalTime();

        // Update Doppler factors
        if (_entity.Valid())
        {
            for (auto&& listener : amEngine->GetState()->listener_list)
            {
                if (listener.GetId() == kAmInvalidObjectId)
                    continue;

                _dopplerFactors[listener.GetId()] = ComputeDopplerFactor(
                    Sub(_entity.GetLocation(), listener.GetLocation()), _entity.GetVelocity(), listener.GetVelocity(),
                    amEngine->GetSoundSpeed(), amEngine->GetDopplerFactor());
            }
        }

        // Update Room gains
        if (_room.Valid())
        {
            AmReal32 gain = 0.0f;

            if (const AmReal32 roomVolume = _room.GetVolume(); roomVolume >= kEpsilon)
            {
                const AmVector3& location = GetLocation();
                const AmVector3& closestPoint = _room.GetShape().GetClosestPoint(location);

                // Avoid division by zero by shifting the attenuation by 1.0f
                const AmReal32 distance = Length(Sub(location, closestPoint)) + 1.0f;

                gain = 1.0f / (distance * distance);
            }

            _roomGains[_room.GetId()] = _room.GetGain() * gain;
        }

        // Update sounds if playing a switch container
        if (_switchContainer != nullptr && _channelState != eChannelPlaybackState_FadingIn &&
            _channelState != eChannelPlaybackState_FadingOut)
        {
            const SwitchContainerDefinition* definition = _switchContainer->GetDefinition();
            const auto switchStateId = _switchContainer->GetSwitch()->GetState().m_id;

            if (switchStateId != kAmInvalidObjectId && switchStateId != _playingSwitchContainerStateId &&
                definition->update_behavior() == SwitchContainerUpdateBehavior_UpdateOnChange)
            {
                const std::vector<SwitchContainerItem>& previousItems = _switchContainer->GetSoundObjects(_playingSwitchContainerStateId);
                std::vector<SwitchContainerItem> nextItems = _switchContainer->GetSoundObjects(switchStateId);

                // Build a map for faster layer lookup, before the outgoing items' layers are faded out.
                std::unordered_map<AmObjectID, AmUInt32> soundToLayer;
                for (const auto& [layerId, layerData] : _realChannel._layers)
                    soundToLayer[layerData.soundInstance->GetSettings().m_id] = layerId;

                const bool isReal = IsReal();

                for (const auto& item : previousItems)
                {
                    if (item.m_continueBetweenStates)
                    {
                        auto it = std::find_if(
                            nextItems.begin(), nextItems.end(),
                            [id = item.m_id](const SwitchContainerItem& nextItem)
                            {
                                return nextItem.m_id == id;
                            });

                        if (it != nextItems.end())
                        {
                            nextItems.erase(it);
                            continue;
                        }
                    }

                    // Old layers finish on their own and are forgotten on their Finished event.
                    if (isReal)
                        if (const auto layerIt = soundToLayer.find(item.m_id); layerIt != soundToLayer.end())
                            _realChannel.FadeOutLayer(layerIt->second, _switchContainer->GetFaderOut(item.m_id)->GetDuration());
                }

                // The new voices start together with the largest fade-in duration among them.
                AmTime fadeIn = 0.0;
                for (const auto& item : nextItems)
                    fadeIn = AM_MAX(fadeIn, _switchContainer->GetFaderIn(item.m_id)->GetDuration());

                _previousSwitchContainerStateId = _playingSwitchContainerStateId;
                PlaySwitchContainerStateUpdate(previousItems, nextItems, fadeIn);
                _playingSwitchContainerStateId = switchStateId;

                _channelState = eChannelPlaybackState_Playing;
            }
        }

        // Fades render in the voice; the channel only needs to know when a fade-in is over.
        if (_channelState == eChannelPlaybackState_FadingIn && currentTime >= _fadeInEndTime)
            _channelState = eChannelPlaybackState_Playing;
    }

    AmObjectID ChannelInternalState::GetPlayingObjectId() const
    {
        if (_switchContainer)
            return _switchContainer->GetId();
        if (_collection)
            return _collection->GetId();
        if (_sound)
            return _sound->GetId();

        return kAmInvalidObjectId;
    }

    void ChannelInternalState::SetObstruction(AmReal32 obstruction)
    {
        if (!Valid())
            return;

        _realChannel.SetObstruction(obstruction);
    }

    void ChannelInternalState::SetOcclusion(AmReal32 occlusion)
    {
        if (!Valid())
            return;

        _realChannel.SetOcclusion(occlusion);
    }

    AmReal32 ChannelInternalState::GetDopplerFactor(AmListenerID listener) const
    {
        return _dopplerFactors.contains(listener) ? _dopplerFactors.at(listener) : 1.0f;
    }

    AmReal32 ChannelInternalState::GetRoomGain(AmRoomID room) const
    {
        return _roomGains.contains(room) ? _roomGains.at(room) : 0.0f;
    }

    void ChannelInternalState::On(const eChannelEvent event, ChannelEventCallback callback, void* userData)
    {
        if (!Valid())
            return;

        if (_eventsMap[event] == nullptr)
            _eventsMap[event] = ampoolshared(eMemoryPoolKind_Engine, ChannelEventListener);

        _eventsMap[event]->Add(callback, userData);
    }

    void ChannelInternalState::Trigger(eChannelEvent event)
    {
        if (!Valid())
            return;

        if (_eventsMap[event] == nullptr)
            _eventsMap[event] = ampoolshared(eMemoryPoolKind_Engine, ChannelEventListener);

        _eventsMap[event]->Call(this);
    }

    void ChannelInternalState::UpdateState()
    {
        switch (_channelState)
        {
        case eChannelPlaybackState_SwitchingState:
        case eChannelPlaybackState_Paused:
        case eChannelPlaybackState_Stopped:
        case eChannelPlaybackState_Pending:
            break;

        case eChannelPlaybackState_FadingOut:
            if (_targetFadeOutState == eChannelPlaybackState_Paused)
            {
                if (!Valid() || _realChannel._layers.empty())
                {
                    SettleStopped();
                }
                else if (_realChannel.Paused())
                {
                    _channelState = eChannelPlaybackState_Paused;
                    _realGain = 0.0f;

                    // Deferred: this runs inside EraseFinishedSounds' channel-list iteration, and a callback that
                    // calls Play() with a full pool could otherwise evict list->back() and disturb the iterator.
                    TriggerOnNextFrame(eChannelEvent_Pause);
                }

                break;
            }

            [[fallthrough]];
        case eChannelPlaybackState_FadingIn:
        case eChannelPlaybackState_Playing:
            if (!Valid() || !_realChannel.Playing())
            {
                SettleStopped();
            }
            break;
        default:
            AMPLITUDE_ASSERT(false);
        }
    }

    void ChannelInternalState::HaltInternal()
    {
        if (!Valid())
            return;

        if (_realChannel.Halt())
        {
            _channelState = eChannelPlaybackState_Stopped;

            // An immediate halt fires its own Stop right below; a stale fade-out event for a stop fade this halt
            // superseded must not fire a second one.
            _stopEventPending = false;
            TriggerStopOnce();
        }
    }

    const AmString& ChannelInternalState::GetFaderName() const
    {
        return _faderName;
    }

    bool ChannelInternalState::IsFadingOutToStopped() const
    {
        return _channelState == eChannelPlaybackState_FadingOut && _targetFadeOutState == eChannelPlaybackState_Stopped;
    }

    void ChannelInternalState::SettleStopped()
    {
        _channelState = eChannelPlaybackState_Stopped;

        // A layer whose voice was already paused finishes without FadedOut{Stopped}: the Stop event is still owed, fire
        // it once here.
        if (_stopEventPending)
        {
            _stopEventPending = false;
            TriggerStopOnce();
        }
    }

    void ChannelInternalState::TriggerStopOnce()
    {
        if (_stopFired)
            return;

        _stopFired = true;
        TriggerOnNextFrame(eChannelEvent_Stop);
    }

    void ChannelInternalState::OnVoiceFadedOut(AmUInt32 mixerLayerId, eVoiceFadeTarget target, AmUInt64 frame, AmUInt64 sourcePosition)
    {
        AM_UNUSED(frame);
        AM_UNUSED(sourcePosition);

        switch (target)
        {
        case eVoiceFadeTarget::Paused:
            if (_channelState == eChannelPlaybackState_FadingOut && _targetFadeOutState == eChannelPlaybackState_Paused)
                _realChannel.MarkLayerPaused(mixerLayerId);
            break;

        case eVoiceFadeTarget::Stopped:
            // The layer is forgotten on its Finished event; UpdateState() stops the channel when none is left.
            if (_stopEventPending)
            {
                _stopEventPending = false;
                TriggerStopOnce();
            }
            break;

        default:
            break;
        }
    }

    void ChannelInternalState::TriggerOnNextFrame(eChannelEvent event)
    {
        const AmUInt64 stateId = _channelStateId;
        amEngine->OnNextFrame(
            [this, stateId, event](AmTime)
            {
                // The channel may have been reused in between.
                if (_channelStateId == stateId)
                    Trigger(event);
            });
    }

    bool ChannelInternalState::PlaySwitchContainerStateUpdate(
        const std::vector<SwitchContainerItem>& previous, const std::vector<SwitchContainerItem>& next, AmTime fadeIn)
    {
        const SwitchContainerDefinition* definition = _switchContainer->GetDefinition();

        std::vector<SoundInstance*> instances;
        for (const auto& item : next)
        {
            bool shouldSkip = false;
            for (const auto& prev : previous)
                if (prev.m_id == item.m_id)
                    shouldSkip = item.m_continueBetweenStates;

            if (shouldSkip)
                continue;

            SoundImpl* sound;

            if (Collection* collection = amEngine->GetCollectionHandle(item.m_id); collection != nullptr)
            {
                sound = _entity.Valid() ? static_cast<SoundImpl*>(collection->SelectFromEntity(_entity, _realChannel._playedSounds))
                                        : static_cast<SoundImpl*>(collection->SelectFromWorld(_realChannel._playedSounds));
            }
            else
            {
                sound = static_cast<SoundImpl*>(amEngine->GetSoundHandle(item.m_id));
            }

            if (!sound)
            {
                amLogError("Unable to find a sound object with id: " AM_ID_CHAR_FMT, item.m_id);
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
            settings.m_nearFieldGain = sound->GetNearFieldGain();
            settings.m_pitch = item.m_pitch;
            settings.m_loop = sound->IsLoop();
            settings.m_loopCount = sound->GetDefinition()->loop()->loop_count();
            settings.m_effectID = definition->effect();

            instances.push_back(ampoolnew(
                eMemoryPoolKind_Amplimix, SoundInstance, sound, settings, static_cast<const EffectImpl*>(_switchContainer->GetEffect())));
        }

        RealChannelPlayOptions options;
        options.fadeIn = fadeIn;

        const bool success = _realChannel.Play(instances, options);
        if (!success)
            for (SoundInstance* instance : instances)
                SoundImpl::DestroyInstance(instance);

        return success;
    }

    bool ChannelInternalState::PlaySwitchContainer()
    {
        AMPLITUDE_ASSERT(_switchContainer != nullptr);

        const SwitchContainerDefinition* definition = _switchContainer->GetDefinition();

        _fader = nullptr;

        _faderName = definition->fader()->str();
        _fader = Fader::Construct(_faderName);

        _channelState = eChannelPlaybackState_Playing;

        if (IsReal())
        {
            const auto& [stateId, stateName] = _switchContainer->GetSwitch()->GetState();
            _playingSwitchContainerStateId = stateId != kAmInvalidObjectId ? stateId : definition->default_switch_state();
            const std::vector<SwitchContainerItem>& items = _switchContainer->GetSoundObjects(_playingSwitchContainerStateId);

            return PlaySwitchContainerStateUpdate({}, items);
        }

        return true;
    }

    bool ChannelInternalState::PlayCollection()
    {
        AMPLITUDE_ASSERT(_collection != nullptr);

        const CollectionDefinition* definition = _collection->GetDefinition();

        SoundImpl* sound = _entity.Valid() ? static_cast<SoundImpl*>(_collection->SelectFromEntity(_entity, _realChannel._playedSounds))
                                           : static_cast<SoundImpl*>(_collection->SelectFromWorld(_realChannel._playedSounds));

        _faderName = definition->fader()->str();
        _fader = Fader::Construct(_faderName);

        if (_channelState != eChannelPlaybackState_FadingIn && _channelState != eChannelPlaybackState_FadingOut)
            _channelState = eChannelPlaybackState_Playing;

        SoundInstance* instance = sound->CreateInstance();

        const bool success = !IsReal() || _realChannel.Play(instance);
        if (!success)
            SoundImpl::DestroyInstance(instance);

        return success;
    }

    bool ChannelInternalState::PlaySound()
    {
        AMPLITUDE_ASSERT(_sound != nullptr);

        const SoundDefinition* definition = _sound->GetDefinition();

        _faderName = definition->fader()->str();
        _fader = Fader::Construct(_faderName);

        _channelState = eChannelPlaybackState_Playing;

        SoundInstance* instance = _sound->CreateInstance();

        const bool success = !IsReal() || _realChannel.Play(instance);
        if (!success)
            SoundImpl::DestroyInstance(instance);

        return success;
    }

    void ChannelInternalState::EnableInstancing(eChannelInstanceMode mode)
    {
        if (_instancingEnabled)
            return; // Already enabled

        _instancingEnabled = true;
        _instancingMode = mode;
        _nextInstanceId = 1;

        PublishInstanceSnapshot();
    }

    void ChannelInternalState::DisableInstancing()
    {
        if (!_instancingEnabled)
            return;

        ClearInstances();

        _instancingEnabled = false;
        _instancingMode = eChannelInstanceMode_Blended;
        _nextInstanceId = 1;
    }

    ChannelInstanceInternalState* ChannelInternalState::AddInstance(const AmVector3& location)
    {
        if (!_instancingEnabled)
            return nullptr;

        if (_instances.size() >= kAmMaxChannelInstances)
        {
            amLogWarning("Cannot add instance: maximum limit of %zu instances reached.", kAmMaxChannelInstances);
            return nullptr;
        }

        auto* instance = ampoolnew(eMemoryPoolKind_Engine, ChannelInstanceInternalState, this);
        instance->SetId(_nextInstanceId++);
        instance->SetGeneration(gChannelInstanceGeneration.fetch_add(1, std::memory_order_relaxed));

        instance->_location = location;
        instance->_previousLocation = location;

        // For separate mode, initialize cursor at 0 if channel is already playing
        // This allows instances to start at different points in time
        if (_instancingMode == eChannelInstanceMode_Separate && !_realChannel._layers.empty())
            instance->SetCursor(0);

        _instances.push_back(*instance);
        _instancesMap[instance->GetId()] = instance;

        PublishInstanceSnapshot();

        return instance;
    }

    void ChannelInternalState::RemoveInstance(AmChannelInstanceID instanceId)
    {
        auto it = _instancesMap.find(instanceId);
        if (it == _instancesMap.end())
            return;

        ChannelInstanceInternalState* instance = it->second;
        instance->instance_node.remove();
        _instancesMap.erase(it);

        instance->Invalidate();
        ampooldelete(eMemoryPoolKind_Engine, ChannelInstanceInternalState, instance);

        PublishInstanceSnapshot();
    }

    void ChannelInternalState::ClearInstances()
    {
        while (!_instances.empty())
        {
            ChannelInstanceInternalState& instance = _instances.front();
            instance.instance_node.remove();
            instance.Invalidate();
            ampooldelete(eMemoryPoolKind_Engine, ChannelInstanceInternalState, &instance);
        }

        _instancesMap.clear();

        PublishInstanceSnapshot();
    }

    void ChannelInternalState::PublishInstanceSnapshot()
    {
        // Lazily allocate the cursor write-back slots. Only instanced channels pay for it.
        if (_instancingEnabled && _instanceSnapshotState.cursors == nullptr)
        {
            _instanceSnapshotState.cursors = std::make_unique<ChannelInstanceCursorSlot[]>(kAmMaxChannelInstances);
            for (AmSize i = 0; i < kAmMaxChannelInstances; ++i)
            {
                _instanceSnapshotState.cursors[i].id.store(kAmInvalidObjectId, std::memory_order_relaxed);
                _instanceSnapshotState.cursors[i].cursor.store(0, std::memory_order_relaxed);
            }
        }

        // Drain audio-thread cursor write-backs into the authoritative states, using
        // the currently published snapshot's slot pairing.
        if (_instanceSnapshotState.cursors != nullptr)
        {
            const auto& published =
                _instanceSnapshotState.snapshots[_instanceSnapshotState.publishedSnapshot.load(std::memory_order_acquire)];
            for (AmSize i = 0; i < published.size(); ++i)
            {
                const AmChannelInstanceID id = _instanceSnapshotState.cursors[i].id.load(std::memory_order_acquire);
                if (id == kAmInvalidObjectId || id != published[i].instanceId)
                    continue;

                if (auto it = _instancesMap.find(id); it != _instancesMap.end())
                    it->second->SetCursor(_instanceSnapshotState.cursors[i].cursor.load(std::memory_order_acquire));
            }
        }

        // Rebuild the back buffer from the authoritative list.
        const AmUInt32 current = _instanceSnapshotState.publishedSnapshot.load(std::memory_order_relaxed);
        const AmUInt32 next = 1 - current;
        auto& snapshot = _instanceSnapshotState.snapshots[next];

        snapshot.clear();
        snapshot.reserve(_instances.size());
        for (const auto& instance : _instances)
        {
            ChannelInstanceData data;
            data.instanceId = instance.GetId();
            data.location = instance.GetLocation();
            data.room = instance.GetRoom();
            data.weight = instance.GetWeight();
            data.computedGain = instance.GetComputedGain();
            data.cursor = instance.GetCursor();
            snapshot.push_back(data);
        }

        // Re-pair the write-back slots with the new snapshot before publishing it.
        if (_instanceSnapshotState.cursors != nullptr)
        {
            for (AmSize i = 0; i < kAmMaxChannelInstances; ++i)
            {
                const AmChannelInstanceID id = i < snapshot.size() ? snapshot[i].instanceId : kAmInvalidObjectId;
                _instanceSnapshotState.cursors[i].id.store(id, std::memory_order_release);
            }
        }

        // Publish: this release store pairs with the acquire load in AcquireInstanceSnapshot().
        _instanceSnapshotState.publishedSnapshot.store(next, std::memory_order_release);

        // Keep per-instance mixer pipelines in step with the published instances (game thread). Blended mode
        // and disabled instancing use the shared layer pipeline, so they sync with an empty list.
        if (_realChannel.Valid())
        {
            static const std::vector<ChannelInstanceData> kNoInstances;
            const bool separate = _instancingEnabled && _instancingMode == eChannelInstanceMode_Separate;
            _realChannel.SyncInstancePipelines(separate ? snapshot : kNoInstances);
        }
    }

    const std::vector<ChannelInstanceData>& ChannelInternalState::GetPublishedInstanceSnapshot() const
    {
        return _instanceSnapshotState.snapshots[_instanceSnapshotState.publishedSnapshot.load(std::memory_order_acquire)];
    }

    ChannelInstanceInternalState* ChannelInternalState::GetInstance(AmChannelInstanceID instanceId)
    {
        auto it = _instancesMap.find(instanceId);
        return it != _instancesMap.end() ? it->second : nullptr;
    }

    const ChannelInstanceInternalState* ChannelInternalState::GetInstance(AmChannelInstanceID instanceId) const
    {
        auto it = _instancesMap.find(instanceId);
        return it != _instancesMap.end() ? it->second : nullptr;
    }
} // namespace SparkyStudios::Audio::Amplitude
