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

#pragma once

#ifndef SS_AMPLITUDE_AUDIO_CHANNEL_INTERNAL_STATE_H
#define SS_AMPLITUDE_AUDIO_CHANNEL_INTERNAL_STATE_H

#include <utility>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Core/Channel.h>
#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Collection.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Switch.h>
#include <SparkyStudios/Audio/Amplitude/Sound/SwitchContainer.h>

#include "RealChannel.h"

#include <Utils/intrusive_list.h>

namespace SparkyStudios::Audio::Amplitude
{
    enum class ChannelState
    {
        Stopped,
        Playing,
        FadingIn,
        FadingOut,
        SwitchingState,
        Paused,
    };

    // Represents a sample that is playing on a channel.
    class ChannelInternalState
    {
    public:
        ChannelInternalState()
            : _realChannel(this)
            , _channelState(ChannelState::Stopped)
            , _switchContainer(nullptr)
            , _collection(nullptr)
            , _sound(nullptr)
            , _switch(nullptr)
            , _playingSwitchContainerStateId(kAmInvalidObjectId)
            , _previousSwitchContainerStateId(kAmInvalidObjectId)
            , _fader(nullptr)
            , _targetFadeOutState(ChannelState::Stopped)
            , _entity()
            , _userGain(1.0f)
            , _gain(1.0f)
            , _pan()
            , _location()
        {}

        // Updates the state enum based on whether this channel is stopped, playing,
        // etc.
        void UpdateState();

        // Remove this channel from all lists that it is a part of.
        void Remove();

        /**
         * @brief Resets this channel to its initial state.
         */
        void Reset();

        // Set the switch container playing on this channel. Note that when you set
        // the switch container, you also add this channel to the bus list that
        // corresponds to that switch container.
        void SetSwitchContainer(SwitchContainer* switchContainer);

        // Get or set the collection playing on this channel. Note that when you set
        // the collection, you also add this channel to the bus list that
        // corresponds to that collection.
        void SetCollection(Collection* collection);

        // Get or set the sound playing on this channel. Note that when you set
        // the sound, you also add this channel to the bus list that
        // corresponds to that sound.
        void SetSound(Sound* sound);

        [[nodiscard]] SwitchContainer* GetSwitchContainer() const
        {
            return _switchContainer;
        }

        [[nodiscard]] Collection* GetCollection() const
        {
            return _collection;
        }

        [[nodiscard]] Sound* GetSound() const;

        void SetEntity(const Entity& entity);

        Entity& GetEntity()
        {
            return _entity;
        }

        [[nodiscard]] const Entity& GetEntity() const
        {
            return _entity;
        }

        // Get the current state of this channel (playing, stopped, paused, etc.). This
        // is tracked manually because not all ChannelInternalStates are backed by
        // real channels.
        [[nodiscard]] ChannelState GetChannelState() const
        {
            return _channelState;
        }

        // Get or set the location of this channel
        void SetLocation(const hmm_vec3& location)
        {
            // Entity scoped channel
            if (_entity.Valid())
                return;

            // World scoped channel
            _location = location;
        }

        [[nodiscard]] const hmm_vec3& GetLocation() const
        {
            // Entity scoped channel
            if (_entity.Valid())
                return _entity.GetLocation();

            // World scoped channel
            return _location;
        }

        // Play the sound associated to this channel.
        bool Play();

        // Check if this channel is currently playing on a real or virtual channel.
        [[nodiscard]] bool Playing() const;

        // Check if this channel is currently stopped on a real or virtual channel.
        [[nodiscard]] bool Stopped() const;

        // Check if this channel is currently paused on a real or virtual channel.
        [[nodiscard]] bool Paused() const;

        // Set and query the user gain of this channel.
        void SetUserGain(const float user_gain)
        {
            _userGain = user_gain;
        }

        [[nodiscard]] float GetUserGain() const
        {
            return _userGain;
        }

        // Set and query the current gain of this channel.
        void SetGain(float gain);

        [[nodiscard]] float GetGain() const
        {
            return _gain;
        }

        // Immediately stop the audio. May cause clicking.
        void Halt();

        // Pauses this channel.
        void Pause();

        // Resumes this channel if it is paused.
        void Resume();

        // Fade in over the specified number of milliseconds.
        void FadeIn(AmTime duration);

        // Fade out over the specified number of milliseconds.
        void FadeOut(AmTime duration, ChannelState targetState = ChannelState::Stopped);

        // Sets the pan based on a position in a unit circle.
        void SetPan(const hmm_vec2& pan);

        // Returns the pan of this channel.
        [[nodiscard]] const hmm_vec2& GetPan() const
        {
            return _pan;
        }

        // Devirtualizes a virtual channel. This transfers ownership of the given
        // channel's channel_id to this channel.
        void Devirtualize(ChannelInternalState* other);

        // Returns the priority of this channel based on its gain and priority
        // multiplier on the sound collection definition.
        [[nodiscard]] float Priority() const;

        /**
         * @brief Update this channel data per frames.
         *
         * @param deltaTime The time elapsed since the last frame.
         */
        void AdvanceFrame(AmTime deltaTime);

        // Returns the real channel.
        RealChannel& GetRealChannel()
        {
            return _realChannel;
        }

        [[nodiscard]] const RealChannel& GetRealChannel() const
        {
            return _realChannel;
        }

        // Returns true if the real channel is valid.
        [[nodiscard]] bool IsReal() const
        {
            return _realChannel.Valid();
        }

        [[nodiscard]] AmObjectID GetPlayingObjectId() const
        {
            if (_collection)
                return _collection->GetId();
            if (_sound)
                return _sound->GetId();

            return kAmInvalidObjectId;
        }

        [[nodiscard]] AmUInt32 GetChannelStateId() const
        {
            return _channelStateId;
        }

        void SetChannelStateId(AmUInt64 id)
        {
            _channelStateId = id;
        }

        // The node that tracks the location in the priority list.
        fplutil::intrusive_list_node priority_node;

        // The node that tracks the location in the free list.
        fplutil::intrusive_list_node free_node;

        // The node that tracks the list of sounds playing on a given bus.
        fplutil::intrusive_list_node bus_node;

        // The node that tracks the list of sounds playing on a given entity.
        fplutil::intrusive_list_node entity_node;

    private:
        bool PlaySwitchContainerStateUpdate(const std::vector<SwitchContainerItem>& previous, const std::vector<SwitchContainerItem>& next);
        bool PlaySwitchContainer();
        bool PlayCollection();
        bool PlaySound();

        RealChannel _realChannel;

        // Whether this channel is currently playing, stopped, fading out, etc.
        ChannelState _channelState;

        // The collection of the sound being played on this channel.
        SwitchContainer* _switchContainer;

        // The collection of the sound being played on this channel.
        Collection* _collection;

        // The sound source that was chosen from the sound collection.
        Sound* _sound;

        const Switch* _switch;
        AmObjectID _playingSwitchContainerStateId;
        AmObjectID _previousSwitchContainerStateId;

        // The sound fade of this channel. This is updated by the current sound collection.
        Fader* _fader;

        // The target state of the fade out transition. Must be either Paused or Stopped.
        ChannelState _targetFadeOutState;

        // The entity which is playing the sound of this channel.
        Entity _entity;

        // The gain set by the user.
        float _userGain;

        // The gain of this channel.
        float _gain;

        // The pan of this channel.
        hmm_vec2 _pan;

        // The location of this channel's sound.
        hmm_vec3 _location;

        AmUInt64 _channelStateId;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_CHANNEL_INTERNAL_STATE_H
