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
#include <cassert>
#include <ranges>

#include <SparkyStudios/Audio/Amplitude/Core/Playback/Channel.h>
#include <SparkyStudios/Audio/Amplitude/IO/Log.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Collection.h>

#include <Core/Engine.h>
#include <Core/Playback/ChannelInternalState.h>
#include <Sound/Sound.h>

#include <Mixer/Amplimix.h>
#include <Mixer/RealChannel.h>

#include "collection_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    RealChannel::RealChannel()
        : RealChannel(nullptr)
    {}

    RealChannel::RealChannel(ChannelInternalState* parent)
        : _channelId(kAmInvalidObjectId)
        , _layers()
        , _defaultGain(1.0f)
        , _pitch(1.0f)
        , _playSpeed(1.0f)
        , _mixer(nullptr)
        , _parentChannelState(parent)
        , _playedSounds()
    {}

    void RealChannel::Initialize(AmChannelID i)
    {
        _channelId = i;
        _mixer = &amEngine->GetState()->mixer;
        _instancePipelineOverflowWarned = false;
    }

    void RealChannel::MarkAsPlayed(const Sound* sound)
    {
        _playedSounds.insert(sound->GetId());
    }

    bool RealChannel::AllSoundsHasPlayed() const
    {
        if (_parentChannelState->GetCollection() == nullptr)
            return false;

        for (auto&& sound : _parentChannelState->GetCollection()->GetSounds())
            if (!_playedSounds.contains(sound))
                return false;

        return true;
    }

    void RealChannel::ClearPlayedSounds()
    {
        _playedSounds.clear();
    }

    ChannelInternalState* RealChannel::GetParentChannelState() const
    {
        return _parentChannelState;
    }

    AmChannelID RealChannel::GetID() const
    {
        return _channelId;
    }

    bool RealChannel::Valid() const
    {
        return _channelId != kAmInvalidObjectId && _mixer != nullptr && _parentChannelState != nullptr;
    }

    bool RealChannel::Play(const std::vector<SoundInstance*>& instances, const RealChannelPlayOptions& options)
    {
        if (instances.empty())
            return false;

        bool success = true;
        AmUInt32 layer = FindFreeLayer(_layers.empty() ? 1 : _layers.begin()->first);
        std::vector<AmUInt32> layers;

        for (auto& instance : instances)
        {
            success &= Play(instance, layer, options);
            layers.push_back(layer);

            if (!success)
            {
                for (auto&& l : layers)
                    Destroy(l);

                return false;
            }

            layer = FindFreeLayer(layer);
        }

        return success;
    }

    bool RealChannel::Play(SoundInstance* sound, AmUInt32 layer, const RealChannelPlayOptions& playOptions)
    {
        AMPLITUDE_ASSERT(sound != nullptr);

        LayerData& data = _layers[layer];

        // A (re)used layer starts without per-instance pipelines.
        data.instanceTableInstalled = false;
        data.attachedInstanceIds.clear();
        data.attachedStreamIds.clear();

        data.soundInstance = sound;
        data.soundInstance->SetChannel(this);
        data.soundInstance->Load();

        if (sound->GetUserData() == nullptr)
        {
            data.mixerLayerId = kAmInvalidObjectId;
            amLogError("The sound was not loaded successfully.");
            return false;
        }

        data.isLoop = sound->GetSound()->IsLoop();
        data.isStream = sound->GetSound()->IsStream();
        data.gain = _defaultGain;

        VoiceStartOptions options;
        options.loop = data.isLoop;
        options.loopCount = sound->GetSettings().m_loopCount;
        options.gain = GetGain(layer);
        options.pitch = _pitch;
        options.speed = _playSpeed;
        options.faderName = _parentChannelState->GetFaderName();
        options.fadeIn = playOptions.fadeIn;

        data.mixerLayerId = _mixer->StartVoice(static_cast<SoundData*>(sound->GetUserData()), options, _channelId, 0);
        data.paused = false;
        data.stopping = false;

        const bool success = data.mixerLayerId != kAmInvalidObjectId;
        if (!success)
        {
            data.mixerLayerId = kAmInvalidObjectId;
            amLogError("Could not play sound '" AM_OS_CHAR_FMT "'.", data.soundInstance->GetSound()->GetPath().c_str());
        }

        // A sound that starts on a channel already in separate mode gets its instance pipelines now.
        if (success)
            SyncCurrentInstancePipelines();

        return success;
    }

    void RealChannel::Destroy(AmUInt32 layer)
    {
        AMPLITUDE_ASSERT(Valid());

        const auto it = _layers.find(layer);
        if (it == _layers.end() || it->second.mixerLayerId == kAmInvalidObjectId)
            return;

        // The mixer layer owns the sound instance (through its sound data) and deletes it when it is destroyed, after
        // the current mix: deleting it here would free the sound data the voice is still rendering.
        _mixer->DiscardVoice(_channelId, it->second.mixerLayerId);
        _layers.erase(it);
    }

    bool RealChannel::Playing() const
    {
        AMPLITUDE_ASSERT(Valid());

        bool any = false;
        for (const auto& [layerIdx, data] : _layers)
        {
            // Layers fading out after a stop or a steal are forgotten on Finished: they do not count.
            if (data.mixerLayerId == 0 || data.stopping)
                continue;

            if (!Playing(layerIdx))
                return false;

            any = true;
        }

        return any;
    }

    bool RealChannel::Playing(AmUInt32 layer) const
    {
        AMPLITUDE_ASSERT(Valid());

        const auto& data = _layers.at(layer);
        if (data.paused || data.stopping)
            return false;

        if (const auto* collection = _parentChannelState->GetCollection(); collection != nullptr)
        {
            const CollectionPlayMode mode = static_cast<const CollectionImpl*>(collection)->GetDefinition()->play_mode();
            if (mode == CollectionPlayMode_PlayAll)
                return _channelId != kAmInvalidObjectId; // the collection chains sounds: the channel plays between them
        }

        // The game side learns that a voice ended only from its events: Ended halts the channel and Finished forgets
        // the layer. A Finished state published by the audio thread before those events are dispatched must not stop
        // the channel early (it would be recycled without its End and Stop callbacks). Idle means the layer is gone.
        return _mixer->GetVoiceState(_channelId, data.mixerLayerId) != eVoiceState::Idle;
    }

    bool RealChannel::Paused() const
    {
        AMPLITUDE_ASSERT(Valid());

        bool any = false;
        for (const auto& [layerIdx, data] : _layers)
        {
            if (data.mixerLayerId == 0 || data.stopping)
                continue;

            if (!Paused(layerIdx))
                return false;

            any = true;
        }

        return any;
    }

    bool RealChannel::Paused(AmUInt32 layer) const
    {
        AMPLITUDE_ASSERT(Valid());
        return _layers.at(layer).paused;
    }

    void RealChannel::SetGain(const AmReal32 gain)
    {
        AMPLITUDE_ASSERT(Valid());

        for (auto&& [layerIdx, data] : _layers)
        {
            if (data.mixerLayerId == 0)
                continue;

            SetGain(gain, layerIdx);
        }

        _defaultGain = gain;
    }

    void RealChannel::SetGain(AmReal32 gain, AmUInt32 layer)
    {
        auto& data = _layers.at(layer);
        AmReal32 finalGain = gain;
        if (data.soundInstance->GetSettings().m_kind != SoundKind::Standalone)
            finalGain = gain * data.soundInstance->GetSettings().m_gain.GetValue();

        _mixer->SetGain(_channelId, data.mixerLayerId, finalGain);
        data.gain = gain;
    }

    AmReal32 RealChannel::GetGain(AmUInt32 layer) const
    {
        AMPLITUDE_ASSERT(Valid());
        if (const auto it = _layers.find(layer); it != _layers.end())
            return it->second.gain;
        return _defaultGain;
    }

    bool RealChannel::Halt(AmUInt32 layer)
    {
        AMPLITUDE_ASSERT(Valid());
        auto& data = _layers.at(layer);

        // A layer whose voice failed to start has no mixer layer: posting to it would target slot 0.
        if (data.mixerLayerId == kAmInvalidObjectId)
            return false;

        _mixer->PostVoiceCommand(_channelId, data.mixerLayerId, eVoiceCommandKind::Stop);
        data.stopping = true;
        return true;
    }

    bool RealChannel::Halt()
    {
        AMPLITUDE_ASSERT(Valid());

        // Layers without a mixer layer have nothing to post to and do not fail the channel-wide command.
        bool success = true;
        for (const auto& [layer, data] : _layers)
            if (data.mixerLayerId != kAmInvalidObjectId)
                success &= Halt(layer);

        return success;
    }

    bool RealChannel::Pause(AmUInt32 layer)
    {
        AMPLITUDE_ASSERT(Valid());
        auto& data = _layers.at(layer);

        // A layer whose voice failed to start has no mixer layer: posting to it would target slot 0.
        if (data.mixerLayerId == kAmInvalidObjectId)
            return false;

        _mixer->PostVoiceCommand(_channelId, data.mixerLayerId, eVoiceCommandKind::Pause);
        data.paused = true;
        return true;
    }

    bool RealChannel::Pause()
    {
        AMPLITUDE_ASSERT(Valid());

        // Layers without a mixer layer have nothing to post to and do not fail the channel-wide command.
        bool success = true;
        for (const auto& [layer, data] : _layers)
            if (data.mixerLayerId != kAmInvalidObjectId)
                success &= Pause(layer);

        return success;
    }

    bool RealChannel::Resume(AmUInt32 layer)
    {
        AMPLITUDE_ASSERT(Valid());
        auto& data = _layers.at(layer);

        // A layer whose voice failed to start has no mixer layer: posting to it would target slot 0.
        if (data.mixerLayerId == kAmInvalidObjectId)
            return false;

        _mixer->PostVoiceCommand(_channelId, data.mixerLayerId, eVoiceCommandKind::Resume);
        data.paused = false;
        return true;
    }

    bool RealChannel::Resume()
    {
        AMPLITUDE_ASSERT(Valid());

        // Layers without a mixer layer have nothing to post to and do not fail the channel-wide command.
        bool success = true;
        for (const auto& [layer, data] : _layers)
            if (data.mixerLayerId != kAmInvalidObjectId)
                success &= Resume(layer);

        return success;
    }

    bool RealChannel::FadeOut(AmTime duration, eVoiceCommandKind kind)
    {
        AMPLITUDE_ASSERT(Valid());
        AMPLITUDE_ASSERT(kind == eVoiceCommandKind::Stop || kind == eVoiceCommandKind::Pause);

        // No game-side flag here: the layer keeps "playing" until the voice reports the end of the fade.
        for (const auto& [index, data] : _layers)
            _mixer->PostVoiceCommand(_channelId, data.mixerLayerId, kind, duration);

        return !_layers.empty();
    }

    void RealChannel::FadeOutLayer(AmUInt32 layer, AmTime duration)
    {
        AMPLITUDE_ASSERT(Valid());
        _mixer->PostVoiceCommand(_channelId, _layers.at(layer).mixerLayerId, eVoiceCommandKind::Stop, duration);
    }

    bool RealChannel::ResumeWithFade(AmTime duration)
    {
        AMPLITUDE_ASSERT(Valid());

        for (auto& [index, data] : _layers)
        {
            _mixer->PostVoiceCommand(_channelId, data.mixerLayerId, eVoiceCommandKind::Resume, duration);
            data.paused = false;
        }

        return !_layers.empty();
    }

    void RealChannel::MarkLayerPaused(AmUInt32 mixerLayerId)
    {
        for (auto& [index, data] : _layers)
            if (data.mixerLayerId == mixerLayerId)
                data.paused = true;
    }

    bool RealChannel::Seek(AmTime position)
    {
        AMPLITUDE_ASSERT(Valid());

        if (_layers.size() != 1)
        {
            amLogWarning("Cannot seek real channel " AM_ID_CHAR_FMT ". Seeking is only supported on single-layer channels.", _channelId);
            return false;
        }

        const auto& data = _layers.begin()->second;
        if (data.mixerLayerId == kAmInvalidObjectId || data.soundInstance == nullptr)
            return false;

        const auto* soundData = static_cast<const SoundData*>(data.soundInstance->GetUserData());
        if (soundData == nullptr || soundData->format.GetSampleRate() == 0)
            return false;

        const AmTime clampedPosition = std::max<AmTime>(position, 0.0);
        const AmUInt64 cursor = static_cast<AmUInt64>(clampedPosition * static_cast<AmTime>(soundData->format.GetSampleRate()) / kAmSecond);

        _mixer->PostVoiceCommand(_channelId, data.mixerLayerId, eVoiceCommandKind::Seek, 0.0, cursor);
        return true;
    }

    AmTime RealChannel::GetPlaybackPosition() const
    {
        AMPLITUDE_ASSERT(Valid());

        if (_layers.size() != 1)
        {
            amLogWarning(
                "Cannot query playback position for real channel " AM_ID_CHAR_FMT
                ". Playback position is only supported on single-layer channels.",
                _channelId);
            return 0.0;
        }

        const auto& data = _layers.begin()->second;
        if (data.mixerLayerId == kAmInvalidObjectId || data.soundInstance == nullptr)
            return 0.0;

        const auto* soundData = static_cast<const SoundData*>(data.soundInstance->GetUserData());
        if (soundData == nullptr || soundData->format.GetSampleRate() == 0)
            return 0.0;

        AmUInt64 cursor = 0;
        if (!_mixer->GetVoicePosition(_channelId, data.mixerLayerId, cursor))
            return 0.0;

        return static_cast<AmTime>(cursor) * kAmSecond / static_cast<AmTime>(soundData->format.GetSampleRate());
    }

    void RealChannel::SetPitch(AmReal32 pitch)
    {
        AMPLITUDE_ASSERT(Valid());

        for (auto&& [layerIdx, data] : _layers)
        {
            if (data.mixerLayerId == 0)
                continue;

            AmReal32 finalPitch = pitch;
            if (data.soundInstance->GetSettings().m_kind != SoundKind::Standalone)
                finalPitch = pitch * data.soundInstance->GetSettings().m_pitch.GetValue();

            _mixer->SetPitch(_channelId, data.mixerLayerId, finalPitch);
        }

        _pitch = pitch;
    }

    void RealChannel::SetSpeed(AmReal32 speed)
    {
        AMPLITUDE_ASSERT(Valid());

        for (const auto& [layerIdx, data] : _layers)
        {
            if (data.mixerLayerId == 0)
                continue;

            _mixer->SetPlaySpeed(_channelId, data.mixerLayerId, speed);
        }

        _playSpeed = speed;
    }

    void RealChannel::SetObstruction(AmReal32 obstruction)
    {
        AMPLITUDE_ASSERT(Valid());

        for (const auto& [layerIdx, data] : _layers)
        {
            if (data.mixerLayerId == 0)
                continue;

            _mixer->SetObstruction(_channelId, data.mixerLayerId, obstruction);
        }
    }

    void RealChannel::SetOcclusion(AmReal32 occlusion)
    {
        AMPLITUDE_ASSERT(Valid());

        for (const auto& [layerIdx, data] : _layers)
        {
            if (data.mixerLayerId == 0)
                continue;

            _mixer->SetOcclusion(_channelId, data.mixerLayerId, occlusion);
        }
    }

    void RealChannel::SyncInstancePipelines(const std::vector<ChannelInstanceData>& instances)
    {
        if (!Valid())
            return;

        const AmSize cap = _mixer->GetMaxInstancePipelines();

        for (auto& [layerIndex, data] : _layers)
        {
            if (data.mixerLayerId == kAmInvalidObjectId)
                continue;

            // Detach pipelines of instances that are gone.
            for (auto it = data.attachedInstanceIds.begin(); it != data.attachedInstanceIds.end();)
            {
                const AmChannelInstanceID id = *it;
                const bool alive = std::ranges::any_of(
                    instances,
                    [id](const ChannelInstanceData& instance)
                    {
                        return instance.instanceId == id;
                    });

                if (alive)
                {
                    ++it;
                    continue;
                }

                AM_UNUSED(_mixer->DetachInstancePipeline(_channelId, data.mixerLayerId, id));
                it = data.attachedInstanceIds.erase(it);
            }

            for (auto it = data.attachedStreamIds.begin(); it != data.attachedStreamIds.end();)
            {
                const AmChannelInstanceID id = *it;
                const bool alive = std::ranges::any_of(
                    instances,
                    [id](const ChannelInstanceData& instance)
                    {
                        return instance.instanceId == id;
                    });

                if (alive)
                {
                    ++it;
                    continue;
                }

                AM_UNUSED(_mixer->DetachInstanceStream(_channelId, data.mixerLayerId, id));
                it = data.attachedStreamIds.erase(it);
            }

            if (instances.empty())
                continue;

            if (!data.instanceTableInstalled)
                data.instanceTableInstalled = _mixer->InstallInstancePipelineTable(_channelId, data.mixerLayerId);

            if (!data.instanceTableInstalled)
                continue;

            // Every instance renders through its own stream, so it keeps its own cursor and resampler phase.
            for (const auto& instance : instances)
            {
                if (std::ranges::find(data.attachedStreamIds, instance.instanceId) != data.attachedStreamIds.end())
                    continue;

                if (_mixer->AttachInstanceStream(_channelId, data.mixerLayerId, instance.instanceId))
                    data.attachedStreamIds.push_back(instance.instanceId);
            }

            // Per-instance pipelines are capped; instances beyond the cap share the layer pipeline.
            if (cap == 0)
                continue;

            // Attach pipelines for new instances, up to the cap.
            for (const auto& instance : instances)
            {
                if (std::ranges::find(data.attachedInstanceIds, instance.instanceId) != data.attachedInstanceIds.end())
                    continue;

                if (data.attachedInstanceIds.size() >= cap)
                {
                    if (!_instancePipelineOverflowWarned)
                    {
                        amLogWarning(
                            "Channel instance pipelines cap (%zu) reached: extra instances share one pipeline.",
                            static_cast<size_t>(cap));
                        _instancePipelineOverflowWarned = true;
                    }

                    break;
                }

                if (_mixer->AttachInstancePipeline(_channelId, data.mixerLayerId, instance.instanceId))
                    data.attachedInstanceIds.push_back(instance.instanceId);
            }
        }
    }

    AmSize RealChannel::GetAttachedInstancePipelineCount() const
    {
        AmSize count = 0;
        for (const auto& [layerIndex, data] : _layers)
            count += data.attachedInstanceIds.size();

        return count;
    }

    std::vector<AmUInt32> RealChannel::GetMixerLayerIds() const
    {
        std::vector<AmUInt32> ids;
        for (const auto& [layerIndex, data] : _layers)
            if (data.mixerLayerId != kAmInvalidObjectId)
                ids.push_back(data.mixerLayerId);

        return ids;
    }

    bool RealChannel::OwnsMixerLayer(AmUInt32 mixerLayerId, const SoundInstance* sound) const
    {
        for (const auto& [index, data] : _layers)
            if (data.mixerLayerId == mixerLayerId && data.soundInstance == sound)
                return true;

        return false;
    }

    void RealChannel::ForgetMixerLayer(AmUInt32 mixerLayerId)
    {
        for (auto it = _layers.begin(); it != _layers.end(); ++it)
        {
            if (it->second.mixerLayerId != mixerLayerId)
                continue;

            _layers.erase(it);
            return;
        }
    }

    bool RealChannel::HasSoundingLayers() const
    {
        if (!Valid())
            return false;

        for (const auto& [index, data] : _layers)
        {
            if (!data.stopping || data.mixerLayerId == kAmInvalidObjectId)
                continue;

            // Stopping layers are forgotten on their Finished event; Idle covers a layer that is already gone.
            if (_mixer->GetVoiceState(_channelId, data.mixerLayerId) != eVoiceState::Idle)
                return true;
        }

        return false;
    }

    void RealChannel::SyncCurrentInstancePipelines()
    {
        if (_parentChannelState == nullptr || !_parentChannelState->IsInstancingEnabled() ||
            _parentChannelState->GetInstancingMode() != eChannelInstanceMode_Separate)
            return;

        SyncInstancePipelines(_parentChannelState->GetPublishedInstanceSnapshot());
    }

    AmUInt32 RealChannel::FindFreeLayer(AmUInt32 layerIndex) const
    {
        while (_layers.contains(layerIndex))
            layerIndex++;

        return layerIndex;
    }
} // namespace SparkyStudios::Audio::Amplitude
