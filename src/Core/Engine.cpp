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

#include <algorithm>
#include <cmath>
#include <fstream>
#include <memory>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/BusInternalState.h>
#include <Core/EngineInternalState.h>
#include <IO/File.h>

#include "buses_definition_generated.h"
#include "engine_config_definition_generated.h"
#include "sound_collection_definition_generated.h"

#pragma region Default Codecs
#include <Core/Codecs/OGG/Codec.h>
#include <Core/Codecs/WAV/Codec.h>
#pragma endregion

#pragma region Default Drivers
#include <Core/Drivers/MiniAudio/Driver.h>
#pragma endregion

namespace SparkyStudios::Audio::Amplitude
{
    typedef flatbuffers::Vector<uint64_t> BusIdList;
    typedef flatbuffers::Vector<flatbuffers::Offset<DuckBusDefinition>> DuckBusDefinitionList;

    bool LoadFile(AmOsString filename, std::string* dest)
    {
        if (!filename)
        {
            CallLogFunc("The filename is empty");
            return false;
        }

        // load into memory:
        MemoryFile mf;
        AmResult result = mf.OpenToMem(filename);
        if (result != AM_ERROR_NO_ERROR)
        {
            CallLogFunc("LoadFile fail on %s", filename);
            return false;
        }

        // get its size:
        dest->assign(mf.Length() + 1, 0);

        // read the data:
        AmUInt32 len = mf.Read(reinterpret_cast<AmUInt8Buffer>(&(*dest)[0]), mf.Length());

        return len == mf.Length() && len > 0;
    }

    AmUInt32 GetMaxNumberOfChannels(const EngineConfigDefinition* config)
    {
        return config->mixer()->virtual_channels() + config->mixer()->active_channels();
    }

    Engine::Engine()
        : _state(nullptr)
        , _audioDriver(nullptr)
        , _configSrc()
    {}

    Engine::~Engine()
    {
        _configSrc.clear();

        _audioDriver = nullptr;

        delete _state;
        _state = nullptr;
    }

    Engine* Engine::GetInstance()
    {
        // Amplitude Engine unique instance.
        static Engine* amplitude = nullptr;

        if (amplitude == nullptr)
            amplitude = new Engine();

        return amplitude;
    }

    BusInternalState* FindBusInternalState(EngineInternalState* state, AmBusID id)
    {
        auto it = std::find_if(
            state->buses.begin(), state->buses.end(),
            [id](const BusInternalState& bus)
            {
                return bus.GetId() == id;
            });

        if (it != state->buses.end())
        {
            return &*it;
        }
        else
        {
            return nullptr;
        }
    }

    BusInternalState* FindBusInternalState(EngineInternalState* state, AmString name)
    {
        auto it = std::find_if(
            state->buses.begin(), state->buses.end(),
            [name](const BusInternalState& bus)
            {
                return bus.GetName() == name;
            });

        if (it != state->buses.end())
        {
            return &*it;
        }
        else
        {
            return nullptr;
        }
    }

    static bool PopulateChildBuses(EngineInternalState* state, BusInternalState* parent, const BusIdList* childIdList)
    {
        std::vector<BusInternalState*>* output = &parent->GetChildBuses();

        for (flatbuffers::uoffset_t i = 0; childIdList && i < childIdList->size(); ++i)
        {
            AmBusID busId = childIdList->Get(i);
            BusInternalState* bus = FindBusInternalState(state, busId);

            if (bus)
            {
                output->push_back(bus);
            }
            else
            {
                CallLogFunc("Unknown bus with ID \"%u\" listed in child buses.\n", busId);
                return false;
            }
        }
        return true;
    }

    static bool PopulateDuckBuses(EngineInternalState* state, BusInternalState* parent, const DuckBusDefinitionList* duckBusDefinitionList)
    {
        std::vector<DuckBusInternalState*>* output = &parent->GetDuckBuses();

        for (flatbuffers::uoffset_t i = 0; duckBusDefinitionList && i < duckBusDefinitionList->size(); ++i)
        {
            const DuckBusDefinition* duck = duckBusDefinitionList->Get(i);
            auto* bus = new DuckBusInternalState(parent);

            if (bus->Initialize(duck))
            {
                output->push_back(bus);
            }
            else
            {
                CallLogFunc("Unknown bus with ID \"%u\" listed in duck buses.\n", duck->id());
                return false;
            }
        }
        return true;
    }

    // The InternalChannelStates have three lists they are a part of: The engine's
    // priority list, the bus's playing sound list, and which free list they are in.
    // Initially, all nodes are in a free list because nothing is playing. Separate
    // free lists are kept for real channels and virtual channels (where 'real'
    // channels are channels that have a channel_id
    static void InitializeChannelFreeLists(
        FreeList* real_channel_free_list,
        FreeList* virtual_channel_free_list,
        std::vector<ChannelInternalState>* channels,
        AmUInt32 virtual_channels,
        AmUInt32 real_channels)
    {
        // We do our own tracking of audio channels so that when a new sound is
        // played we can determine if one of the currently playing channels is lower
        // priority so that we can drop it.
        AmUInt32 total_channels = real_channels + virtual_channels;
        channels->resize(total_channels);
        for (size_t i = 0; i < total_channels; ++i)
        {
            ChannelInternalState& channel = (*channels)[i];

            // Track real channels separately from virtual channels.
            if (i < real_channels)
            {
                channel.GetRealChannel().Initialize(static_cast<int>(i));
                real_channel_free_list->push_front(channel);
            }
            else
            {
                virtual_channel_free_list->push_front(channel);
            }
        }
    }

    static void InitializeListenerFreeList(
        std::vector<ListenerInternalState*>* listener_state_free_list, ListenerStateVector* listener_list, AmUInt32 list_size)
    {
        listener_list->resize(list_size);
        listener_state_free_list->reserve(list_size);
        for (size_t i = 0; i < list_size; ++i)
        {
            ListenerInternalState& listener = (*listener_list)[i];
            listener_state_free_list->push_back(&listener);
        }
    }

    static void InitializeEntityFreeList(
        std::vector<EntityInternalState*>* entity_state_free_list, EntityStateVector* entity_list, AmUInt32 list_size)
    {
        entity_list->resize(list_size);
        entity_state_free_list->reserve(list_size);
        for (size_t i = 0; i < list_size; ++i)
        {
            EntityInternalState& entity = (*entity_list)[i];
            entity_state_free_list->push_back(&entity);
        }
    }

    bool Engine::Initialize(AmOsString config_file)
    {
        if (!LoadFile(config_file, &_configSrc))
        {
            CallLogFunc("Could not load audio config file.\n");
            return false;
        }
        return Initialize(GetEngineConfigDefinition());
    }

    bool Engine::Initialize(const EngineConfigDefinition* config)
    {
        // Lock drivers and codecs registry
        Driver::LockRegistry();
        Codec::LockRegistry();

        // Create the internal engine state
        _state = new EngineInternalState();
        _state->version = Version();

        // Load the audio driver
        if (config->driver())
        {
            if (_audioDriver = Driver::Find(config->driver()->c_str()); _audioDriver == nullptr)
            {
                CallLogFunc("Could load the audio driver '%s'. Loading the default driver.\n", config->driver()->c_str());
                _audioDriver = Driver::Default();
            }
        }
        else
        {
            _audioDriver = Driver::Default();
        }

        if (_audioDriver == nullptr)
        {
            CallLogFunc("Could not load the audio driver.\n");
            return false;
        }

        // Initialize audio mixer
        if (!_state->mixer.Initialize(config))
        {
            CallLogFunc("Could not initialize the audio mixer.\n");
            return false;
        }

        // Initialize the audio driver
        _audioDriver->Initialize(&_state->mixer);

        // Initialize the channel internal data.
        InitializeChannelFreeLists(
            &_state->real_channel_free_list, &_state->virtual_channel_free_list, &_state->channel_state_memory,
            config->mixer()->virtual_channels(), config->mixer()->active_channels());

        // Initialize the listener internal data.
        InitializeListenerFreeList(&_state->listener_state_free_list, &_state->listener_state_memory, config->listeners());

        // Initialize the entity internal data.
        InitializeEntityFreeList(&_state->entity_state_free_list, &_state->entity_state_memory, config->entities());

        // Load the audio buses.
        if (!LoadFile(AM_STRING_TO_OS_STRING(config->buses_file()->c_str()), &_state->buses_source))
        {
            CallLogFunc("Could not load audio bus file.\n");
            return false;
        }
        const BusDefinitionList* bus_def_list = Amplitude::GetBusDefinitionList(_state->buses_source.c_str());
        _state->buses.resize(bus_def_list->buses()->size());
        for (flatbuffers::uoffset_t i = 0; i < bus_def_list->buses()->size(); ++i)
        {
            _state->buses[i].Initialize(bus_def_list->buses()->Get(i));
        }

        // Set up the children and ducking pointers.
        for (auto& bus : _state->buses)
        {
            const BusDefinition* def = bus.GetBusDefinition();
            if (!PopulateChildBuses(_state, &bus, def->child_buses()))
            {
                return false;
            }
            if (!PopulateDuckBuses(_state, &bus, def->duck_buses()))
            {
                return false;
            }
        }

        // Fetch the master bus with name
        _state->master_bus = FindBusInternalState(_state, "master");
        if (!_state->master_bus)
        {
            // Fetch the master bus by ID
            _state->master_bus = FindBusInternalState(_state, kAmMasterBusId);
            if (!_state->master_bus)
            {
                CallLogFunc("[Error] Unable to find a master bus.\n");
                return false;
            }
        }

        _state->paused = false;
        _state->mute = false;
        _state->master_gain = 1.0f;

        // Open the audio device through the driver
        return _audioDriver->Open(config);
    }

    bool Engine::Deinitialize()
    {
        _state->stopping = true;

        // Auto deinitialize loaded sound banks
        for (const auto& item : _state->sound_bank_map)
            item.second->Deinitialize(this);

        // Close the audio device through the driver
        return _audioDriver->Close();
    }

    bool Engine::LoadSoundBank(AmOsString filename)
    {
        bool success = true;
        auto iter = _state->sound_bank_map.find(filename);
        if (iter == _state->sound_bank_map.end())
        {
            auto& sound_bank = _state->sound_bank_map[filename];
            sound_bank = std::make_unique<SoundBank>();
            success = sound_bank->Initialize(filename, this);
            if (success)
            {
                sound_bank->GetRefCounter()->Increment();
            }
        }
        else
        {
            iter->second->GetRefCounter()->Increment();
        }
        return success;
    }

    void Engine::UnloadSoundBank(AmOsString filename)
    {
        auto iter = _state->sound_bank_map.find(filename);
        if (iter == _state->sound_bank_map.end())
        {
            CallLogFunc("Error while deinitializing SoundBank " AM_OS_CHAR_FMT " - sound bank not loaded.\n", filename);
            AMPLITUDE_ASSERT(0);
        }
        if (iter->second->GetRefCounter()->Decrement() == 0)
        {
            iter->second->Deinitialize(this);
        }
    }

    void Engine::StartLoadingSoundFiles()
    {
        _state->loader.StartLoading();
    }

    bool Engine::TryFinalize()
    {
        return _state->loader.TryFinalize();
    }

    bool BestListener(
        ListenerList::const_iterator* best_listener,
        float* distance_squared,
        hmm_vec3* listener_space_location,
        const ListenerList& listener_list,
        const hmm_vec3& location)
    {
        if (listener_list.empty())
        {
            return false;
        }
        ListenerList::const_iterator listener = listener_list.cbegin();
        hmm_mat4 mat = listener->GetInverseMatrix();
        *listener_space_location = AM_Multiply(mat, AM_Vec4v(location, 1.0f)).XYZ;
        *distance_squared = AM_LengthSquared(*listener_space_location);
        *best_listener = listener;
        for (++listener; listener != listener_list.cend(); ++listener)
        {
            hmm_vec3 transformed_location = AM_Multiply(listener->GetInverseMatrix(), AM_Vec4v(location, 1.0f)).XYZ;
            float magnitude_squared = AM_LengthSquared(transformed_location);
            if (magnitude_squared < *distance_squared)
            {
                *best_listener = listener;
                *distance_squared = magnitude_squared;
                *listener_space_location = transformed_location;
            }
        }
        return true;
    }

    hmm_vec2 CalculatePan(const hmm_vec3& listener_space_location)
    {
        // Zero length vectors just end up with NaNs when normalized. Return a zero
        // vector instead.
        const float kEpsilon = 0.0001f;
        if (AM_LengthSquared(listener_space_location) <= kEpsilon)
        {
            return AM_Vec2(0.0f, 0.0f);
        }
        hmm_vec3 direction = AM_Normalize(listener_space_location);
        return AM_Vec2(AM_Dot(AM_Vec3(1, 0, 0), direction), AM_Dot(AM_Vec3(0, 0, 1), direction));
    }

    float AttenuationCurve(float point, float lower_bound, float upper_bound, float curve_factor)
    {
        AMPLITUDE_ASSERT(lower_bound <= point && point <= upper_bound && curve_factor >= 0.0f);
        float distance = point - lower_bound;
        float range = upper_bound - lower_bound;
        return distance / ((range - distance) * (curve_factor - 1.0f) + range);
    }

    inline float Square(float f)
    {
        return f * f;
    }

    float CalculateDistanceAttenuation(float distance_squared, const SoundCollectionDefinition* def)
    {
        if (distance_squared < Square(def->min_audible_radius()) || distance_squared > Square(def->max_audible_radius()))
        {
            return 0.0f;
        }
        float distance = AM_SquareRootF(distance_squared);
        if (distance < def->roll_in_radius())
        {
            return AttenuationCurve(distance, def->min_audible_radius(), def->roll_in_radius(), def->roll_in_curve_factor());
        }
        else if (distance > def->roll_out_radius())
        {
            return 1.0f - AttenuationCurve(distance, def->roll_out_radius(), def->max_audible_radius(), def->roll_out_curve_factor());
        }
        else
        {
            return 1.0f;
        }
    }

    static void CalculateGainAndPan(
        float* gain,
        hmm_vec2* pan,
        SoundCollection* collection,
        const Entity& entity,
        const hmm_vec3& location,
        const ListenerList& listener_list,
        float user_gain)
    {
        const SoundCollectionDefinition* def = collection->GetSoundCollectionDefinition();
        *gain = def->gain() * collection->GetBus()->GetGain() * user_gain;
        if (def->spatialization() == Spatialization_Position || def->spatialization() == Spatialization_PositionOrientation)
        {
            ListenerList::const_iterator listener;
            float distance_squared;
            hmm_vec3 listener_space_location;
            if (BestListener(&listener, &distance_squared, &listener_space_location, listener_list, location))
            {
                AttenuationHandle attenuation = collection->GetAttenuation();
                if (attenuation)
                {
                    *gain *= attenuation->GetGain(AM_SquareRootF(distance_squared));

                    if (def->spatialization() == Spatialization_PositionOrientation)
                    {
                        AMPLITUDE_ASSERT(entity.Valid())
                        *gain *= attenuation->GetShape()->GetAttenuationFactor(entity.GetState(), &*listener);
                    }
                    else
                    {
                        *gain *= attenuation->GetShape()->GetAttenuationFactor(location, &*listener);
                    }
                }

                *pan = CalculatePan(listener_space_location);
            }
            else
            {
                *gain = 0.0f;
                *pan = AM_Vec2(0, 0);
            }
        }
        else
        {
            *pan = AM_Vec2(0, 0);
        }
    }

    // Given the priority of a node, and the list of ChannelInternalStates sorted by
    // priority, find the location in the list where the node would be inserted.
    // Note that the node should be inserted using InsertAfter. If the node you want
    // to insert turns out to be the highest priority node, this will return the
    // list terminator (and inserting after the terminator will put it at the front
    // of the list).
    PriorityList::iterator FindInsertionPoint(PriorityList* list, float priority)
    {
        PriorityList::reverse_iterator iter;
        for (iter = list->rbegin(); iter != list->rend(); ++iter)
        {
            float p = iter->Priority();
            if (p > priority)
            {
                break;
            }
        }
        return iter.base();
    }

    // Given a location to insert a node, take an ChannelInternalState from the
    // appropriate list and insert it there. Return the new ChannelInternalState.
    //
    // There are three places an ChannelInternalState may be taken from. First, if
    // there are any real channels available in the real channel free list, use one
    // of those so that your channel can play.
    //
    // If there are no real channels, then use a free virtual channel instead so
    // that your channel can at least be tracked.
    //
    // If there are no real or virtual channels, use the node in the priority list,
    // remove it from the list, and insert it in the new insertion point. This
    // causes the lowest priority sound to stop being tracked.
    //
    // If the node you are trying to insert is the lowest priority, do nothing and
    // return a nullptr.
    //
    // This function could use some unit tests b/20752976
    static ChannelInternalState* FindFreeChannelInternalState(
        PriorityList::iterator insertion_point,
        PriorityList* list,
        FreeList* real_channel_free_list,
        FreeList* virtual_channel_free_list,
        bool paused)
    {
        ChannelInternalState* new_channel = nullptr;
        // Grab a free ChannelInternalState if there is one and the engine is not
        // paused. The engine is paused, grab a virtual channel for now, and it will
        // fix itself when the engine is not paused.
        if (!paused && !real_channel_free_list->empty())
        {
            new_channel = &real_channel_free_list->front();
            real_channel_free_list->pop_front();
            PriorityList::insert_before(*insertion_point, *new_channel, &ChannelInternalState::priority_node);
        }
        else if (!virtual_channel_free_list->empty())
        {
            new_channel = &virtual_channel_free_list->front();
            virtual_channel_free_list->pop_front();
            PriorityList::insert_before(*insertion_point, *new_channel, &ChannelInternalState::priority_node);
        }
        else if (&*insertion_point != &list->back())
        {
            // If there are no free sounds, and the new sound is not the lowest priority
            // sound, evict the lowest priority sound.
            new_channel = &list->back();
            new_channel->Halt();

            // Move it to a new spot in the list if it needs to be moved.
            if (&*insertion_point != new_channel)
            {
                list->pop_back();
                list->insert(insertion_point, *new_channel);
            }
        }
        return new_channel;
    }

    // Returns this channel to the free appropriate free list based on whether it's
    // backed by a real channel or not.
    static void InsertIntoFreeList(EngineInternalState* state, ChannelInternalState* channel)
    {
        channel->Remove();
        channel->Reset();
        FreeList* list = channel->IsReal() ? &state->real_channel_free_list : &state->virtual_channel_free_list;
        list->push_front(*channel);
    }

    Channel Engine::Play(SoundHandle sound_handle)
    {
        return Play(sound_handle, AM_Vec3(0, 0, 0), 1.0f);
    }

    Channel Engine::Play(SoundHandle sound_handle, const hmm_vec3& location)
    {
        return Play(sound_handle, location, 1.0f);
    }

    Channel Engine::Play(SoundHandle sound_handle, const hmm_vec3& location, float user_gain)
    {
        return _playScopedSound(sound_handle, Entity(nullptr), location, user_gain);
    }

    Channel Engine::Play(SoundHandle sound_handle, const Entity& entity)
    {
        return Play(sound_handle, entity, 1.0f);
    }

    Channel Engine::Play(SoundHandle sound_handle, const Entity& entity, float user_gain)
    {
        return _playScopedSound(sound_handle, entity, entity.GetLocation(), user_gain);
    }

    Channel Engine::Play(const std::string& sound_name)
    {
        return Play(sound_name, AM_Vec3(0, 0, 0), 1.0f);
    }

    Channel Engine::Play(const std::string& sound_name, const hmm_vec3& location)
    {
        return Play(sound_name, location, 1.0f);
    }

    Channel Engine::Play(const std::string& sound_name, const hmm_vec3& location, float user_gain)
    {
        SoundHandle handle = GetSoundHandle(sound_name);
        if (handle)
        {
            return Play(handle, location, user_gain);
        }
        else
        {
            CallLogFunc("Cannot play sound: invalid name (%s).\n", sound_name.c_str());
            return Channel(nullptr);
        }
    }

    Channel Engine::Play(const std::string& sound_name, const Entity& entity)
    {
        return Play(sound_name, entity, 1.0f);
    }

    Channel Engine::Play(const std::string& sound_name, const Entity& entity, float user_gain)
    {
        SoundHandle handle = GetSoundHandle(sound_name);
        if (handle)
        {
            return Play(handle, entity, user_gain);
        }
        else
        {
            CallLogFunc("Cannot play sound: invalid name (%s)\n", sound_name.c_str());
            return Channel(nullptr);
        }
    }

    Channel Engine::Play(AmSoundCollectionID id)
    {
        return Play(id, AM_Vec3(0, 0, 0), 1.0f);
    }

    Channel Engine::Play(AmSoundCollectionID id, const hmm_vec3& location)
    {
        return Play(id, location, 1.0f);
    }

    Channel Engine::Play(AmSoundCollectionID id, const hmm_vec3& location, float user_gain)
    {
        SoundHandle handle = GetSoundHandle(id);
        if (handle)
        {
            return Play(handle, location, user_gain);
        }
        else
        {
            CallLogFunc("Cannot play sound: invalid ID (%u).\n", id);
            return Channel(nullptr);
        }
    }

    Channel Engine::Play(AmSoundCollectionID id, const Entity& entity)
    {
        return Play(id, entity, 1.0f);
    }

    Channel Engine::Play(AmSoundCollectionID id, const Entity& entity, float user_gain)
    {
        SoundHandle handle = GetSoundHandle(id);
        if (handle)
        {
            return Play(handle, entity, user_gain);
        }
        else
        {
            CallLogFunc("Cannot play sound: invalid name (%u)\n", id);
            return Channel(nullptr);
        }
    }

    EventCanceler Engine::Trigger(EventHandle event_handle, const Entity& entity)
    {
        EventHandle event = event_handle;
        if (!event)
        {
            CallLogFunc("Cannot trigger event: Invalid event handle.\n");
            return EventCanceler(nullptr);
        }

        EventInstance instance = event->Trigger(entity);
        _state->running_events.push_back(instance);

        return EventCanceler(&instance);
    }

    EventCanceler Engine::Trigger(const std::string& event_name, const Entity& entity)
    {
        EventHandle handle = GetEventHandle(event_name);
        if (handle)
        {
            return Trigger(handle, entity);
        }
        else
        {
            CallLogFunc("Cannot trigger event: invalid name (%s).\n", event_name.c_str());
            return EventCanceler(nullptr);
        }
    }

    SoundHandle Engine::GetSoundHandle(const std::string& name) const
    {
        auto iter = std::find_if(
            _state->sound_collection_map.begin(), _state->sound_collection_map.end(),
            [name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        if (iter == _state->sound_collection_map.end())
        {
            return nullptr;
        }
        return iter->second.get();
    }

    SoundHandle Engine::GetSoundHandle(AmSoundCollectionID id) const
    {
        auto iter = _state->sound_collection_map.find(id);
        if (iter == _state->sound_collection_map.end())
        {
            return nullptr;
        }
        return iter->second.get();
    }

    SoundHandle Engine::GetSoundHandleFromFile(AmOsString filename) const
    {
        auto iter = _state->sound_id_map.find(filename);
        if (iter == _state->sound_id_map.end())
        {
            return nullptr;
        }
        return GetSoundHandle(iter->second);
    }

    EventHandle Engine::GetEventHandle(const std::string& name) const
    {
        auto iter = std::find_if(
            _state->event_map.begin(), _state->event_map.end(),
            [name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        if (iter == _state->event_map.end())
        {
            return nullptr;
        }
        return iter->second.get();
    }

    EventHandle Engine::GetEventHandle(AmEventID id) const
    {
        auto iter = _state->event_map.find(id);
        if (iter == _state->event_map.end())
        {
            return nullptr;
        }
        return iter->second.get();
    }

    EventHandle Engine::GetEventHandleFromFile(AmOsString filename) const
    {
        auto iter = _state->event_id_map.find(filename);
        if (iter == _state->event_id_map.end())
        {
            return nullptr;
        }
        return GetEventHandle(iter->second);
    }

    AttenuationHandle Engine::GetAttenuationHandle(const std::string& name) const
    {
        auto iter = std::find_if(
            _state->attenuation_map.begin(), _state->attenuation_map.end(),
            [name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        if (iter == _state->attenuation_map.end())
        {
            return nullptr;
        }

        return iter->second.get();
    }

    AttenuationHandle Engine::GetAttenuationHandle(AmAttenuationID id) const
    {
        auto iter = _state->attenuation_map.find(id);
        if (iter == _state->attenuation_map.end())
        {
            return nullptr;
        }

        return iter->second.get();
    }

    AttenuationHandle Engine::GetAttenuationHandleFromFile(AmOsString filename) const
    {
        auto iter = _state->attenuation_id_map.find(filename);
        if (iter == _state->attenuation_id_map.end())
        {
            return nullptr;
        }

        return GetAttenuationHandle(iter->second);
    }

    void Engine::SetMasterGain(float gain)
    {
        FindBus(kAmMasterBusId).SetGain(gain);
    }

    Listener Engine::AddListener(AmListenerID id)
    {
        if (_state->listener_state_free_list.empty())
        {
            return Listener(nullptr);
        }
        ListenerInternalState* listener = _state->listener_state_free_list.back();
        listener->SetId(id);
        _state->listener_state_free_list.pop_back();
        _state->listener_list.push_back(*listener);
        return Listener(listener);
    }

    void Engine::RemoveListener(Listener* listener)
    {
        AMPLITUDE_ASSERT(listener->Valid());
        listener->GetState()->SetId(kAmInvalidObjectId);
        listener->GetState()->node.remove();
        _state->listener_state_free_list.push_back(listener->GetState());
    }

    Entity Engine::AddEntity(AmEntityID id)
    {
        if (_state->entity_state_free_list.empty())
        {
            return Entity(nullptr);
        }
        EntityInternalState* entity = _state->entity_state_free_list.back();
        entity->SetId(id);
        _state->entity_state_free_list.pop_back();
        _state->entity_list.push_back(*entity);
        return Entity(entity);
    }

    void Engine::RemoveEntity(Entity* entity)
    {
        AMPLITUDE_ASSERT(entity->Valid());
        entity->GetState()->SetId(kAmInvalidObjectId);
        entity->GetState()->node.remove();
        _state->entity_state_free_list.push_back(entity->GetState());
    }

    Bus Engine::FindBus(AmString bus_name)
    {
        return Bus(FindBusInternalState(_state, bus_name));
    }

    Bus Engine::FindBus(AmBusID id)
    {
        return Bus(FindBusInternalState(_state, id));
    }

    void Engine::Pause(bool pause)
    {
        _state->paused = pause;

        PriorityList& list = _state->playing_channel_list;
        for (auto& iter : list)
        {
            if (!iter.Paused() && iter.IsReal())
            {
                if (pause)
                {
                    // Pause the real channel underlying this virtual channel. This freezes
                    // playback of the channel without marking it as paused from the audio
                    // engine's point of view, so that we know to restart it when the audio
                    // engine is not paused.
                    iter.GetRealChannel().Pause();
                }
                else
                {
                    // Resumed all channels that were not explicitly paused.
                    iter.GetRealChannel().Resume();
                }
            }
        }
    }

    void EraseFinishedSounds(EngineInternalState* state)
    {
        PriorityList& list = state->playing_channel_list;
        for (auto iter = list.begin(); iter != list.end();)
        {
            auto current = iter++;
            current->UpdateState();
            if (current->Stopped())
            {
                InsertIntoFreeList(state, &*current);
            }
        }
    }

    static void UpdateChannel(ChannelInternalState* channel, EngineInternalState* state)
    {
        float gain;
        hmm_vec2 pan;
        CalculateGainAndPan(
            &gain, &pan, channel->GetSoundCollection(), channel->GetEntity(), channel->GetLocation(), state->listener_list,
            channel->GetUserGain());
        channel->SetGain(gain);
        channel->SetPan(pan);
    }

    // If there are any free real channels, assign those to virtual channels that
    // need them. If the priority list has gaps (i.e. if there are real channels
    // that are lower priority than virtual channels) then move the lower priority
    // real channels to the higher priority virtual channels.
    // TODO(amablue): Write unit tests for this function. b/20696606
    static void UpdateRealChannels(PriorityList* priority_list, FreeList* real_free_list, FreeList* virtual_free_list)
    {
        PriorityList::reverse_iterator reverse_iter = priority_list->rbegin();
        for (auto iter = priority_list->begin(); iter != priority_list->end(); ++iter)
        {
            if (!iter->IsReal())
            {
                // First check if there are any free real channels.
                if (!real_free_list->empty())
                {
                    // We have a free real channel. Assign this channel id to the channel
                    // that is trying to resume, clear the free channel, and push it into
                    // the virtual free list.
                    ChannelInternalState* free_channel = &real_free_list->front();
                    iter->Devirtualize(free_channel);
                    virtual_free_list->push_front(*free_channel);
                    iter->Resume();
                }
                else
                {
                    // If there aren't any free channels, then scan from the back of the
                    // list for low priority real channels.
                    reverse_iter = std::find_if(
                        reverse_iter, PriorityList::reverse_iterator(iter),
                        [](const ChannelInternalState& channel)
                        {
                            return channel.GetRealChannel().Valid();
                        });
                    if (reverse_iter == priority_list->rend())
                    {
                        // There is no more swapping that can be done. Return.
                        return;
                    }
                    // Found a real channel that we can give to the higher priority
                    // channel.
                    iter->Devirtualize(&*reverse_iter);
                }
            }
        }
    }

    void Engine::AdvanceFrame(AmTime delta_time)
    {
        EraseFinishedSounds(_state);

        for (auto&& bus : _state->buses)
        {
            bus.ResetDuckGain();
        }
        for (auto&& bus : _state->buses)
        {
            bus.UpdateDuckGain(delta_time);
        }

        if (_state->master_bus)
        {
            float master_gain = _state->mute ? 0.0f : _state->master_gain;
            _state->master_bus->AdvanceFrame(delta_time, master_gain);
        }

        PriorityList& list = _state->playing_channel_list;
        for (auto&& iter : list)
        {
            UpdateChannel(&iter, _state);
        }
        list.sort(
            [](const ChannelInternalState& a, const ChannelInternalState& b) -> bool
            {
                return a.Priority() < b.Priority();
            });

        // No point in updating which channels are real and virtual when paused.
        if (!_state->paused)
        {
            UpdateRealChannels(&_state->playing_channel_list, &_state->real_channel_free_list, &_state->virtual_channel_free_list);
        }

        for (size_t i = 0; i < _state->running_events.size(); ++i)
        {
            EventInstance* event = &_state->running_events[i];

            if (!event->IsRunning())
            {
                _state->running_events.erase(_state->running_events.begin() + i);
                --i;
                continue;
            }

            event->AdvanceFrame(delta_time);
        }

        for (auto&& item : _state->listener_list)
        {
            item.Update();
        }

        ++_state->current_frame;
        _state->total_time += delta_time;
    }

    AmTime Engine::GetTotalTime() const
    {
        return _state->total_time;
    }

    const struct Version* Engine::Version() const
    {
        return _state->version;
    }

    const EngineConfigDefinition* Engine::GetEngineConfigDefinition() const
    {
        return Amplitude::GetEngineConfigDefinition(_configSrc.c_str());
    }

    Channel Engine::_playScopedSound(SoundHandle sound_handle, const Entity& entity, const hmm_vec3& location, float user_gain)
    {
        SoundCollection* collection = sound_handle;
        if (!collection)
        {
            CallLogFunc("Cannot play sound: invalid sound handle\n");
            return Channel(nullptr);
        }

        if (collection->GetSoundCollectionDefinition()->scope() == Scope_Entity && !entity.Valid())
        {
            CallLogFunc("[Debug] Cannot play a channel in entity scope. No entity defined.\n");
            return Channel(nullptr);
        }

        // Find where it belongs in the list.
        float gain;
        hmm_vec2 pan;
        CalculateGainAndPan(&gain, &pan, collection, entity, location, _state->listener_list, user_gain);
        float priority = gain * sound_handle->GetSoundCollectionDefinition()->priority();
        PriorityList::iterator insertion_point = FindInsertionPoint(&_state->playing_channel_list, priority);

        // Decide which ChannelInternalState object to use.
        ChannelInternalState* new_channel = FindFreeChannelInternalState(
            insertion_point, &_state->playing_channel_list, &_state->real_channel_free_list, &_state->virtual_channel_free_list,
            _state->paused);

        // The sound could not be added to the list; not high enough priority.
        if (new_channel == nullptr)
        {
            return Channel(nullptr);
        }

        // Now that we have our new sound, set the data on it and update the next
        // pointers.
        new_channel->SetEntity(entity);
        new_channel->SetSoundCollection(sound_handle);
        new_channel->SetUserGain(user_gain);

        // Attempt to play the sound if the engine is not paused.
        if (!_state->paused)
        {
            if (!new_channel->Play())
            {
                // Error playing the sound, put it back in the free list.
                InsertIntoFreeList(_state, new_channel);
                return Channel(nullptr);
            }
        }

        new_channel->SetGain(gain);
        new_channel->SetPan(pan);
        new_channel->SetLocation(location);

        return Channel(new_channel);
    }
} // namespace SparkyStudios::Audio::Amplitude