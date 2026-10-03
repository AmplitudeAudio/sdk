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

#ifndef _AM_IMPLEMENTATION_CORE_PLAYBACK_CHANNEL_INTERNAL_STATE_H
#define _AM_IMPLEMENTATION_CORE_PLAYBACK_CHANNEL_INTERNAL_STATE_H

#include <array>
#include <atomic>
#include <map>
#include <memory>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>
#include <SparkyStudios/Audio/Amplitude/Core/Playback/Channel.h>
#include <SparkyStudios/Audio/Amplitude/Core/Playback/ChannelEventListener.h>
#include <SparkyStudios/Audio/Amplitude/Core/Playback/ChannelInstance.h>

#include <Core/Playback/ChannelInstanceInternalState.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Collection.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Switch.h>
#include <SparkyStudios/Audio/Amplitude/Sound/SwitchContainer.h>

#include <Mixer/RealChannel.h>
#include <Mixer/Voice/VirtualCursor.h>
#include <Mixer/Voice/VoiceTypes.h>
#include <Utils/intrusive_list.h>

namespace SparkyStudios::Audio::Amplitude
{
    class SwitchImpl;
    class SoundImpl;
    class CollectionImpl;
    class SwitchContainerImpl;

    typedef fplutil::intrusive_list<class ChannelInternalState> ChannelList;

    // Represents a sample that is playing on a channel.
    class ChannelInternalState
    {
    public:
        ChannelInternalState()
            : _realChannel(this)
            , _channelState(eChannelPlaybackState_Stopped)
            , _switchContainer(nullptr)
            , _collection(nullptr)
            , _sound(nullptr)
            , _playingSwitchContainerStateId(kAmInvalidObjectId)
            , _previousSwitchContainerStateId(kAmInvalidObjectId)
            , _fader(nullptr)
            , _targetFadeOutState(eChannelPlaybackState_Stopped)
            , _fadeInEndTime(0.0)
            , _stopEventPending(false)
            , _stopFired(false)
            , _pendingEventCount(0)
            , _deferredEvents(0)
            , _virtualPaused(false)
            , _entity()
            , _userGain(1.0f)
            , _gain(1.0f)
            , _realGain(1.0f)
            , _pitch(1.0f)
            , _cachedPriority(0.0f)
            , _priorityDirty(true)
            , _location()
            , _channelStateId(kAmInvalidObjectId)
            , _dopplerFactors()
            , _instancingEnabled(false)
            , _instancingMode(eChannelInstanceMode_Blended)
            , _nextInstanceId(1)
            , _instances(&ChannelInstanceInternalState::instance_node)
            , _instancesMap()
            , _instanceSnapshotState()
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
        void SetSwitchContainer(SwitchContainerImpl* switchContainer);

        // Get or set the collection playing on this channel. Note that when you set
        // the collection, you also add this channel to the bus list that
        // corresponds to that collection.
        void SetCollection(CollectionImpl* collection);

        // Get or set the sound playing on this channel. Note that when you set
        // the sound, you also add this channel to the bus list that
        // corresponds to that sound.
        void SetSound(SoundImpl* sound);

        [[nodiscard]] AM_INLINE SwitchContainerImpl* GetSwitchContainer() const
        {
            return _switchContainer;
        }

        [[nodiscard]] AM_INLINE CollectionImpl* GetCollection() const
        {
            return _collection;
        }

        [[nodiscard]] AM_INLINE SoundImpl* GetSound() const
        {
            return _sound;
        }

        void SetEntity(const Entity& entity);

        AM_INLINE Entity& GetEntity()
        {
            return _entity;
        }

        [[nodiscard]] AM_INLINE const Entity& GetEntity() const
        {
            return _entity;
        }

        void SetListener(const Listener& listener);

        AM_INLINE Listener& GetListener()
        {
            return _activeListener;
        }

        [[nodiscard]] AM_INLINE const Listener& GetListener() const
        {
            return _activeListener;
        }

        void SetRoom(const Room& room);

        AM_INLINE Room& GetRoom()
        {
            return _room;
        }

        [[nodiscard]] AM_INLINE const Room& GetRoom() const
        {
            return _room;
        }

        // Get the current state of this channel (playing, stopped, paused, etc.). This
        // is tracked manually because not all ChannelInternalStates are backed by
        // real channels.
        [[nodiscard]] AM_INLINE eChannelPlaybackState GetChannelState() const
        {
            return _channelState;
        }

        AM_INLINE eChannelPlaybackState SetChannelState(eChannelPlaybackState state)
        {
            return _channelState = state;
        }

        // Get or set the location of this channel
        AM_INLINE void SetLocation(const AmVector3& location)
        {
            // Entity scoped channel
            if (_entity.Valid())
                return;

            // World scoped channel
            _location = location;
        }

        [[nodiscard]] AM_INLINE const AmVector3& GetLocation() const
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
        AM_INLINE void SetUserGain(const AmReal32 user_gain)
        {
            _userGain = user_gain;
        }

        [[nodiscard]] AM_INLINE AmReal32 GetUserGain() const
        {
            return _userGain;
        }

        // Set and query the current gain of this channel.
        void SetGain(AmReal32 gain);

        [[nodiscard]] AM_INLINE AmReal32 GetGain() const
        {
            return _gain;
        }

        void SetPitch(AmReal32 pitch);

        [[nodiscard]] AmReal32 GetPitch() const;

        // Immediately stop the audio. May cause clicking.
        void Halt();

        // Pauses this channel.
        void Pause();

        // Resumes this channel if it is paused.
        void Resume();

        // Sets this channel's playback position, in milliseconds.
        bool SetPlaybackPosition(AmTime position, AmUInt64 clock = kVoiceAsap);

        // Gets this channel's current playback position, in milliseconds.
        [[nodiscard]] AmTime GetPlaybackPosition() const;

        // Fade in over the specified number of milliseconds.
        void FadeIn(AmTime duration, AmUInt64 clock = kVoiceAsap);

        // Fade out over the specified number of milliseconds.
        void FadeOut(
            AmTime duration, eChannelPlaybackState targetState = eChannelPlaybackState_Stopped, AmUInt64 clock = kVoiceAsap);

        // Schedules the first sample of a pending channel at a frame of the audio clock.
        bool ScheduleStart(AmUInt64 clock);

        // Devirtualizes a virtual channel. This transfers ownership of the given
        // channel's channel_id to this channel.
        void Devirtualize(ChannelInternalState* other);

        /**
         * @brief Fades this channel's voices out for virtualization (@c kStealFade) and anchors its virtual cursor,
         * so it can keep being tracked as virtual once its real channel is taken away.
         *
         * Called on the channel losing its real channel, before the real channel id is handed to another.
         */
        void Demote();

        /**
         * @brief Restarts this channel on its (freshly assigned) real channel, at the virtual cursor if one is
         * anchored; falls back to a full @c Play() for collections, switch containers, and sounds that were never
         * anchored.
         *
         * @return @c true on success.
         */
        bool Promote(AmTime fadeIn = kStealFade);

        // Resumes a channel that was paused while virtual, from the position it froze at.
        bool ResumeFrozen(AmTime fadeIn);

        /**
         * @brief Anchors this channel's virtual cursor: @p position is the source frame heard at audio-clock frame
         * @p clock.
         *
         * A no-op for channels not playing a single @c Sound (collections, switch containers) or whose sound format
         * is not yet known.
         */
        void AnchorVirtualCursor(AmUInt64 position, AmUInt64 clock);

        // Returns the priority of this channel based on its gain and priority
        // multiplier on the sound collection definition.
        [[nodiscard]] AmReal32 Priority() const;

        /**
         * @brief Checks if the priority cache is dirty.
         * @return true if priority needs recalculation.
         */
        [[nodiscard]] AM_INLINE bool IsPriorityDirty() const
        {
            return _priorityDirty;
        }

        /**
         * @brief Update this channel data per frames.
         *
         * @param deltaTime The time elapsed since the last frame.
         */
        void AdvanceFrame(AmTime deltaTime);

        // Returns the real channel.
        AM_INLINE RealChannel& GetRealChannel()
        {
            return _realChannel;
        }

        [[nodiscard]] AM_INLINE const RealChannel& GetRealChannel() const
        {
            return _realChannel;
        }

        [[nodiscard]] AM_INLINE bool Valid() const
        {
            return IsAlive() && IsReal();
        }

        [[nodiscard]] AM_INLINE bool IsAlive() const
        {
            return _sound != nullptr || _collection != nullptr || _switchContainer != nullptr;
        }

        // Returns true if the real channel is valid.
        [[nodiscard]] AM_INLINE bool IsReal() const
        {
            return _realChannel.Valid();
        }

        [[nodiscard]] AmObjectID GetPlayingObjectId() const;

        [[nodiscard]] AM_INLINE AmUInt32 GetChannelStateId() const
        {
            return _channelStateId;
        }

        AM_INLINE void SetChannelStateId(AmUInt64 id)
        {
            _channelStateId = id;
        }

        /**
         * @brief Set the obstruction level of sounds played by this Channel.
         *
         * @param obstruction The obstruction amount. This is provided by the
         * game engine.
         */
        void SetObstruction(AmReal32 obstruction);

        /**
         * @brief Set the occlusion level of sounds played by this Channel.
         *
         * @param occlusion The occlusion amount. This is provided by the
         * game engine.
         */
        void SetOcclusion(AmReal32 occlusion);

        /**
         * @brief Get the Doppler factor of this sound for the given Listener.
         *
         * @param listener The listener to get the Doppler factor for.
         *
         * @return A Doppler factor value for the given Listener.
         */
        [[nodiscard]] AmReal32 GetDopplerFactor(AmListenerID listener) const;

        [[nodiscard]] AmReal32 GetRoomGain(AmRoomID room) const;

        AM_INLINE void SetRoomGain(AmRoomID room, AmReal32 gain)
        {
            _roomGains[room] = gain;
        }

        void HaltInternal();

        /**
         * @brief Gets the name of the fader used by this channel's transport fades.
         */
        [[nodiscard]] const AmString& GetFaderName() const;

        /**
         * @brief Called on the game thread when a voice of this channel finished a fade-out.
         *
         * @param mixerLayerId The mixer layer of the voice.
         * @param target What the fade-out ended in.
         * @param frame The audio-clock frame the fade reached zero at.
         * @param sourcePosition The source frame the voice stopped at, for paused and released voices.
         */
        void OnVoiceFadedOut(AmUInt32 mixerLayerId, eVoiceFadeTarget target, AmUInt64 frame, AmUInt64 sourcePosition);

        /**
         * @brief Triggers @p event at the start of the next engine frame, unless this channel was reused in between.
         */
        void TriggerOnNextFrame(eChannelEvent event);

        /**
         * @brief Registers a callback for a channel event.
         *
         * @param event The channel event.
         * @param callback The callback function.
         * @param userData The user data to pass to the callback.
         */
        void On(eChannelEvent event, ChannelEventCallback callback, void* userData = nullptr);

        void Trigger(eChannelEvent event);

        /**
         * @brief Checks whether a @c TriggerOnNextFrame() event is still queued and has not fired yet.
         *
         * A channel with pending events must not be recycled (its state reused by @c Reset()): the deferred
         * callback checks @c GetChannelStateId(), which @c Reset() zeroes, and would silently drop the event.
         */
        [[nodiscard]] AM_INLINE bool HasPendingEvents() const
        {
            return _pendingEventCount > 0 || _deferredEvents != 0;
        }

        /**
         * @brief Checks whether an event could not be queued for the next frame (the engine's callback queue was full)
         * and still waits for @c DrainDeferredEvents().
         */
        [[nodiscard]] AM_INLINE bool HasDeferredEvents() const
        {
            return _deferredEvents != 0;
        }

        /**
         * @brief Fires the events that could not be queued for the next frame. Called by the engine once per frame,
         * outside the channel list iteration.
         */
        void DrainDeferredEvents();

        // Whether this channel is currently fading out toward Stopped: Pause()/Resume()/FadeIn() must do nothing
        // while a stop fade owns the channel.
        [[nodiscard]] bool IsFadingOutToStopped() const;


        /**
         * @brief Enables multi-position instancing for this channel.
         *
         * @param mode The instance mode.
         */
        void EnableInstancing(eChannelInstanceMode mode);

        /**
         * @brief Disables multi-position instancing for this channel.
         */
        void DisableInstancing();

        /**
         * @brief Checks if instancing is enabled for this channel.
         *
         * @return @c true if instancing is enabled, @c false otherwise.
         */
        [[nodiscard]] AM_INLINE bool IsInstancingEnabled() const
        {
            return _instancingEnabled;
        }

        /**
         * @brief Gets the instancing mode.
         *
         * @return The current instancing mode.
         */
        [[nodiscard]] AM_INLINE eChannelInstanceMode GetInstancingMode() const
        {
            return _instancingMode;
        }

        /**
         * @brief Adds a new instance at the specified location.
         *
         * @param location The world-space location for the new instance.
         *
         * @return The internal state of the new instance.
         */
        ChannelInstanceInternalState* AddInstance(const AmVector3& location);

        /**
         * @brief Removes an instance by ID.
         *
         * @param instanceId The ID of the instance to remove.
         */
        void RemoveInstance(AmChannelInstanceID instanceId);

        /**
         * @brief Removes all instances.
         */
        void ClearInstances();

        /**
         * @brief Gets an instance by ID.
         *
         * @param instanceId The ID of the instance to retrieve.
         *
         * @return The internal state of the instance, or nullptr if not found.
         */
        [[nodiscard]] ChannelInstanceInternalState* GetInstance(AmChannelInstanceID instanceId);

        /**
         * @brief Gets an instance by ID (const version).
         *
         * @param instanceId The ID of the instance to retrieve.
         *
         * @return The internal state of the instance, or nullptr if not found.
         */
        [[nodiscard]] const ChannelInstanceInternalState* GetInstance(AmChannelInstanceID instanceId) const;

        /**
         * @brief Gets the number of active instances.
         *
         * @return The number of instances.
         */
        [[nodiscard]] AM_INLINE AmSize GetInstanceCount() const
        {
            return _instances.size();
        }

        /**
         * @brief Gets the list of instances.
         *
         * @return Reference to the instance list.
         */
        [[nodiscard]] AM_INLINE ChannelInstanceList& GetInstances()
        {
            return _instances;
        }

        /**
         * @brief Gets the list of instances (const version).
         *
         * @return Const reference to the instance list.
         */
        [[nodiscard]] AM_INLINE const ChannelInstanceList& GetInstances() const
        {
            return _instances;
        }

        /**
         * @brief Gets the instance map.
         *
         * @return Reference to the instance map.
         */
        [[nodiscard]] AM_INLINE std::unordered_map<AmChannelInstanceID, ChannelInstanceInternalState*>& GetInstancesMap()
        {
            return _instancesMap;
        }

        /**
         * @brief Gets the instance map.
         *
         * @return Const reference to the instance map.
         */
        [[nodiscard]] AM_INLINE const std::unordered_map<AmChannelInstanceID, ChannelInstanceInternalState*>& GetInstancesMap() const
        {
            return _instancesMap;
        }

        /**
         * @brief Publishes an immutable snapshot of the current instances for the audio thread.
         *
         * Rebuilds the back buffer from the authoritative instance list, drains the
         * cursor write-back slots, then atomically swaps the published index.
         *
         * @note Game thread only. Must be called after every mutation of the instance
         * containers or of any instance's published properties.
         */
        void PublishInstanceSnapshot();

        /**
         * @brief Acquires the latest published instance snapshot.
         *
         * @return The published snapshot. Remains valid and immutable until the next
         * publish, which only ever mutates the other buffer.
         *
         * @note Audio thread only.
         */
        [[nodiscard]] const std::vector<ChannelInstanceData>* AcquireInstanceSnapshot() const
        {
            return &_instanceSnapshotState.snapshots[_instanceSnapshotState.publishedSnapshot.load(std::memory_order_acquire)];
        }

        /**
         * @brief Gets the currently published instance snapshot (game thread only).
         */
        [[nodiscard]] const std::vector<ChannelInstanceData>& GetPublishedInstanceSnapshot() const;

        /**
         * @brief Gets the cursor write-back slots, paired by index with the published snapshot.
         *
         * @return The slot array, or @c nullptr when instancing has never been enabled
         * on this channel.
         *
         * @note Audio thread: write only, after checking the slot's @c id matches.
         */
        [[nodiscard]] ChannelInstanceCursorSlot* GetInstanceCursorSlots() const
        {
            return _instanceSnapshotState.cursors.get();
        }

        // The node that tracks the location in the priority list.
        fplutil::intrusive_list_node priority_node;

        // The node that tracks the location in the free list.
        fplutil::intrusive_list_node free_node;

        // The node that tracks the list of sounds playing on a given bus.
        fplutil::intrusive_list_node bus_node;

        // The node that tracks the list of sounds playing on a given entity.
        fplutil::intrusive_list_node entity_node;

        // The node that tracks the list of sounds rendered by a given listener.
        fplutil::intrusive_list_node listener_node;

        // The node that tracks the list of sounds playing in a given room.
        fplutil::intrusive_list_node room_node;

    private:
        bool PlaySwitchContainerStateUpdate(
            const std::vector<SwitchContainerItem>& previous, const std::vector<SwitchContainerItem>& next, AmTime fadeIn = 0.0);

        // Settles the channel to Stopped from UpdateState() and fires the Stop event if it is still owed.
        void SettleStopped();

        // Fires eChannelEvent_Stop at most once per play: several paths can reach it for the same stop.
        void TriggerStopOnce();

        bool PlaySwitchContainer();
        bool PlayCollection();
        bool PlaySound();

        // The real channel feeding the mixer with audio data.
        RealChannel _realChannel;

        // Where this channel's audio would be, tracked while it has no real channel of its own.
        VirtualCursor _virtualCursor;

        // Whether this channel is currently playing, stopped, fading out, etc.
        eChannelPlaybackState _channelState;

        // The collection of the sound being played on this channel.
        SwitchContainerImpl* _switchContainer;

        // The collection of the sound being played on this channel.
        CollectionImpl* _collection;

        // The sound source that was chosen from the sound collection.
        SoundImpl* _sound;

        AmObjectID _playingSwitchContainerStateId;
        AmObjectID _previousSwitchContainerStateId;

        // The sound fader of this channel.
        std::shared_ptr<FaderInstance> _fader;
        AmString _faderName;

        // The target state of the fade out transition. Must be either Paused or Stopped.
        eChannelPlaybackState _targetFadeOutState;

        // The engine time at which the current audio-rate fade-in is expected to be over.
        AmTime _fadeInEndTime;

        // The audio-clock frame the pending channel was scheduled to start at; consumed by the first Play.
        AmUInt64 _scheduledStartFrame = kVoiceAsap;

        // Whether a Stop event is still owed once the voice's fade-out event arrives.
        bool _stopEventPending;

        // Whether eChannelEvent_Stop already fired for the current play. Reset in Play().
        bool _stopFired;

        // TriggerOnNextFrame() events queued but not fired yet.
        AmUInt32 _pendingEventCount;

        // Events (one bit per eChannelEvent) that could not be queued because the engine's callback queue was full.
        // EraseFinishedSounds() drains them, so a full queue never loses a Stop or an End.
        AmUInt32 _deferredEvents;

        // The channel was paused when it lost its real channel: it stays virtual, with a frozen cursor, until it is
        // resumed.
        bool _virtualPaused;

        // The entity which is playing the sound of this channel.
        Entity _entity;

        // The listener currently rendering this channel.
        Listener _activeListener;

        // The room in which this channel is playing.
        Room _room;

        // The gain set by the user.
        AmReal32 _userGain;

        // The gain of this channel.
        AmReal32 _gain;
        AmReal32 _realGain;

        // The pitch of this channel.
        AmReal32 _pitch;

        // Cached priority value to avoid recalculation
        mutable AmReal32 _cachedPriority;
        mutable bool _priorityDirty;

        // The location of this channel's sound.
        AmVector3 _location;

        AmUInt64 _channelStateId;

        std::unordered_map<AmListenerID, AmReal32> _dopplerFactors;
        std::unordered_map<AmRoomID, AmReal32> _roomGains;

        std::map<eChannelEvent, std::shared_ptr<ChannelEventListener>> _eventsMap;

        /**
         * @brief Double-buffered instance snapshot state.
         *
         * Elements are constructed in place by a single resize() at engine init and
         * never actually relocated; the move constructor only exists to satisfy the
         * vector's compile-time move-insertability requirement.
         */
        struct InstanceSnapshotState
        {
            InstanceSnapshotState() = default;
            InstanceSnapshotState(const InstanceSnapshotState&) = delete;
            InstanceSnapshotState& operator=(const InstanceSnapshotState&) = delete;
            InstanceSnapshotState(InstanceSnapshotState&& other) noexcept
                : snapshots(std::move(other.snapshots))
                , publishedSnapshot(other.publishedSnapshot.load(std::memory_order_relaxed))
                , cursors(std::move(other.cursors))
            {}
            InstanceSnapshotState& operator=(InstanceSnapshotState&& other) noexcept
            {
                snapshots = std::move(other.snapshots);
                publishedSnapshot.store(other.publishedSnapshot.load(std::memory_order_relaxed), std::memory_order_relaxed);
                cursors = std::move(other.cursors);
                return *this;
            }

            // Audio-thread instance snapshots (double-buffered). The game thread rebuilds
            // the back buffer and release-stores the swapped index; the audio thread
            // acquire-loads the index and reads the published buffer.
            std::array<std::vector<ChannelInstanceData>, 2> snapshots;
            std::atomic<AmUInt32> publishedSnapshot;

            // Lazily allocated cursor write-back slots (only instanced channels pay for
            // them). Deliberately never freed while the channel is alive: the audio thread
            // may hold the pointer across a callback boundary. Released with the state.
            std::unique_ptr<ChannelInstanceCursorSlot[]> cursors;
        };

        // Multi-position instancing
        bool _instancingEnabled;
        eChannelInstanceMode _instancingMode;
        AmChannelInstanceID _nextInstanceId;
        ChannelInstanceList _instances;
        std::unordered_map<AmChannelInstanceID, ChannelInstanceInternalState*> _instancesMap;

        InstanceSnapshotState _instanceSnapshotState;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_CORE_PLAYBACK_CHANNEL_INTERNAL_STATE_H
