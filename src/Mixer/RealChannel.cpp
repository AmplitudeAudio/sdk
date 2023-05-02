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
#include <Mixer/RealChannel.h>

#include "collection_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    RealChannel::RealChannel()
        : RealChannel(nullptr)
    {}

    RealChannel::RealChannel(ChannelInternalState* parent)
        : _channelId(kAmInvalidObjectId)
        , _channelLayersId()
        , _stream()
        , _loop()
        , _pan()
        , _gain()
        , _pitch(1.0f)
        , _playSpeed(1.0f)
        , _mixer(nullptr)
        , _activeSounds()
        , _parentChannelState(parent)
        , _playedSounds()
    {}

    void RealChannel::Initialize(int i)
    {
        _channelId = i;
        _mixer = &Engine::GetInstance()->GetState()->mixer;
    }

    void RealChannel::MarkAsPlayed(const Sound* sound)
    {
        _playedSounds.push_back(sound->GetId());
    }

    bool RealChannel::AllSoundsHasPlayed() const
    {
        if (_parentChannelState->GetCollection() == nullptr)
            return false;

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
        return _channelId != kAmInvalidObjectId && _mixer != nullptr && _parentChannelState != nullptr;
    }

    bool RealChannel::Play(const std::vector<SoundInstance*>& instances)
    {
        if (instances.empty())
        {
            return false;
        }

        bool success = true;
        AmUInt32 layer = FindFreeLayer(_channelLayersId.empty() ? 1 : _channelLayersId.rbegin()->first);
        std::vector<AmUInt32> layers;

        for (auto& instance : instances)
        {
            success &= Play(instance, layer);
            layers.push_back(layer);

            if (!success)
            {
                for (auto&& layer : layers)
                {
                    Destroy(layer);
                }
                return false;
            }

            layer = FindFreeLayer(layer);
        }

        return success;
    }

    bool RealChannel::Play(SoundInstance* sound, AmUInt32 layer)
    {
        AMPLITUDE_ASSERT(sound != nullptr);

        _activeSounds[layer] = sound;
        _activeSounds[layer]->SetChannel(this);
        _activeSounds[layer]->Load();

        if (!_activeSounds[layer]->GetUserData())
        {
            _channelLayersId[layer] = kAmInvalidObjectId;
            CallLogFunc("[ERROR] The sound was not loaded successfully.");
            return false;
        }

        _loop[layer] = _activeSounds[layer]->GetSound()->IsLoop();
        _stream[layer] = _activeSounds[layer]->GetSound()->IsStream();

        const PlayStateFlag loops = _loop[layer] ? PLAY_STATE_FLAG_LOOP : PLAY_STATE_FLAG_PLAY;

        _channelLayersId[layer] = _mixer->Play(
            static_cast<SoundData*>(_activeSounds[layer]->GetUserData()), loops, _gain[layer], _pan, _pitch, _playSpeed, _channelId, 0);

        // Check if playing the sound was successful, and display the error if it was not.
        const bool success = _channelLayersId[layer] != kAmInvalidObjectId;
        if (!success)
        {
            _channelLayersId[layer] = kAmInvalidObjectId;
            CallLogFunc("[ERROR] Could not play sound " AM_OS_CHAR_FMT "\n", _activeSounds[layer]->GetSound()->GetFilename().c_str());
        }

        return success;
    }

    void RealChannel::Destroy(AmUInt32 layer)
    {
        AMPLITUDE_ASSERT(Valid() && _channelLayersId[layer] != kAmInvalidObjectId);

        MixerCommandCallback callback = [&, layer]() -> bool
        {
            _mixer->SetPlayState(_channelId, _channelLayersId[layer], PLAY_STATE_FLAG_MIN);

            _channelLayersId.erase(layer);

            delete _activeSounds[layer];
            _activeSounds.erase(layer);

            return true;
        };

        if (_mixer->IsInsideThreadMutex())
        {
            _mixer->PushCommand({ callback });
            return;
        }

        callback();
    }

    bool RealChannel::Playing() const
    {
        AMPLITUDE_ASSERT(Valid());
        bool playing = true;
        for (auto&& layer : _channelLayersId)
        {
            if (layer.second != 0)
            {
                playing &= Playing(layer.first);
            }
        }

        return playing;
    }

    bool RealChannel::Playing(AmUInt32 layer) const
    {
        AMPLITUDE_ASSERT(Valid());
        const AmUInt32 state = _mixer->GetPlayState(_channelId, _channelLayersId.at(layer));

        if (const auto* collection = _parentChannelState->GetCollection(); collection == nullptr)
        {
            return !_loop.at(layer) && state == PLAY_STATE_FLAG_PLAY || _loop.at(layer) && state == PLAY_STATE_FLAG_LOOP;
        }
        else
        {
            const CollectionPlayMode mode = collection->GetCollectionDefinition()->play_mode();

            return mode == CollectionPlayMode_PlayOne && !_loop.at(layer) ? state == PLAY_STATE_FLAG_PLAY
                : mode == CollectionPlayMode_PlayOne && _loop.at(layer)   ? state == PLAY_STATE_FLAG_LOOP
                                                                          : _channelId != kAmInvalidObjectId;
        }
    }

    bool RealChannel::Paused() const
    {
        AMPLITUDE_ASSERT(Valid());
        bool paused = true;
        for (auto&& layer : _channelLayersId)
        {
            if (layer.second != 0)
            {
                paused &= Paused(layer.first);
            }
        }

        return paused;
    }

    bool RealChannel::Paused(AmUInt32 layer) const
    {
        AMPLITUDE_ASSERT(Valid());
        return _mixer->GetPlayState(_channelId, _channelLayersId.at(layer)) == PLAY_STATE_FLAG_HALT;
    }

    void RealChannel::SetGain(const float gain)
    {
        AMPLITUDE_ASSERT(Valid());
        for (auto&& layer : _channelLayersId)
        {
            if (layer.second != 0)
            {
                SetGain(gain, layer.first);
            }
        }
    }

    void RealChannel::SetGain(float gain, AmUInt32 layer)
    {
        SetGainPan(gain, _pan, layer);
    }

    float RealChannel::GetGain(AmUInt32 layer) const
    {
        AMPLITUDE_ASSERT(Valid());
        return _gain.count(layer) > 0 ? _gain.at(layer) : 0.0f;
    }

    void RealChannel::Halt(AmUInt32 layer)
    {
        AMPLITUDE_ASSERT(Valid());
        _mixer->SetPlayState(_channelId, _channelLayersId[layer], PLAY_STATE_FLAG_STOP);
    }

    void RealChannel::Pause(AmUInt32 layer)
    {
        AMPLITUDE_ASSERT(Valid());
        _mixer->SetPlayState(_channelId, _channelLayersId[layer], PLAY_STATE_FLAG_HALT);
    }

    void RealChannel::Resume(AmUInt32 layer)
    {
        AMPLITUDE_ASSERT(Valid());
        _mixer->SetPlayState(_channelId, _channelLayersId[layer], _loop[layer] ? PLAY_STATE_FLAG_LOOP : PLAY_STATE_FLAG_PLAY);
    }

    void RealChannel::SetPan(const AmVec2& pan)
    {
        AMPLITUDE_ASSERT(Valid());
        for (auto&& layer : _channelLayersId)
        {
            if (layer.second != 0)
            {
                SetGainPan(_gain[layer.first], pan.X, layer.first);
            }
        }

        _pan = pan.X;
    }

    void RealChannel::SetPitch(AmReal32 pitch)
    {
        AMPLITUDE_ASSERT(Valid());
        for (auto&& layer : _channelLayersId)
        {
            if (layer.second != 0)
            {
                _mixer->SetPitch(_channelId, _channelLayersId[layer.first], pitch);
            }
        }

        _pitch = pitch;
    }

    void RealChannel::SetSpeed(AmReal32 speed)
    {
        AMPLITUDE_ASSERT(Valid());
        for (auto&& layer : _channelLayersId)
        {
            if (layer.second != 0)
            {
                _mixer->SetPlaySpeed(_channelId, _channelLayersId[layer.first], speed);
            }
        }

        _playSpeed = speed;
    }

    void RealChannel::SetObstruction(AmReal32 obstruction)
    {
        AMPLITUDE_ASSERT(Valid());
        for (auto&& layer : _channelLayersId)
        {
            if (layer.second != 0)
            {
                if (_activeSounds[layer.first] != nullptr)
                {
                    _activeSounds[layer.first]->SetObstruction(obstruction);
                }
            }
        }
    }

    void RealChannel::SetOcclusion(AmReal32 occlusion)
    {
        AMPLITUDE_ASSERT(Valid());
        for (auto&& layer : _channelLayersId)
        {
            if (layer.second != 0)
            {
                if (_activeSounds[layer.first] != nullptr)
                {
                    _activeSounds[layer.first]->SetOcclusion(occlusion);
                }
            }
        }
    }

    void RealChannel::SetGainPan(float gain, float pan, AmUInt32 layer)
    {
        float finalGain = gain;
        if (_activeSounds[layer]->GetSettings().m_kind != SoundKind::Standalone)
        {
            finalGain = gain * _activeSounds[layer]->GetSettings().m_gain.GetValue();
        }

        _mixer->SetGainPan(_channelId, _channelLayersId[layer], finalGain, pan);

        _gain[layer] = gain;
        _pan = pan;
    }

    AmUInt32 RealChannel::FindFreeLayer(AmUInt32 layerIndex) const
    {
        while (_channelLayersId.count(layerIndex) > 0)
        {
            layerIndex++;
        }

        return layerIndex;
    }
} // namespace SparkyStudios::Audio::Amplitude
