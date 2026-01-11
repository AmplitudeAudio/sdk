// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#ifndef _AM_CORE_PLAYBACK_CHANNEL_INSTANCE_H
#define _AM_CORE_PLAYBACK_CHANNEL_INSTANCE_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Room.h>

namespace SparkyStudios::Audio::Amplitude
{
    class ChannelInstanceInternalState;

    /**
     * @brief Defines how channel instances share audio data.
     *
     * This determines the playback behavior when a channel has multiple position instances.
     *
     * @ingroup engine
     */
    enum eChannelInstanceMode : AmUInt8
    {
        /**
         * @brief All instances share the same playback cursor.
         *
         * Sound is perceived from multiple points with weighted-blend attenuation.
         * Use this mode for continuous, ambient sounds that should be perceived as
         * coming from multiple locations simultaneously.
         *
         * @par Example
         * A river heard from multiple points along its path.
         */
        eChannelInstanceMode_Blended = 0,

        /**
         * @brief Instances share decoded audio data but have independent cursors.
         *
         * Each instance plays the same sound but with independent timing.
         * Use this mode for sounds that should play independently at different locations,
         * with natural variation in their timing.
         *
         * @par Example
         * Multiple torches with the same fire sound but unsynchronized.
         */
        eChannelInstanceMode_Separate = 1,
    };

    /**
     * @brief Represents a single position instance within a multi-position channel.
     *
     * A @c ChannelInstance stores per-position spatialization data including world location,
     * per-instance room/environment associations, and instance-specific attenuation weights.
     *
     * Channel instances enable a single sound channel to emit from multiple world positions
     * simultaneously, similar to instanced rendering in graphics where one mesh is rendered
     * at multiple transforms.
     *
     * @note The @c ChannelInstance class is a lightweight wrapper around the internal
     * @c ChannelInstanceInternalState class.
     *
     * @see Channel::AddInstance
     * @see eChannelInstanceMode
     *
     * @ingroup engine
     */
    class AM_API_PUBLIC ChannelInstance
    {
    public:
        /**
         * @brief Creates an uninitialized channel instance.
         *
         * @note An uninitialized channel instance cannot have its data set or queried.
         */
        ChannelInstance();

        /**
         * @brief Creates a wrapper instance over the provided state.
         *
         * @param[in] state The internal state to wrap.
         *
         * @warning This constructor is for internal usage only.
         */
        explicit ChannelInstance(ChannelInstanceInternalState* state);

        /**
         * @brief Uninitializes this channel instance.
         *
         * @note This does not remove the instance from its parent channel, it just
         * removes this reference to the internal state.
         *
         * @note To completely remove the instance, use the @c Channel::RemoveInstance method.
         */
        void Clear();

        /**
         * @brief Checks whether this channel instance has been initialized.
         *
         * @return @c true if this instance has been initialized with a valid state, @c false otherwise.
         */
        [[nodiscard]] bool Valid() const;

        /**
         * @brief Gets the unique ID of this instance within its parent channel.
         *
         * @return The instance's unique ID.
         */
        [[nodiscard]] AmChannelInstanceID GetId() const;

        /**
         * @brief Gets the world location of this instance.
         *
         * @return The world-space position of this instance.
         */
        [[nodiscard]] const AmVector3& GetLocation() const;

        /**
         * @brief Sets the world location of this instance.
         *
         * @param[in] location The new world-space position.
         */
        void SetLocation(const AmVector3& location) const;

        /**
         * @brief Gets the room associated with this instance.
         *
         * The room is used for per-instance environmental effects such as
         * early reflections and reverberation.
         *
         * @return The room associated with this instance, or an uninitialized Room if none.
         */
        [[nodiscard]] Room GetRoom() const;

        /**
         * @brief Sets the room associated with this instance.
         *
         * Setting a room enables per-instance environmental effects. Each instance
         * can be in a different room, allowing for spatially-accurate reverb and reflections.
         *
         * @param[in] room The room to associate with this instance.
         */
        void SetRoom(const Room& room) const;

        /**
         * @brief Gets the attenuation weight for blended mode.
         *
         * The weight determines how much this instance contributes to the final
         * blended output. Higher weights result in more contribution from this instance.
         *
         * @note This value is primarily used in @c eChannelInstanceMode_Blended mode.
         * In @c eChannelInstanceMode_Separate mode, each instance plays independently.
         *
         * @return The weight value (default is 1.0).
         */
        [[nodiscard]] AmReal32 GetWeight() const;

        /**
         * @brief Sets the attenuation weight for blended mode.
         *
         * @param[in] weight The weight (0.0 to any positive value). Values are clamped to be non-negative.
         * Higher values result in more contribution from this instance.
         *
         * @note This value is primarily used in @c eChannelInstanceMode_Blended mode.
         */
        void SetWeight(AmReal32 weight) const;

        /**
         * @brief Gets the internal state of the channel instance.
         *
         * @return The internal state.
         *
         * @warning This method is for internal usage only.
         */
        [[nodiscard]] ChannelInstanceInternalState* GetState() const;

    private:
        /**
         * @brief The internal state of the channel instance.
         *
         * @internal
         */
        ChannelInstanceInternalState* _state;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_PLAYBACK_CHANNEL_INSTANCE_H
