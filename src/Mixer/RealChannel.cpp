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
#include <ranges>

#include <SparkyStudios/Audio/Amplitude/Core/Playback/Channel.h>
#include <SparkyStudios/Audio/Amplitude/IO/Log.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Collection.h>

#include <Core/Engine.h>
#include <Core/Playback/ChannelInternalState.h>
#include <Sound/Sound.h>

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

    bool RealChannel::Play(const std::vector<SoundInstance*>& instances)
    {
        if (instances.empty())
            return false;

        bool success = true;
        AmUInt32 layer = FindFreeLayer(_layers.empty() ? 1 : _layers.begin()->first);
        std::vector<AmUInt32> layers;

        for (auto& instance : instances)
        {
            success &= Play(instance, layer);
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

    bool RealChannel::Play(SoundInstance* sound, AmUInt32 layer)
    {
        AMPLITUDE_ASSERT(sound != nullptr);

        LayerData& data = _layers[layer];
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

        const PlayStateFlag loops = data.isLoop ? ePSF_LOOP : ePSF_PLAY;

        data.mixerLayerId =
            _mixer->Play(static_cast<SoundData*>(sound->GetUserData()), loops, GetGain(layer), _pitch, _playSpeed, _channelId, 0);

        const bool success = data.mixerLayerId != kAmInvalidObjectId;
        if (!success)
        {
            data.mixerLayerId = kAmInvalidObjectId;
            amLogError("Could not play sound '" AM_OS_CHAR_FMT "'.", data.soundInstance->GetSound()->GetPath().c_str());
        }

        return success;
    }

    void RealChannel::Destroy(AmUInt32 layer)
    {
        AMPLITUDE_ASSERT(Valid());

        const auto it = _layers.find(layer);
        if (it == _layers.end() || it->second.mixerLayerId == kAmInvalidObjectId)
            return;

        const AmUInt32 mixerLayerId = it->second.mixerLayerId;
        SoundInstance* soundInstance = it->second.soundInstance;

        const MixerCommandCallback callback = [this, layer, mixerLayerId, soundInstance]() -> bool
        {
            _mixer->SetPlayState(_channelId, mixerLayerId, ePSF_MIN);

            ampooldelete(eMemoryPoolKind_Engine, SoundInstance, soundInstance);
            _layers.erase(layer);

            return true;
        };

        if (_mixer->IsInsideThreadMutex())
        {
            _mixer->PushCommand({ callback });
            return;
        }

        AM_UNUSED(callback());
    }

    bool RealChannel::Playing() const
    {
        AMPLITUDE_ASSERT(Valid());

        if (_layers.empty())
            return false;

        for (const auto& [layerIdx, data] : _layers)
        {
            if (data.mixerLayerId == 0)
                continue;

            if (!Playing(layerIdx))
                return false;
        }

        return true;
    }

    bool RealChannel::Playing(AmUInt32 layer) const
    {
        AMPLITUDE_ASSERT(Valid());

        const auto& data = _layers.at(layer);
        const AmUInt32 state = _mixer->GetPlayState(_channelId, data.mixerLayerId);
        if (state < ePSF_PLAY)
            return false;

        if (const auto* collection = _parentChannelState->GetCollection(); collection == nullptr)
        {
            return (!data.isLoop && state == ePSF_PLAY) || (data.isLoop && state == ePSF_LOOP);
        }
        else
        {
            const CollectionPlayMode mode = static_cast<const CollectionImpl*>(collection)->GetDefinition()->play_mode();

            return mode == CollectionPlayMode_PlayOne && !data.isLoop ? state == ePSF_PLAY
                : mode == CollectionPlayMode_PlayOne && data.isLoop   ? state == ePSF_LOOP
                                                                      : _channelId != kAmInvalidObjectId;
        }
    }

    bool RealChannel::Paused() const
    {
        AMPLITUDE_ASSERT(Valid());

        if (_layers.empty())
            return false;

        for (const auto& [layerIdx, data] : _layers)
        {
            if (data.mixerLayerId == 0)
                continue;

            if (!Paused(layerIdx))
                return false;
        }

        return true;
    }

    bool RealChannel::Paused(AmUInt32 layer) const
    {
        AMPLITUDE_ASSERT(Valid());
        return _mixer->GetPlayState(_channelId, _layers.at(layer).mixerLayerId) == ePSF_HALT;
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
        return _mixer->SetPlayState(_channelId, _layers.at(layer).mixerLayerId, ePSF_STOP);
    }

    bool RealChannel::Halt()
    {
        AMPLITUDE_ASSERT(Valid());

        bool success = true;
        for (const auto& layer : _layers | std::views::keys)
            success &= Halt(layer);

        return success;
    }

    bool RealChannel::Pause(AmUInt32 layer)
    {
        AMPLITUDE_ASSERT(Valid());
        return _mixer->SetPlayState(_channelId, _layers.at(layer).mixerLayerId, ePSF_HALT);
    }

    bool RealChannel::Pause()
    {
        AMPLITUDE_ASSERT(Valid());

        bool success = true;
        for (const auto& layer : _layers | std::views::keys)
            success &= Pause(layer);

        return success;
    }

    bool RealChannel::Resume(AmUInt32 layer)
    {
        AMPLITUDE_ASSERT(Valid());
        return _mixer->SetPlayState(_channelId, _layers.at(layer).mixerLayerId, _layers.at(layer).isLoop ? ePSF_LOOP : ePSF_PLAY);
    }

    bool RealChannel::Resume()
    {
        AMPLITUDE_ASSERT(Valid());

        bool success = true;
        for (const auto& layer : _layers | std::views::keys)
            success &= Resume(layer);

        return success;
    }

    bool RealChannel::Seek(AmTime position)
    {
        AMPLITUDE_ASSERT(Valid());

        if (_layers.size() != 1)
        {
            amLogWarning("Cannot seek real channel " AM_ID_CHAR_FMT ". Seeking is only supported on single-layer channels.", _channelId);
            return false;
        }

        auto& data = _layers.begin()->second;
        if (data.mixerLayerId == kAmInvalidObjectId || data.soundInstance == nullptr)
            return false;

        const auto* soundData = static_cast<const SoundData*>(data.soundInstance->GetUserData());
        if (soundData == nullptr || soundData->format.GetSampleRate() == 0)
            return false;

        const AmTime clampedPosition = std::max<AmTime>(position, 0.0);
        const AmUInt64 cursor = static_cast<AmUInt64>(clampedPosition * static_cast<AmTime>(soundData->format.GetSampleRate()) / kAmSecond);

        return _mixer->SetCursor(_channelId, data.mixerLayerId, cursor);
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
        if (!_mixer->GetCursor(_channelId, data.mixerLayerId, cursor))
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

    AmUInt32 RealChannel::FindFreeLayer(AmUInt32 layerIndex) const
    {
        while (_layers.contains(layerIndex))
            layerIndex++;

        return layerIndex;
    }
} // namespace SparkyStudios::Audio::Amplitude
