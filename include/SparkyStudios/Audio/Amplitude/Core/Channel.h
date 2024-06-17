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

#ifndef SS_AMPLITUDE_AUDIO_CHANNEL_H
#define SS_AMPLITUDE_AUDIO_CHANNEL_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    class ChannelInternalState;

    enum class ChannelPlaybackState : AmUInt8
    {
        Stopped = 0,
        Playing = 1,
        FadingIn = 2,
        FadingOut = 3,
        SwitchingState = 4,
        Paused = 5,
    };

    enum class ChannelEvent : AmUInt8
    {
        Begin = 0,
        End = 1,
        Resume = 2,
        Pause = 3,
        Stop = 4,
        Loop = 5
    };

    /**
     * @brief An object that represents a single channel of audio.
     *
     * The Channel class is a lightweight reference to a ChannelInternalState object
     * which is managed by the Engine. Multiple Channel objects may point to the same
     * underlying data.
     */
    class AM_API_PUBLIC Channel
    {
    public:
        /**
         * @brief Construct an uninitialized Channel.
         *
         * An uninitialized Channel cannot have its data set or queried.
         * To initialize the Channel, use <code>Engine::PlaySound();</code>.
         */
        Channel();

        explicit Channel(ChannelInternalState* state);

        Channel(const Channel& other);

        /**
         * @brief Uninitializes this Channel.
         *
         * Note that this does not stop the audio or destroy the internal
         * state it references, it just removes this reference to it.
         * To stop the Channel, use <code>Channel::Stop();</code>.
         */
        void Clear();

        /**
         * @brief Checks whether this Channel has been initialized.
         *
         * @return true if this Channel has been initialized.
         */
        [[nodiscard]] bool Valid() const;

        /**
         * @brief Checks if the sound associated to this @c Channel is playing.
         *
         * @return Whether the channel is currently playing.
         */
        [[nodiscard]] bool Playing() const;

        /**
         * @brief Stops a channel.
         *
         * Stops this channel from playing. A sound will stop on its own if its not set
         * to loop. Looped audio must be explicitly stopped.
         *
         * @param duration The fade out duration before to stop the channel.
         */
        void Stop(AmTime duration = kMinFadeDuration) const;

        /**
         * @brief Pauses a channel.
         *
         * Pauses this channel. A paused channel may be resumed where it left off.
         *
         * @param duration The fade out duration before to pause the channel.
         */
        void Pause(AmTime duration = kMinFadeDuration) const;

        /**
         * @brief Resumes a paused channel.
         *
         * Resumes this channel. If this channel was paused it will continue where it
         * left off.
         *
         * @param duration The fade in duration after resuming the channel.
         */
        void Resume(AmTime duration = kMinFadeDuration) const;

        /**
         * @brief Gets the location of this Channel.
         *
         * If the audio on this channel is not set to be Positional, this method will
         * return an invalid location.
         *
         * @return The location of this Channel.
         */
        [[nodiscard]] const AmVec3& GetLocation() const;

        /**
         * @brief Sets the location of this Channel.
         *
         * If the audio on this channel is not set to be Positional, this method
         * does nothing.
         *
         * @param location The new location of the Channel.
         */
        void SetLocation(const AmVec3& location) const;

        /**
         * @brief Sets the gain on this Channel.
         *
         * @param gain The new gain value.
         */
        void SetGain(AmReal32 gain) const;

        /**
         * @brief Returns the gain on this Channel.
         *
         * @return Returns the gain.
         */
        [[nodiscard]] AmReal32 GetGain() const;

        /**
         * @brief Returns the playback state of this Channel.
         */
        [[nodiscard]] ChannelPlaybackState GetPlaybackState() const;

        /**
         * @brief Returns the internal state of this Channel.
         *
         * @return The internal state of this Channel.
         */
        [[nodiscard]] ChannelInternalState* GetState() const;

    private:
        Channel(ChannelInternalState* state, AmUInt64 id);

        [[nodiscard]] bool IsValidStateId() const;

        ChannelInternalState* _state;

        AmUInt64 _stateId;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_CHANNEL_H
