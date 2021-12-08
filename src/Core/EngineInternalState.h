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

#ifndef SS_AMPLITUDE_AUDIO_ENGINEINTERNALSTATE_H
#define SS_AMPLITUDE_AUDIO_ENGINEINTERNALSTATE_H

#include <map>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/BusInternalState.h>
#include <Core/ChannelInternalState.h>
#include <Core/EntityInternalState.h>
#include <Core/ListenerInternalState.h>
#include <Mixer/Mixer.h>
#include <Utils/intrusive_list.h>

#include "collection_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    struct EngineConfigDefinition;
    struct BusDefinitionList;
    struct SoundBankDefinition;

#if defined(AM_WCHAR_SUPPORTED)
    typedef std::map<std::wstring, AmEffectID> EffectIdMap;
    typedef std::map<std::wstring, AmRtpcID> RtpcIdMap;
    typedef std::map<std::wstring, AmSwitchID> SwitchIdMap;
    typedef std::map<std::wstring, AmAttenuationID> AttenuationIdMap;
    typedef std::map<std::wstring, AmCollectionID> CollectionIdMap;
    typedef std::map<std::wstring, AmSwitchContainerID> SwitchContainerIdMap;
    typedef std::map<std::wstring, AmSoundID> SoundIdMap;
    typedef std::map<std::wstring, AmEventID> EventIdMap;
    typedef std::map<std::wstring, AmBankID> SoundBankIdMap;
#else
    typedef std::map<std::string, AmEffectID> EffectIdMap;
    typedef std::map<std::string, AmRtpcID> RtpcIdMap;
    typedef std::map<std::string, AmSwitchID> SwitchIdMap;
    typedef std::map<std::string, AmAttenuationID> AttenuationIdMap;
    typedef std::map<std::string, AmCollectionID> CollectionIdMap;
    typedef std::map<std::string, AmSwitchContainerID> SwitchContainerIdMap;
    typedef std::map<std::string, AmSoundID> SoundIdMap;
    typedef std::map<std::string, AmEventID> EventIdMap;
    typedef std::map<std::string, AmBankID> SoundBankIdMap;
#endif

    typedef std::map<AmSwitchContainerID, std::unique_ptr<SwitchContainer>> SwitchContainerMap;

    typedef std::map<AmCollectionID, std::unique_ptr<Collection>> CollectionMap;

    typedef std::map<AmSoundID, std::unique_ptr<Sound>> SoundMap;

    typedef std::map<AmAttenuationID, std::unique_ptr<Attenuation>> AttenuationMap;

    typedef std::map<AmSwitchID, std::unique_ptr<Switch>> SwitchMap;

    typedef std::map<AmRtpcID, std::unique_ptr<Rtpc>> RtpcMap;

    typedef std::map<AmEffectID, std::unique_ptr<Effect>> EffectMap;

    typedef std::map<AmEventID, std::unique_ptr<Event>> EventMap;

    typedef std::map<AmBankID, std::unique_ptr<SoundBank>> SoundBankMap;

    typedef std::vector<EventInstance> EventInstanceVector;

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
            : mixer(1.0f)
            , buses_source()
            , buses()
            , master_bus(nullptr)
            , master_gain(1.0f)
            , mute(true)
            , paused(true)
            , stopping(false)
            , switch_container_map()
            , switch_container_id_map()
            , collection_map()
            , collection_id_map()
            , event_map()
            , event_id_map()
            , running_events()
            , attenuation_map()
            , attenuation_id_map()
            , switch_map()
            , switch_id_map()
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
            , current_frame(0)
            , total_time(0.0)
            , version(nullptr)
        {}

        Mixer mixer;

        // Hold the audio buses definition file contents.
        std::string buses_source;

        // The state of the buses.
        std::vector<BusInternalState> buses;

        // The master bus, cached to prevent needless lookups.
        BusInternalState* master_bus;

        // The gain applied to all buses.
        float master_gain;

        // If true, the master gain is ignored and all channels have a gain of 0.
        bool mute;

        // If true, the entire audio engine has paused all playback.
        bool paused;

        // If true, the engine is in the process of shutting down.
        bool stopping;

        // A map of sound names to SoundCollections.
        SwitchContainerMap switch_container_map;

        // A map of file names to sound ids to determine if a file needs to be loaded.
        SwitchContainerIdMap switch_container_id_map;

        // A map of sound names to SoundCollections.
        CollectionMap collection_map;

        // A map of file names to sound ids to determine if a file needs to be loaded.
        CollectionIdMap collection_id_map;

        // A map of sound names to SoundCollections.
        SoundMap sound_map;

        // A map of file names to sound ids to determine if a file needs to be loaded.
        SoundIdMap sound_id_map;

        // A map of event names to EventInternalStates.
        EventMap event_map;

        // A map of file names to event ids to determine if a file needs to be loaded.
        EventIdMap event_id_map;

        // A vector of currently active events.
        EventInstanceVector running_events;

        // A map of attenuation ids to Attenuation
        AttenuationMap attenuation_map;

        // A map of file names to attenuation ids to determine if a file needs to be loaded.
        AttenuationIdMap attenuation_id_map;

        // A map of switch ids to Switch
        SwitchMap switch_map;

        // A map of file names to switch ids to determine if a file needs to be loaded.
        SwitchIdMap switch_id_map;

        // A map of RTPC ids to Rtpc
        RtpcMap rtpc_map;

        // A map of file names to RTPC ids to determine if a file needs to be loaded.
        RtpcIdMap rtpc_id_map;

        // A map of effect ids to Effect
        EffectMap effect_map;

        // A map of file names to effect ids to determine if a file needs to be loaded.
        EffectIdMap effect_id_map;

        // A map of sound banks id to SoundBank.
        SoundBankIdMap sound_bank_id_map;

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

        // The current frame, i.e. the number of times AdvanceFrame has been called.
        AmUInt64 current_frame;

        // The total elapsed time since the start of the game.
        AmTime total_time;

        const struct Version* version;
    };

    /**
     * @brief Removes all the finished sounds from the playing list.
     * @param state The engine state to update.
     */
    void EraseFinishedSounds(EngineInternalState* state);

    // Find a bus with the given ID.
    BusInternalState* FindBusInternalState(EngineInternalState* state, AmBusID id);

    // Find a bus with the given name.
    BusInternalState* FindBusInternalState(EngineInternalState* state, AmString id);

    // Given a playing sound, find where a new sound with the given priority should
    // be inserted into the list.
    PriorityList::iterator FindInsertionPoint(PriorityList* list, float priority);

    // Given a list of listeners and a location, find which listener is closest.
    // Additionally, return the square of the distance between the closest listener
    // and the location, as well as the given location translated into listener
    // space.  Returns true on success, or false if the list was empty.
    bool BestListener(
        ListenerList::const_iterator* bestListener,
        float* distanceSquared,
        hmm_vec3* listenerSpaceLocation,
        const ListenerList& listeners,
        const hmm_vec3& location);

    // Given a vector in listener space, return a vector inside a unit circle
    // representing the direction from the listener to the sound. A value of (-1, 0)
    // means the sound is directly to the listener's left, while a value of (1, 0)
    // means the sound is directly to the listener's right. Likewise, values of
    // (0, 1) and (0, -1) mean the sound is directly in front or behind the
    // listener, respectively.
    hmm_vec2 CalculatePan(const hmm_vec3& listenerSpaceLocation);

    bool LoadFile(AmOsString filename, std::string* dest);

    AmUInt32 GetMaxNumberOfChannels(const EngineConfigDefinition* config);
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_ENGINEINTERNALSTATE_H
