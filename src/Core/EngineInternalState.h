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

#ifndef SPARK_AUDIO_ENGINE_INTERNAL_STATE_H
#define SPARK_AUDIO_ENGINE_INTERNAL_STATE_H

#include <map>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/BusInternalState.h>
#include <Core/ChannelInternalState.h>
#include <Core/EntityInternalState.h>
#include <Core/ListenerInternalState.h>
#include <Utils/intrusive_list.h>

#include "Mixer.h"

#include "sound_collection_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    struct EngineConfigDefinition;
    struct BusDefinitionList;
    struct SoundBankDefinition;

    typedef std::map<std::string, std::unique_ptr<SoundCollection>> SoundCollectionMap;
    typedef std::map<std::string, std::string> SoundIdMap;

    typedef std::map<std::string, std::unique_ptr<Event>> EventMap;
    typedef std::map<std::string, std::string> EventIdMap;
    typedef std::vector<EventInstance> EventInstanceVector;

    typedef std::map<std::string, std::unique_ptr<SoundBank>> SoundBankMap;

    typedef std::vector<ChannelInternalState> ChannelStateVector;

    typedef fplutil::intrusive_list<ChannelInternalState> PriorityList;

    typedef fplutil::intrusive_list<ChannelInternalState> FreeList;

    typedef std::vector<EntityInternalState> EntityStateVector;
    typedef fplutil::intrusive_list<EntityInternalState> EntityList;

    typedef std::vector<ListenerInternalState> ListenerStateVector;
    typedef fplutil::intrusive_list<ListenerInternalState> ListenerList;

    struct EngineInternalState
    {
        explicit EngineInternalState()
            : mixer(this)
            , buses_source()
            , buses()
            , master_bus(nullptr)
            , master_gain(1.0f)
            , mute(true)
            , paused(true)
            , sound_collection_map()
            , sound_id_map()
            , event_map()
            , event_id_map()
            , running_events()
            , sound_bank_map()
            , channel_state_memory()
            , playing_channel_list(&ChannelInternalState::priority_node)
            , real_channel_free_list(&ChannelInternalState::free_node)
            , virtual_channel_free_list(&ChannelInternalState::free_node)
            , listener_list(&ListenerInternalState::node)
            , listener_state_memory()
            , listener_state_free_list()
            , entity_list(&EntityInternalState::node)
            , entity_state_memory()
            , entity_state_free_list()
            , loader()
            , current_frame(0)
            , version(nullptr)
        {}

        Mixer mixer;

        // Hold the audio buses definition file contents.
        std::string buses_source;

        // The state of the buses.
        std::vector<BusInternalState> buses;

        // The master GetBus, cached to prevent needless lookups.
        BusInternalState* master_bus;

        // The gain applied to all buses.
        float master_gain;

        // If true, the master gain is ignored and all channels have a gain of 0.
        bool mute;

        // If true, the entire audio engine has paused all playback.
        bool paused;

        // A map of sound names to SoundCollections.
        SoundCollectionMap sound_collection_map;

        // A map of file names to sound ids to determine if a file needs to be loaded.
        SoundIdMap sound_id_map;

        // A map of event names to EventInternalStates.
        EventMap event_map;

        // A map of file names to event ids to determine if a file needs to be loaded.
        EventIdMap event_id_map;

        // A vector of currently active events.
        EventInstanceVector running_events;

        // Hold the sounds banks.
        SoundBankMap sound_bank_map;

        // The pre-allocated pool of all ChannelInternalState objects
        ChannelStateVector channel_state_memory;

        // The lists that track currently playing channels and free channels.
        PriorityList playing_channel_list;
        FreeList real_channel_free_list;
        FreeList virtual_channel_free_list;

        // The list of listeners.
        ListenerList listener_list;
        ListenerStateVector listener_state_memory;
        std::vector<ListenerInternalState*> listener_state_free_list;

        // The list of entities.
        EntityList entity_list;
        EntityStateVector entity_state_memory;
        std::vector<EntityInternalState*> entity_state_free_list;

        // Loads the sound files.
        FileLoader loader;

        // The current frame, i.e. the number of times AdvanceFrame has been called.
        AmUInt32 current_frame;

        const struct Version* version;
    };

    /**
     * @brief Removes all the finished sounds from the playing list.
     * @param state The engine state to update.
     */
    void EraseFinishedSounds(EngineInternalState* state);

    // Find a GetBus with the given name.
    BusInternalState* FindBusInternalState(EngineInternalState* state, AmString name);

    // Given a playing sound, find where a new sound with the given priority should
    // be inserted into the list.
    PriorityList::iterator FindInsertionPoint(PriorityList* list, float priority);

    // Given a list of listeners and a location, find which listener is closest.
    // Additionally, return the square of the distance between the closest listener
    // and the location, as well as the given location translated into listener
    // space.  Returns true on success, or false if the list was empty.
    bool BestListener(
        ListenerList::const_iterator* best_listener,
        float* distance_squared,
        hmm_vec3* listener_space_location,
        const ListenerList& listeners,
        const hmm_vec3& location);

    // This will take a point between lower_bound and upper_bound and use it to
    // calculate an attenuation multiplier. The curve_factor can be adjusted based
    // on how rapidly the attenuation should change with distance.
    //
    // A curve_factor of 1.0 means the attenuation will adjust linearly with
    // distance.
    //
    // A curve_factor greater than 1.0 means the attenuation will change gently at
    // first, then rapidly approach its target.
    //
    // A fractional curve_factor between 0.0 and 1.0 means the attenuation will
    // change rapidly at first, then gently approach its target.
    float AttenuationCurve(float point, float lower_bound, float upper_bound, float curve_factor);

    // Determine whether the sound can be heard at all. If it can, apply the roll
    // in gain, roll out gain, or norminal gain appropriately.
    float CalculateDistanceAttenuation(float distance_squared, const SoundCollectionDefinition* def);

    // Given a vector in listener space, return a vector inside a unit circle
    // representing the direction from the listener to the sound. A value of (-1, 0)
    // means the sound is directly to the listener's left, while a value of (1, 0)
    // means the sound is directly to the listener's right. Likewise, values of
    // (0, 1) and (0, -1) mean the sound is directly in front or behind the
    // listener, respectively.
    hmm_vec2 CalculatePan(const hmm_vec3& listener_space_location);

    bool LoadFile(AmString filename, std::string* dest);

    AmUInt32 GetMaxNumberOfChannels(const EngineConfigDefinition* config);

} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_ENGINE_INTERNAL_STATE_H
