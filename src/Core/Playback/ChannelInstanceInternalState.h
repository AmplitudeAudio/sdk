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

#include <atomic>
#include <unordered_map>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Playback/ChannelInstance.h>
#include <SparkyStudios/Audio/Amplitude/Core/Room.h>

#include <Utils/intrusive_list.h>

namespace SparkyStudios::Audio::Amplitude
{
    class ChannelInternalState;

    /**
     * @brief Immutable snapshot of a single channel instance, for audio-thread consumption.
     *
     * The game thread owns @c ChannelInstanceInternalState objects and publishes
     * snapshots of them; the audio thread only ever reads these plain-data copies.
     */
    struct ChannelInstanceData
    {
        AmChannelInstanceID instanceId;
        AmVector3 location;
        Room room;
        AmReal32 weight;
        AmReal32 computedGain;
        AmUInt64 cursor; ///< Initial cursor, used when a layer (re)discovers the instance (separate mode).
        Orientation orientation = Orientation::Zero(); ///< Explicit orientation, meaningful only when @c hasOrientation.
        bool hasOrientation = false; ///< Whether the instance has an explicit orientation, else it follows the entity's.
        AmReal32 obstruction = 0.0f; ///< Explicit obstruction, meaningful only when @c hasObstruction.
        bool hasObstruction = false; ///< Whether the instance has an explicit obstruction, else it follows the entity's.
        AmReal32 occlusion = 0.0f; ///< Explicit occlusion, meaningful only when @c hasOcclusion.
        bool hasOcclusion = false; ///< Whether the instance has an explicit occlusion, else it follows the entity's.
    };

    /**
     * @brief Cursor write-back slot for separate-mode playback.
     *
     * The audio thread writes the latest cursor, the game thread drains it into the
     * authoritative state at publish time. The @c id is written by the game thread at
     * publish time and checked by the audio thread before storing, so a store is never
     * applied to an instance whose id doesn't match. A store that passed the check but
     * races a re-pair may be drained into the slot's new owner or dropped; both are
     * bounded and harmless for cursor write-back.
     */
    struct ChannelInstanceCursorSlot
    {
        std::atomic<AmChannelInstanceID> id;
        std::atomic<AmUInt64> cursor;
    };

    /**
     * @brief Immutable snapshot of a channel's instancing state, for audio-thread consumption.
     *
     * Everything the audio thread needs to know about a channel's instances comes from here, never from the
     * game-owned containers or fields of the channel: whether it is instanced, in which mode, the instances, and
     * where to write the cursors back.
     */
    struct ChannelInstanceSnapshot
    {
        bool instancingEnabled = false;
        eChannelInstanceMode instancingMode = eChannelInstanceMode_Blended;

        /// The channel's cursor write-back slots. They live as long as the channel, so the pointer stays valid.
        ChannelInstanceCursorSlot* cursorSlots = nullptr;

        std::vector<ChannelInstanceData> instances;
    };

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
         * @brief Gets the generation of this instance state.
         *
         * The generation changes every time the state is recycled, allowing
         * @c ChannelInstance handles to detect staleness after pool reuse.
         *
         * @return The generation value.
         */
        [[nodiscard]] AM_INLINE AmUInt64 GetGeneration() const
        {
            return _generation;
        }

        /**
         * @brief Sets the generation of this instance state.
         *
         * @param generation The generation value.
         */
        AM_INLINE void SetGeneration(AmUInt64 generation)
        {
            _generation = generation;
        }

        /**
         * @brief Invalidates this state before it is returned to the memory pool.
         *
         * Must be called before deleting the state so that stale handles observe
         * a mismatched id/generation instead of reading recycled values.
         */
        void Invalidate();

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
         * Marks the parent channel's instance snapshot stale. It is republished once per frame,
         * from the channel's @c AdvanceFrame.
         *
         * @param location The new world-space position.
         */
        void SetLocation(const AmVector3& location);

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
         * @brief Gets the orientation used to render this instance.
         *
         * @return The explicit orientation when one was set, else the one of the parent channel's entity,
         * else a zero orientation.
         */
        [[nodiscard]] const Orientation& GetOrientation() const;

        /**
         * @brief Sets an explicit orientation for this instance.
         *
         * Marks the parent channel's instance snapshot stale. It is republished once per frame,
         * from the channel's @c AdvanceFrame.
         *
         * @param orientation The new world-space orientation.
         */
        void SetOrientation(const Orientation& orientation);

        /**
         * @brief Drops the explicit orientation, so the instance follows the parent channel's entity again.
         */
        void ResetOrientation();

        /**
         * @brief Checks whether an explicit orientation was set on this instance.
         */
        [[nodiscard]] AM_INLINE bool HasOrientation() const
        {
            return _hasOrientation;
        }

        /**
         * @brief Gets the obstruction applied to this instance.
         *
         * @return The explicit obstruction when one was set, else the one of the parent channel's entity, else 0.
         */
        [[nodiscard]] AmReal32 GetObstruction() const;

        /**
         * @brief Sets an explicit obstruction for this instance.
         *
         * Marks the parent channel's instance snapshot stale. It is republished once per frame,
         * from the channel's @c AdvanceFrame.
         *
         * @param obstruction The obstruction amount.
         */
        void SetObstruction(AmReal32 obstruction);

        /**
         * @brief Drops the explicit obstruction, so the instance follows the parent channel's entity again.
         */
        void ResetObstruction();

        /**
         * @brief Checks whether an explicit obstruction was set on this instance.
         */
        [[nodiscard]] AM_INLINE bool HasObstruction() const
        {
            return _hasObstruction;
        }

        /**
         * @brief Gets the occlusion applied to this instance.
         *
         * @return The explicit occlusion when one was set, else the one of the parent channel's entity, else 0.
         */
        [[nodiscard]] AmReal32 GetOcclusion() const;

        /**
         * @brief Sets an explicit occlusion for this instance.
         *
         * Marks the parent channel's instance snapshot stale. It is republished once per frame,
         * from the channel's @c AdvanceFrame.
         *
         * @param occlusion The occlusion amount.
         */
        void SetOcclusion(AmReal32 occlusion);

        /**
         * @brief Drops the explicit occlusion, so the instance follows the parent channel's entity again.
         */
        void ResetOcclusion();

        /**
         * @brief Checks whether an explicit occlusion was set on this instance.
         */
        [[nodiscard]] AM_INLINE bool HasOcclusion() const
        {
            return _hasOcclusion;
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
         * Marks the parent channel's instance snapshot stale. It is republished once per frame,
         * from the channel's @c AdvanceFrame.
         *
         * @param room The room to associate.
         */
        void SetRoom(const Room& room);

        /**
         * @brief Assigns the room of this instance without marking it as set by the game.
         *
         * The Engine uses this to give this instance the room it is in, or the room of its entity when
         * the game is tracking rooms. Unlike @c SetRoom, an assigned room is replaced on the next frame
         * and does not count as a room of its own.
         *
         * @param room The room to associate.
         */
        void AssignRoom(const Room& room);

        /**
         * @brief Checks whether this instance has a room of its own.
         *
         * @return @c true if a room was set on this instance through @c SetRoom, @c false otherwise.
         */
        [[nodiscard]] AM_INLINE bool HasRoom() const
        {
            return _hasRoom;
        }

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
         * Marks the parent channel's instance snapshot stale. It is republished once per frame,
         * from the channel's @c AdvanceFrame.
         *
         * @param weight The weight (clamped to be non-negative).
         */
        void SetWeight(AmReal32 weight);

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
        AmUInt64 _generation = 0;
        ChannelInternalState* _parentChannel;

        // Position
        AmVector3 _location;
        AmVector3 _previousLocation;

        // Explicit orientation, meaningful only when _hasOrientation is set
        Orientation _orientation;
        bool _hasOrientation;

        // Explicit obstruction and occlusion, meaningful only when their flag is set
        AmReal32 _obstruction;
        bool _hasObstruction;
        AmReal32 _occlusion;
        bool _hasOcclusion;

        // Environment
        Room _room;
        bool _hasRoom;

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
        std::unordered_map<AmListenerID, AmReal32> _dopplerFactors;
    };

    typedef fplutil::intrusive_list<ChannelInstanceInternalState> ChannelInstanceList;
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_CORE_PLAYBACK_CHANNEL_INSTANCE_INTERNAL_STATE_H
