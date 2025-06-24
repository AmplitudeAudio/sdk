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

#ifndef _AM_CORE_PLAYBACK_CHANNEL_H
#define _AM_CORE_PLAYBACK_CHANNEL_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>
#include <SparkyStudios/Audio/Amplitude/Core/Listener.h>
#include <SparkyStudios/Audio/Amplitude/Core/Playback/ChannelEventListener.h>
#include <SparkyStudios/Audio/Amplitude/Core/Room.h>

namespace SparkyStudios::Audio::Amplitude
{
    class ChannelInternalState;

    /**
     * @brief Lists the playback states for a @c Channel.
     *
     * @ingroup engine
     */
    enum eChannelPlaybackState : AmUInt8
    {
        /**
         * @brief The channel is stopped and not rendering audio.
         */
        eChannelPlaybackState_Stopped = 0,

        /**
         * @brief The channel is playing audio.
         */
        eChannelPlaybackState_Playing = 1,

        /**
         * @brief The channel has just been played or resumed and is fading in to the @c eChannelPlaybackState_Playing state.
         */
        eChannelPlaybackState_FadingIn = 2,

        /**
         * @brief The channel has just been stopped or paused and is fading out to the @c eChannelPlaybackState_Stopped
         * or @c eChannelPlaybackState_Paused state.
         */
        eChannelPlaybackState_FadingOut = 3,

        /**
         * @brief The channel is updating the value of his linked switch state.
         */
        eChannelPlaybackState_SwitchingState = 4,

        /**
         * @brief The channel is playing audio but in a paused state.
         */
        eChannelPlaybackState_Paused = 5,
    };

    /**
     * @brief Lists the events triggered by a @c Channel during playback.
     *
     * @ingroup engine
     */
    enum eChannelEvent : AmUInt8
    {
        /**
         * @brief The playback of the channel has started.
         */
        eChannelEvent_Begin = 0,

        /**
         * @brief The playback of the channel has ended.
         */
        eChannelEvent_End = 1,

        /**
         * @brief The playback of the channel has resumed after being paused.
         */
        eChannelEvent_Resume = 2,

        /**
         * @brief The playback of the channel has been paused.
         */
        eChannelEvent_Pause = 3,

        /**
         * @brief The playback of the channel has been stopped.
         */
        eChannelEvent_Stop = 4,

        /**
         * @brief The playback of the channel has looped.
         */
        eChannelEvent_Loop = 5
    };

    /**
     * @brief An object that represents a single channel of audio.
     *
     * The @c Channel class is a lightweight reference to a @c ChannelInternalState object which is managed internally by the
     * Engine. Multiple channels may point to the same underlying data while maintaining different states.
     *
     * @ingroup engine
     */
    class AM_API_PUBLIC Channel
    {
    public:
        /**
         * @brief Construct an uninitialized channel.
         *
         * @note An uninitialized channel cannot have its data set or queried.
         */
        Channel();

        /**
         * @brief Creates a wrapper instance over the provided state.
         *
         * @param[in] state The internal state to wrap.
         *
         * @warning This constructor is for internal usage only.
         */
        explicit Channel(ChannelInternalState* state);

        /**
         * @brief Uninitializes this channel.
         *
         * @note This does not stop the audio or destroy the internal state it references, it just removes this reference to it.
         */
        void Clear();

        /**
         * @brief Checks whether this channel has been initialized.
         *
         * @return @c true if this channel has been initialized, @c false otherwise.
         */
        [[nodiscard]] bool Valid() const;

        /**
         * @brief Gets the ID of this channel.
         *
         * @return The ID of this channel.
         */
        [[nodiscard]] AmUInt64 GetId() const;

        /**
         * @brief Checks if the sound associated with this channel is playing.
         *
         * @return @c true if the channel is currently playing, @c false otherwise.
         */
        [[nodiscard]] bool Playing() const;

        /**
         * @brief Stops the channel.
         *
         * @note A sound will stop on its own if it's not set to loop. Looped audio must be explicitly stopped.
         *
         * @param[in] duration The fade out duration before to stop the channel.
         */
        void Stop(AmTime duration = kMinFadeDuration) const;

        /**
         * @brief Pauses the channel.
         *
         * @note A paused channel may be resumed where it left off.
         *
         * @param[in] duration The fade out duration before to pause the channel.
         *
         * @see Resume
         */
        void Pause(AmTime duration = kMinFadeDuration) const;

        /**
         * @brief Resumes the channel.
         *
         * If this channel is paused, it will continue where it left off.
         *
         * @param[in] duration The fade-in duration after resuming the channel.
         *
         * @see Pause
         */
        void Resume(AmTime duration = kMinFadeDuration) const;

        /**
         * @brief Gets the location of this channel in the game environment.
         *
         * If the audio on this channel doesn't support positional data, this method will
         * return an invalid location.
         *
         * @return The location of this channel.
         */
        [[nodiscard]] const AmVector3& GetLocation() const;

        /**
         * @brief Sets the location of this channel in the game environment.
         *
         * If the audio on this channel doesn't support positional data, this method
         * does nothing.
         *
         * @param[in] location The new location of the channel.
         */
        void SetLocation(const AmVector3& location) const;

        /**
         * @brief Sets the gain on this channel.
         *
         * @param[in] gain The new gain value.
         */
        void SetGain(AmReal32 gain) const;

        /**
         * @brief Returns the gain on this channel.
         *
         * @return The channel's gain.
         */
        [[nodiscard]] AmReal32 GetGain() const;

        /**
         * @brief Returns the playback state of this channel.
         *
         * @return An @ref eChannelPlaybackState enumeration value representing the current state of the channel.
         */
        [[nodiscard]] eChannelPlaybackState GetPlaybackState() const;

        /**
         * @brief Returns the Entity associated with this channel.
         *
         * @note If no @c Entity is associated with this channel, this method will return an
         * uninitialized @c Entity object. You should check if the entity is valid before using it.
         *
         * @return The entity associated with this channel.
         *
         * @see Entity
         */
        [[nodiscard]] Entity GetEntity() const;

        /**
         * @brief Returns the Listener associated with this channel.
         *
         * @note If no @c Listener is associated with this channel, this method will return an
         * uninitialized @c Listener object. You should check if the listener is valid before using it.
         *
         * @return The listener associated with this channel.
         *
         * @see Listener
         */
        [[nodiscard]] Listener GetListener() const;

        /**
         * @brief Returns the Room associated with this channel.
         *
         * @note If no @c Room is associated with this channel, this method will return an
         * uninitialized @c Room object. You should check if the room is valid before using it.
         *
         * @return The room associated with this channel.
         */
        [[nodiscard]] Room GetRoom() const;

        /**
         * @brief Returns the internal state of this channel.
         *
         * @return The internal state of this channel.
         *
         * @warning This method is for internal usage only.
         */
        [[nodiscard]] ChannelInternalState* GetState() const;

        /**
         * @brief Registers a callback for a channel event.
         *
         * @param[in] event The channel event.
         * @param[in] callback The callback function.
         * @param[in] userData The user data to pass to the callback.
         *
         * @see eChannelEvent
         * @see ChannelEventCallback
         */
        void On(eChannelEvent event, ChannelEventCallback callback, void* userData = nullptr) const;

    private:
        /**
         * @brief Private constructor to wrap an existing state.
         *
         * @internal
         */
        Channel(ChannelInternalState* state, AmUInt64 id);

        /**
         * @brief Checks if the internal state ID is valid.
         *
         * @return @c true if the internal state ID is valid, @c false otherwise.
         *
         * @internal
         */
        [[nodiscard]] bool IsValidStateId() const;

        /**
         * @brief The internal state of the channel.
         *
         * @internal
         */
        ChannelInternalState* _state;

        /**
         * @brief The ID of the channel.
         *
         * @internal
         */
        AmUInt64 _stateId;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_PLAYBACK_CHANNEL_H
