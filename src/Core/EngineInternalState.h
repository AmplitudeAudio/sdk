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
#include <Core/EnvironmentInternalState.h>
#include <Core/ListenerInternalState.h>

#include <Mixer/Mixer.h>

#include <Utils/intrusive_list.h>

#include "collection_definition_generated.h"
#include "engine_config_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    struct EngineConfigDefinition;
    struct BusDefinitionList;
    struct SoundBankDefinition;

    typedef std::map<AmOsString, AmEffectID> EffectIdMap;
    typedef std::map<AmOsString, AmRtpcID> RtpcIdMap;
    typedef std::map<AmOsString, AmSwitchID> SwitchIdMap;
    typedef std::map<AmOsString, AmAttenuationID> AttenuationIdMap;
    typedef std::map<AmOsString, AmCollectionID> CollectionIdMap;
    typedef std::map<AmOsString, AmSwitchContainerID> SwitchContainerIdMap;
    typedef std::map<AmOsString, AmSoundID> SoundIdMap;
    typedef std::map<AmOsString, AmEventID> EventIdMap;
    typedef std::map<AmOsString, AmBankID> SoundBankIdMap;

    typedef std::map<AmSwitchContainerID, AmUniquePtr<MemoryPoolKind::Engine, SwitchContainer>> SwitchContainerMap;

    typedef std::map<AmCollectionID, AmUniquePtr<MemoryPoolKind::Engine, Collection>> CollectionMap;

    typedef std::map<AmSoundID, AmUniquePtr<MemoryPoolKind::Engine, Sound>> SoundMap;

    typedef std::map<AmAttenuationID, AmUniquePtr<MemoryPoolKind::Engine, Attenuation>> AttenuationMap;

    typedef std::map<AmSwitchID, AmUniquePtr<MemoryPoolKind::Engine, Switch>> SwitchMap;

    typedef std::map<AmRtpcID, AmUniquePtr<MemoryPoolKind::Engine, Rtpc>> RtpcMap;

    typedef std::map<AmEffectID, AmUniquePtr<MemoryPoolKind::Engine, Effect>> EffectMap;

    typedef std::map<AmEventID, AmUniquePtr<MemoryPoolKind::Engine, Event>> EventMap;

    typedef std::map<AmBankID, std::unique_ptr<SoundBank>> SoundBankMap;

    typedef std::vector<EventInstance> EventInstanceVector;

    typedef std::vector<ChannelInternalState> ChannelStateVector;

    typedef fplutil::intrusive_list<ChannelInternalState> PriorityList;

    typedef fplutil::intrusive_list<ChannelInternalState> FreeList;

    typedef std::vector<EntityInternalState> EntityStateVector;
    typedef fplutil::intrusive_list<EntityInternalState> EntityList;

    typedef std::vector<ListenerInternalState> ListenerStateVector;
    typedef fplutil::intrusive_list<ListenerInternalState> ListenerList;

    typedef std::vector<EnvironmentInternalState> EnvironmentStateVector;
    typedef fplutil::intrusive_list<EnvironmentInternalState> EnvironmentList;

    struct ObstructionOcclusionState
    {
        Curve lpf;
        Curve gain;

        void Init(const ObstructionOcclusionConfig* config)
        {
            lpf = Curve();
            lpf.Initialize(config->lpf_curve());

            gain = Curve();
            gain.Initialize(config->gain_curve());
        }
    };

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
            , sound_bank_id_map()
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
            , environment_list(&EnvironmentInternalState::node)
            , environment_state_memory()
            , environment_state_free_list()
            , current_frame(0)
            , total_time(0.0)
            , listener_fetch_mode(eListenerFetchMode_None)
            , sound_speed(333.0)
            , doppler_factor(1.0)
            , up_axis(eGameEngineUpAxis_Y)
            , obstruction_config()
            , occlusion_config()
            , track_environments(false)
            , samples_per_stream(512)
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

        // The list of environments.
        EnvironmentList environment_list;
        EnvironmentStateVector environment_state_memory;
        std::vector<EnvironmentInternalState*> environment_state_free_list;

        // The current frame, i.e. the number of times AdvanceFrame has been called.
        AmUInt64 current_frame;

        // The total elapsed time since the start of the game.
        AmTime total_time;

        // The way Amplitude should fetch the best listener for an audio source.
        eListenerFetchMode listener_fetch_mode;

        AmReal32 sound_speed;

        AmReal32 doppler_factor;

        // The up axis of the game engine.
        eGameEngineUpAxis up_axis;

        ObstructionOcclusionState obstruction_config;

        ObstructionOcclusionState occlusion_config;

        bool track_environments;

        AmUInt32 samples_per_stream;

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
    BusInternalState* FindBusInternalState(EngineInternalState* state, const AmString& name);

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
        AmVec3* listenerSpaceLocation,
        const ListenerList& listeners,
        const AmVec3& location);

    // Given a vector in listener space, return a vector inside a unit circle
    // representing the direction from the listener to the sound. A value of (-1, 0)
    // means the sound is directly to the listener's left, while a value of (1, 0)
    // means the sound is directly to the listener's right. Likewise, values of
    // (0, 1) and (0, -1) mean the sound is directly in front or behind the
    // listener, respectively.
    AmVec2 CalculatePan(const AmVec3& listenerSpaceLocation);

    bool LoadFile(const std::shared_ptr<File>& file, std::string* dest);

    AmUInt32 GetMaxNumberOfChannels(const EngineConfigDefinition* config);
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_ENGINEINTERNALSTATE_H
