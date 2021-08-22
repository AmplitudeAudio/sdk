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

#ifndef SPARK_AUDIO_CHANNEL_INTERNAL_STATE_H
#define SPARK_AUDIO_CHANNEL_INTERNAL_STATE_H

#include <utility>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Core/Channel.h>
#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>

#include "RealChannel.h"

#include <Utils/intrusive_list.h>

namespace SparkyStudios::Audio::Amplitude
{
    enum ChannelState
    {
        ChannelStateStopped,
        ChannelStatePlaying,
        ChannelStateFadingOut,
        ChannelStatePaused,
    };

    // Represents a sample that is playing on a channel.
    class ChannelInternalState
    {
    public:
        ChannelInternalState()
            : _realChannel(this)
            , _channelState(ChannelStateStopped)
            , _collection(nullptr)
            , _sound(nullptr)
            , _location()
            , _userGain(1.0f)
            , _gain(1.0f)
        {}

        // Updates the state enum based on whether this channel is stopped, playing,
        // etc.
        void UpdateState();

        // Returns true if this channel holds streaming data.
        [[nodiscard]] bool IsStream() const;

        // Remove this channel from all lists that it is a part of.
        void Remove();

        // Get or set the sound collection playing on this channel. Note that when you set
        // the sound collection, you also add this channel to the GetBus list that
        // corresponds to that sound collection.
        void SetSoundCollection(SoundCollection* collection);

        [[nodiscard]] SoundCollection* GetSoundCollection() const
        {
            return _collection;
        }

        void SetEntity(const Entity& entity);

        Entity& GetEntity()
        {
            return _entity;
        }

        [[nodiscard]] const Entity& GetEntity() const
        {
            return _entity;
        }

        // Get the current state of this channel (playing, stopped, paused, etc). This
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

        // Set and query the user GetGain of this channel.
        void SetUserGain(const float user_gain)
        {
            _userGain = user_gain;
        }

        [[nodiscard]] float GetUserGain() const
        {
            return _userGain;
        }

        // Set and query the current GetGain of this channel.
        void SetGain(const float gain)
        {
            _gain = gain;
        }

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

        // Fade out over the specified number of milliseconds.
        void FadeOut(AmInt32 milliseconds);

        // Sets the pan based on a position in a unit circle.
        void SetPan(const hmm_vec2& pan);

        // Devirtualizes a virtual channel. This transfers ownership of the given
        // channel's channel_id to this channel.
        void Devirtualize(ChannelInternalState* other);

        // Returns the priority of this channel based on its GetGain and priority
        // multiplier on the sound collection definition.
        [[nodiscard]] float Priority() const;

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

        // The node that tracks the location in the priority list.
        fplutil::intrusive_list_node priority_node;

        // The node that tracks the location in the free list.
        fplutil::intrusive_list_node free_node;

        // The node that tracks the list of sounds playing on a given bus.
        fplutil::intrusive_list_node bus_node;

        // The node that tracks the list of sounds playing on a given entity.
        fplutil::intrusive_list_node entity_node;

    private:
        RealChannel _realChannel;

        // Whether this channel is currently playing, stopped, fading out, etc.
        ChannelState _channelState;

        // The collection of the sound being played on this channel.
        SoundCollection* _collection;

        // The entity which is playing the sound of this channel.
        Entity _entity;

        // The sound source that was chosen from the sound collection.
        Sound* _sound;

        // The GetGain set by the user.
        float _userGain;

        // The GetGain of this channel.
        float _gain;

        // The location of this channel's sound.
        hmm_vec3 _location;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_CHANNEL_INTERNAL_STATE_H
