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

#ifndef _AM_IMPLEMENTATION_CORE_PLAYBACK_CHANNEL_INSTANCE_INTERNAL_STATE_H
#define _AM_IMPLEMENTATION_CORE_PLAYBACK_CHANNEL_INSTANCE_INTERNAL_STATE_H

#include <map>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Playback/ChannelInstance.h>
#include <SparkyStudios/Audio/Amplitude/Core/Room.h>

#include <Utils/intrusive_list.h>

namespace SparkyStudios::Audio::Amplitude
{
    class ChannelInternalState;

    /**
     * @brief Internal state for a single channel instance (position).
     *
     * This class stores all the per-instance data needed for multi-position
     * sound processing, including position, room association, attenuation
     * weight, and playback cursor for separate mode.
     */
    class ChannelInstanceInternalState
    {
        friend class ChannelInstance;
        friend class ChannelInternalState;

    public:
        /**
         * @brief Creates a new uninitialized channel instance state.
         */
        ChannelInstanceInternalState();

        /**
         * @brief Creates a new channel instance state with a parent channel.
         *
         * @param parentChannel The parent channel that owns this instance.
         */
        explicit ChannelInstanceInternalState(ChannelInternalState* parentChannel);

        /**
         * @brief Resets this instance to its initial state.
         */
        void Reset();

        /**
         * @brief Gets the unique ID of this instance.
         *
         * @return The instance ID.
         */
        [[nodiscard]] AM_INLINE AmChannelInstanceID GetId() const
        {
            return _instanceId;
        }

        /**
         * @brief Sets the unique ID of this instance.
         *
         * @param id The instance ID.
         */
        AM_INLINE void SetId(AmChannelInstanceID id)
        {
            _instanceId = id;
        }

        /**
         * @brief Gets the world location of this instance.
         *
         * @return The world-space position.
         */
        [[nodiscard]] AM_INLINE const AmVector3& GetLocation() const
        {
            return _location;
        }

        /**
         * @brief Sets the world location of this instance.
         *
         * @param location The new world-space position.
         */
        AM_INLINE void SetLocation(const AmVector3& location)
        {
            _previousLocation = _location;
            _location = location;
        }

        /**
         * @brief Gets the previous frame's location.
         *
         * @return The previous world-space position.
         */
        [[nodiscard]] AM_INLINE const AmVector3& GetPreviousLocation() const
        {
            return _previousLocation;
        }

        /**
         * @brief Gets the room associated with this instance.
         *
         * @return The associated room.
         */
        [[nodiscard]] AM_INLINE const Room& GetRoom() const
        {
            return _room;
        }

        /**
         * @brief Sets the room associated with this instance.
         *
         * @param room The room to associate.
         */
        void SetRoom(const Room& room);

        /**
         * @brief Gets the attenuation weight for blended mode.
         *
         * @return The weight value.
         */
        [[nodiscard]] AM_INLINE AmReal32 GetWeight() const
        {
            return _weight;
        }

        /**
         * @brief Sets the attenuation weight for blended mode.
         *
         * @param weight The weight (clamped to be non-negative).
         */
        AM_INLINE void SetWeight(AmReal32 weight)
        {
            _weight = AM_MAX(weight, 0.0f);
        }

        /**
         * @brief Gets the parent channel that owns this instance.
         *
         * @return The parent channel.
         */
        [[nodiscard]] AM_INLINE ChannelInternalState* GetParentChannel() const
        {
            return _parentChannel;
        }

        /**
         * @brief Gets the playback cursor for this instance (separate mode).
         *
         * @return The cursor position in frames.
         */
        [[nodiscard]] AM_INLINE AmUInt64 GetCursor() const
        {
            return _cursor;
        }

        /**
         * @brief Sets the playback cursor for this instance (separate mode).
         *
         * @param cursor The cursor position in frames.
         */
        AM_INLINE void SetCursor(AmUInt64 cursor)
        {
            _cursor = cursor;
        }

        /**
         * @brief Advances the playback cursor by the given number of frames.
         *
         * @param frames The number of frames to advance.
         * @param soundLength The total length of the sound in frames.
         * @param loop Whether the sound should loop.
         */
        void AdvanceCursor(AmUInt64 frames, AmUInt64 soundLength, bool loop);

        /**
         * @brief Gets the loop count for this instance (separate mode).
         *
         * @return The number of times the sound has looped.
         */
        [[nodiscard]] AM_INLINE AmUInt32 GetLoopCount() const
        {
            return _loopCount;
        }

        /**
         * @brief Checks if this instance has finished playing (separate mode).
         *
         * @return @c true if playback has finished, @c false otherwise.
         */
        [[nodiscard]] AM_INLINE bool IsFinished() const
        {
            return _finished;
        }

        /**
         * @brief Gets the pre-computed attenuation gain for this instance.
         *
         * @return The computed gain value.
         */
        [[nodiscard]] AM_INLINE AmReal32 GetComputedGain() const
        {
            return _computedGain;
        }

        /**
         * @brief Sets the pre-computed attenuation gain for this instance.
         *
         * @param gain The computed gain value.
         */
        AM_INLINE void SetComputedGain(AmReal32 gain)
        {
            _computedGain = gain;
        }

        /**
         * @brief Gets the pre-computed stereo panned gain for this instance.
         *
         * @return The panned gain (left, right).
         */
        [[nodiscard]] AM_INLINE const AmVector2& GetPannedGain() const
        {
            return _pannedGain;
        }

        /**
         * @brief Sets the pre-computed stereo panned gain for this instance.
         *
         * @param gain The panned gain (left, right).
         */
        AM_INLINE void SetPannedGain(const AmVector2& gain)
        {
            _pannedGain = gain;
        }

        /**
         * @brief Gets the Doppler factor for a specific listener.
         *
         * @param listener The listener ID.
         *
         * @return The Doppler factor (1.0 = no shift).
         */
        [[nodiscard]] AmReal32 GetDopplerFactor(AmListenerID listener) const;

        /**
         * @brief Sets the Doppler factor for a specific listener.
         *
         * @param listener The listener ID.
         * @param factor The Doppler factor.
         */
        void SetDopplerFactor(AmListenerID listener, AmReal32 factor);

        /**
         * @brief Intrusive list node for parent channel's instance list.
         */
        fplutil::intrusive_list_node instance_node;

    private:
        AmChannelInstanceID _instanceId;
        ChannelInternalState* _parentChannel;

        // Position
        AmVector3 _location;
        AmVector3 _previousLocation;

        // Environment
        Room _room;

        // Blended mode weight
        AmReal32 _weight;

        // Separate mode: independent cursor
        AmUInt64 _cursor;
        AmUInt32 _loopCount;
        bool _finished;

        // Cached computed values for pipeline
        AmReal32 _computedGain;
        AmVector2 _pannedGain;

        // Per-listener Doppler factors
        std::map<AmListenerID, AmReal32> _dopplerFactors;
    };

    typedef fplutil::intrusive_list<ChannelInstanceInternalState> ChannelInstanceList;
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_CORE_PLAYBACK_CHANNEL_INSTANCE_INTERNAL_STATE_H
