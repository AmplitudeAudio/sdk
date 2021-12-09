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
#include "collection_definition_generated.h"
#include "engine_config_definition_generated.h"
#include "rtpc_definition_generated.h"
#include "sound_definition_generated.h"
#include "switch_container_definition_generated.h"

#pragma region Default Codecs
#include <Core/Codecs/FLAC/Codec.h>
#include <Core/Codecs/MP3/Codec.h>
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
            CallLogFunc("[ERROR] The filename is empty");
            return false;
        }

        // load into memory:
        MemoryFile mf;
        if (const AmResult result = mf.OpenToMem(filename); result != AM_ERROR_NO_ERROR)
        {
            CallLogFunc("[ERROR] LoadFile fail on %s", filename);
            return false;
        }

        // get its size:
        dest->assign(static_cast<size_t>(mf.Length()) + 1, 0);

        // read the data:
        const AmUInt32 len = mf.Read(reinterpret_cast<AmUInt8Buffer>(&(*dest)[0]), mf.Length());

        return len == mf.Length() && len > 0;
    }

    AmUInt32 GetMaxNumberOfChannels(const EngineConfigDefinition* config)
    {
        return config->mixer()->virtual_channels() + config->mixer()->active_channels();
    }

    Engine::Engine()
        : _configSrc()
        , _state(nullptr)
        , _audioDriver(nullptr)
        , _loader()
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
        if (const auto it = std::find_if(
                state->buses.begin(), state->buses.end(),
                [id](const BusInternalState& bus)
                {
                    return bus.GetId() == id;
                });
            it != state->buses.end())
        {
            return &*it;
        }

        return nullptr;
    }

    BusInternalState* FindBusInternalState(EngineInternalState* state, AmString name)
    {
        if (const auto it = std::find_if(
                state->buses.begin(), state->buses.end(),
                [name](const BusInternalState& bus)
                {
                    return bus.GetName() == name;
                });
            it != state->buses.end())
        {
            return &*it;
        }

        return nullptr;
    }

    static bool PopulateChildBuses(EngineInternalState* state, BusInternalState* parent, const BusIdList* childIdList)
    {
        std::vector<BusInternalState*>* output = &parent->GetChildBuses();

        for (flatbuffers::uoffset_t i = 0; childIdList && i < childIdList->size(); ++i)
        {
            const AmBusID busId = childIdList->Get(i);

            if (BusInternalState* bus = FindBusInternalState(state, busId))
            {
                output->push_back(bus);
            }
            else
            {
                CallLogFunc("[ERROR] Unknown bus with ID \"%u\" listed in child buses.\n", busId);
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

            if (auto* bus = new DuckBusInternalState(parent); bus->Initialize(duck))
            {
                output->push_back(bus);
            }
            else
            {
                CallLogFunc("[ERROR] Unknown bus with ID \"%u\" listed in duck buses.\n", duck->id());
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
        FreeList* realChannelFreeList,
        FreeList* virtualChannelFreeList,
        std::vector<ChannelInternalState>* channels,
        const AmUInt32 virtualChannels,
        const AmUInt32 realChannels)
    {
        // We do our own tracking of audio channels so that when a new sound is
        // played we can determine if one of the currently playing channels is lower
        // priority so that we can drop it.
        const AmUInt32 totalChannels = realChannels + virtualChannels;
        channels->resize(totalChannels);

        for (size_t i = 0; i < totalChannels; ++i)
        {
            ChannelInternalState& channel = (*channels)[i];

            // Track real channels separately from virtual channels.
            if (i < realChannels)
            {
                channel.GetRealChannel().Initialize(static_cast<int>(i + 1));
                realChannelFreeList->push_front(channel);
            }
            else
            {
                channel.GetRealChannel().Initialize(kAmInvalidObjectId);
                virtualChannelFreeList->push_front(channel);
            }
        }
    }

    static void InitializeListenerFreeList(
        std::vector<ListenerInternalState*>* listenerStateFreeList, ListenerStateVector* listenerList, const AmUInt32 listSize)
    {
        listenerList->resize(listSize);
        listenerStateFreeList->reserve(listSize);
        for (size_t i = 0; i < listSize; ++i)
        {
            ListenerInternalState& listener = (*listenerList)[i];
            listenerStateFreeList->push_back(&listener);
        }
    }

    static void InitializeEntityFreeList(
        std::vector<EntityInternalState*>* entityStateFreeList, EntityStateVector* entityList, const AmUInt32 listSize)
    {
        entityList->resize(listSize);
        entityStateFreeList->reserve(listSize);
        for (size_t i = 0; i < listSize; ++i)
        {
            EntityInternalState& entity = (*entityList)[i];
            entityStateFreeList->push_back(&entity);
        }
    }

    bool Engine::Initialize(AmOsString configFile)
    {
        std::filesystem::path configFilePath = _loader.ResolvePath(configFile);
        if (!LoadFile(configFilePath.c_str(), &_configSrc))
        {
            CallLogFunc("[ERROR] Could not load audio config file at path '" AM_OS_CHAR_FMT "'.\n", configFile);
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
        _state->version = &Amplitude::Version();

        // Load the audio driver
        if (config->driver())
        {
            if (_audioDriver = Driver::Find(config->driver()->c_str()); _audioDriver == nullptr)
            {
                CallLogFunc("[WARNING] Could load the audio driver '%s'. Loading the default driver.\n", config->driver()->c_str());
                _audioDriver = Driver::Default();
            }
        }
        else
        {
            _audioDriver = Driver::Default();
        }

        if (_audioDriver == nullptr)
        {
            CallLogFunc("[ERROR] Could not load the audio driver.\n");
            return false;
        }

        // Initialize audio mixer
        if (!_state->mixer.Init(config))
        {
            CallLogFunc("[ERROR] Could not initialize the audio mixer.\n");
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
            CallLogFunc("[ERROR] Could not load audio bus file.\n");
            return false;
        }
        const BusDefinitionList* busDefList = Amplitude::GetBusDefinitionList(_state->buses_source.c_str());
        _state->buses.resize(busDefList->buses()->size());
        for (flatbuffers::uoffset_t i = 0; i < busDefList->buses()->size(); ++i)
        {
            _state->buses[i].Initialize(busDefList->buses()->Get(i));
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
                CallLogFunc("[ERROR] Unable to find a master bus.\n");
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

        delete _state;
        _state = nullptr;

        // Close the audio device through the driver
        return _audioDriver->Close();
    }

    bool Engine::IsInitialized()
    {
        // An initialized engine have a running state
        return _state != nullptr && _state->stopping == false;
    }

    void Engine::SetFileLoader(const FileLoader& loader)
    {
        _loader = loader;
    }

    const FileLoader* Engine::GetFileLoader() const
    {
        return &_loader;
    }

    bool Engine::LoadSoundBank(AmOsString filename)
    {
        bool success = true;
        if (const auto findIt = _state->sound_bank_id_map.find(filename); findIt == _state->sound_bank_id_map.end())
        {
            auto soundBank = std::make_unique<SoundBank>();
            success = soundBank->Initialize(filename, this);

            if (success)
            {
                _state->sound_bank_id_map[filename] = soundBank->GetId();
                soundBank->GetRefCounter()->Increment();
            }
        }
        else
        {
            _state->sound_bank_map[findIt->second]->GetRefCounter()->Increment();
        }

        return success;
    }

    void Engine::UnloadSoundBank(AmOsString filename)
    {
        const auto findIt = _state->sound_bank_id_map.find(filename);
        if (findIt == _state->sound_bank_id_map.end())
        {
            CallLogFunc("[ERROR] Error while deinitializing SoundBank " AM_OS_CHAR_FMT " - sound bank not loaded.\n", filename);
            AMPLITUDE_ASSERT(0);
        }
        UnloadSoundBank(findIt->second);
    }

    void Engine::UnloadSoundBank(AmBankID id)
    {
        const auto findIt = _state->sound_bank_map.find(id);
        if (findIt == _state->sound_bank_map.end())
        {
            CallLogFunc("[ERROR] Error while deinitializing SoundBank with ID %d - sound bank not loaded.\n", id);
            AMPLITUDE_ASSERT(0);
        }
        if (findIt->second->GetRefCounter()->Decrement() == 0)
        {
            findIt->second->Deinitialize(this);
        }
    }

    void Engine::UnloadSoundBanks()
    {
        for (auto&& soundbank : _state->sound_bank_id_map)
        {
            UnloadSoundBank(soundbank.second);
        }
    }

    void Engine::StartLoadingSoundFiles()
    {
        _loader.StartLoading();
    }

    bool Engine::TryFinalizeLoadingSoundFiles()
    {
        return _loader.TryFinalize();
    }

    bool BestListener(
        ListenerList::const_iterator* bestListener,
        float* distanceSquared,
        hmm_vec3* listenerSpaceLocation,
        const ListenerList& listeners,
        const hmm_vec3& location)
    {
        if (listeners.empty())
        {
            return false;
        }

        auto listener = listeners.cbegin();
        const hmm_mat4& mat = listener->GetInverseMatrix();
        *listenerSpaceLocation = AM_Multiply(mat, AM_Vec4v(location, 1.0f)).XYZ;
        *distanceSquared = AM_LengthSquared(*listenerSpaceLocation);
        *bestListener = listener;
        for (++listener; listener != listeners.cend(); ++listener)
        {
            const hmm_vec3 transformedLocation = AM_Multiply(listener->GetInverseMatrix(), AM_Vec4v(location, 1.0f)).XYZ;
            if (const float magnitudeSquared = AM_LengthSquared(transformedLocation); magnitudeSquared < *distanceSquared)
            {
                *bestListener = listener;
                *distanceSquared = magnitudeSquared;
                *listenerSpaceLocation = transformedLocation;
            }
        }

        return true;
    }

    hmm_vec2 CalculatePan(const hmm_vec3& listenerSpaceLocation)
    {
        if (AM_LengthSquared(listenerSpaceLocation) <= kEpsilon)
        {
            return AM_Vec2(0.0f, 0.0f);
        }

        const hmm_vec3 direction = AM_Normalize(listenerSpaceLocation);
        return AM_Vec2(AM_Dot(AM_Vec3(1, 0, 0), direction), AM_Dot(AM_Vec3(0, 0, 1), direction));
    }

    static void CalculateGainAndPan(
        float* gain,
        hmm_vec2* pan,
        const float soundGain,
        const BusInternalState* bus,
        const Spatialization spatialization,
        const Attenuation* attenuation,
        const Entity& entity,
        const hmm_vec3& location,
        const ListenerList& listeners,
        const float userGain)
    {
        *gain = soundGain * bus->GetGain() * userGain;
        if (spatialization == Spatialization_Position || spatialization == Spatialization_PositionOrientation)
        {
            ListenerList::const_iterator listener;
            float distanceSquared;
            hmm_vec3 listenerSpaceLocation;
            if (BestListener(&listener, &distanceSquared, &listenerSpaceLocation, listeners, location))
            {
                if (attenuation != nullptr)
                {
                    if (spatialization == Spatialization_PositionOrientation)
                    {
                        AMPLITUDE_ASSERT(entity.Valid());
                        *gain *= attenuation->GetGain(entity.GetState(), &*listener);
                    }
                    else
                    {
                        *gain *= attenuation->GetGain(location, &*listener);
                    }
                }

                *pan = CalculatePan(listenerSpaceLocation);
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
    PriorityList::iterator FindInsertionPoint(PriorityList* list, const float priority)
    {
        PriorityList::reverse_iterator iter;
        for (iter = list->rbegin(); iter != list->rend(); ++iter)
        {
            if (const float p = iter->Priority(); p > priority)
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
        PriorityList::iterator insertionPoint,
        PriorityList* list,
        FreeList* realChannelFreeList,
        FreeList* virtualChannelFreeList,
        const bool paused)
    {
        ChannelInternalState* newChannel = nullptr;
        // Grab a free ChannelInternalState if there is one and the engine is not
        // paused. The engine is paused, grab a virtual channel for now, and it will
        // fix itself when the engine is not paused.
        if (!paused && !realChannelFreeList->empty())
        {
            newChannel = &realChannelFreeList->front();
            realChannelFreeList->pop_front();
            PriorityList::insert_before(*insertionPoint, *newChannel, &ChannelInternalState::priority_node);
        }
        else if (!virtualChannelFreeList->empty())
        {
            newChannel = &virtualChannelFreeList->front();
            virtualChannelFreeList->pop_front();
            PriorityList::insert_before(*insertionPoint, *newChannel, &ChannelInternalState::priority_node);
        }
        else if (&*insertionPoint != &list->back())
        {
            // If there are no free sounds, and the new sound is not the lowest priority
            // sound, evict the lowest priority sound.
            newChannel = &list->back();
            newChannel->Halt();

            // Move it to a new spot in the list if it needs to be moved.
            if (&*insertionPoint != newChannel)
            {
                list->pop_back();
                list->insert(insertionPoint, *newChannel);
            }
        }

        return newChannel;
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

    Channel Engine::Play(SwitchContainerHandle handle)
    {
        return Play(handle, AM_Vec3(0, 0, 0), 1.0f);
    }

    Channel Engine::Play(SwitchContainerHandle handle, const hmm_vec3& location)
    {
        return Play(handle, location, 1.0f);
    }

    Channel Engine::Play(SwitchContainerHandle handle, const hmm_vec3& location, const float userGain)
    {
        return PlayScopedSwitchContainer(handle, Entity(nullptr), location, userGain);
    }

    Channel Engine::Play(SwitchContainerHandle handle, const Entity& entity)
    {
        return Play(handle, entity, 1.0f);
    }

    Channel Engine::Play(SwitchContainerHandle handle, const Entity& entity, const float userGain)
    {
        return PlayScopedSwitchContainer(handle, entity, entity.GetLocation(), userGain);
    }

    Channel Engine::Play(CollectionHandle handle)
    {
        return Play(handle, AM_Vec3(0, 0, 0), 1.0f);
    }

    Channel Engine::Play(CollectionHandle handle, const hmm_vec3& location)
    {
        return Play(handle, location, 1.0f);
    }

    Channel Engine::Play(CollectionHandle handle, const hmm_vec3& location, const float userGain)
    {
        return PlayScopedCollection(handle, Entity(nullptr), location, userGain);
    }

    Channel Engine::Play(CollectionHandle handle, const Entity& entity)
    {
        return Play(handle, entity, 1.0f);
    }

    Channel Engine::Play(CollectionHandle handle, const Entity& entity, const float userGain)
    {
        return PlayScopedCollection(handle, entity, entity.GetLocation(), userGain);
    }

    Channel Engine::Play(SoundHandle handle)
    {
        return Play(handle, AM_Vec3(0, 0, 0), 1.0f);
    }

    Channel Engine::Play(SoundHandle handle, const hmm_vec3& location)
    {
        return Play(handle, location, 1.0f);
    }

    Channel Engine::Play(SoundHandle handle, const hmm_vec3& location, float userGain)
    {
        return PlayScopedSound(handle, Entity(nullptr), location, userGain);
    }

    Channel Engine::Play(SoundHandle handle, const Entity& entity)
    {
        return Play(handle, entity, 1.0f);
    }

    Channel Engine::Play(SoundHandle handle, const Entity& entity, float userGain)
    {
        return PlayScopedSound(handle, entity, entity.GetLocation(), userGain);
    }

    Channel Engine::Play(const std::string& name)
    {
        return Play(name, AM_Vec3(0, 0, 0), 1.0f);
    }

    Channel Engine::Play(const std::string& name, const hmm_vec3& location)
    {
        return Play(name, location, 1.0f);
    }

    Channel Engine::Play(const std::string& name, const hmm_vec3& location, const float userGain)
    {
        if (SoundHandle handle = GetSoundHandle(name))
        {
            return Play(handle, location, userGain);
        }

        if (CollectionHandle handle = GetCollectionHandle(name))
        {
            return Play(handle, location, userGain);
        }

        if (SwitchContainerHandle handle = GetSwitchContainerHandle(name))
        {
            return Play(handle, location, userGain);
        }

        CallLogFunc("[ERROR] Cannot play object: invalid name (%s).\n", name.c_str());
        return Channel(nullptr);
    }

    Channel Engine::Play(const std::string& name, const Entity& entity)
    {
        return Play(name, entity, 1.0f);
    }

    Channel Engine::Play(const std::string& name, const Entity& entity, const float userGain)
    {
        if (SoundHandle handle = GetSoundHandle(name))
        {
            return Play(handle, entity, userGain);
        }

        if (CollectionHandle handle = GetCollectionHandle(name))
        {
            return Play(handle, entity, userGain);
        }

        if (SwitchContainerHandle handle = GetSwitchContainerHandle(name))
        {
            return Play(handle, entity, userGain);
        }

        CallLogFunc("Cannot play sound: invalid name (%s)\n", name.c_str());
        return Channel(nullptr);
    }

    Channel Engine::Play(AmObjectID id)
    {
        return Play(id, AM_Vec3(0, 0, 0), 1.0f);
    }

    Channel Engine::Play(AmObjectID id, const hmm_vec3& location)
    {
        return Play(id, location, 1.0f);
    }

    Channel Engine::Play(AmObjectID id, const hmm_vec3& location, const float userGain)
    {
        if (SoundHandle handle = GetSoundHandle(id))
        {
            return Play(handle, location, userGain);
        }

        if (CollectionHandle handle = GetCollectionHandle(id))
        {
            return Play(handle, location, userGain);
        }

        if (SwitchContainerHandle handle = GetSwitchContainerHandle(id))
        {
            return Play(handle, location, userGain);
        }

        CallLogFunc("Cannot play sound: invalid ID (%u).\n", id);
        return Channel(nullptr);
    }

    Channel Engine::Play(AmObjectID id, const Entity& entity)
    {
        return Play(id, entity, 1.0f);
    }

    Channel Engine::Play(AmObjectID id, const Entity& entity, const float userGain)
    {
        if (SoundHandle handle = GetSoundHandle(id))
        {
            return Play(handle, entity, userGain);
        }

        if (CollectionHandle handle = GetCollectionHandle(id))
        {
            return Play(handle, entity, userGain);
        }

        if (SwitchContainerHandle handle = GetSwitchContainerHandle(id))
        {
            return Play(handle, entity, userGain);
        }

        CallLogFunc("Cannot play sound: invalid ID (%u)\n", id);
        return Channel(nullptr);
    }

    EventCanceler Engine::Trigger(EventHandle handle, const Entity& entity)
    {
        EventHandle event = handle;
        if (!event)
        {
            CallLogFunc("Cannot trigger event: Invalid event handle.\n");
            return EventCanceler(nullptr);
        }

        EventInstance instance = event->Trigger(entity);
        _state->running_events.push_back(instance);

        return EventCanceler(&instance);
    }

    EventCanceler Engine::Trigger(const std::string& name, const Entity& entity)
    {
        if (EventHandle handle = GetEventHandle(name))
        {
            return Trigger(handle, entity);
        }

        CallLogFunc("Cannot trigger event: invalid name (%s).\n", name.c_str());
        return EventCanceler(nullptr);
    }

    void Engine::SetSwitchState(SwitchHandle handle, AmObjectID stateId)
    {
        if (!handle)
        {
            CallLogFunc("[ERROR] Cannot update switch state: Invalid switch handle.\n");
            return;
        }

        handle->SetState(stateId);
    }

    void Engine::SetSwitchState(SwitchHandle handle, const std::string& stateName)
    {
        if (!handle)
        {
            CallLogFunc("[ERROR] Cannot update switch state: Invalid switch handle.\n");
            return;
        }

        handle->SetState(stateName);
    }

    void Engine::SetSwitchState(SwitchHandle handle, const SwitchState& state)
    {
        if (!handle)
        {
            CallLogFunc("[ERROR] Cannot update switch state: Invalid switch handle.\n");
            return;
        }

        handle->SetState(state);
    }

    void Engine::SetSwitchState(AmSwitchID id, AmObjectID stateId)
    {
        if (SwitchHandle handle = GetSwitchHandle(id))
        {
            return SetSwitchState(handle, stateId);
        }

        CallLogFunc("Cannot update switch: Invalid ID (%u).\n", id);
    }

    void Engine::SetSwitchState(AmSwitchID id, const std::string& stateName)
    {
        if (SwitchHandle handle = GetSwitchHandle(id))
        {
            return SetSwitchState(handle, stateName);
        }

        CallLogFunc("Cannot update switch: Invalid ID (%u).\n", id);
    }

    void Engine::SetSwitchState(AmSwitchID id, const SwitchState& state)
    {
        if (SwitchHandle handle = GetSwitchHandle(id))
        {
            return SetSwitchState(handle, state);
        }

        CallLogFunc("Cannot update switch: Invalid ID (%u).\n", id);
    }

    void Engine::SetSwitchState(const std::string name, AmObjectID stateId)
    {
        if (SwitchHandle handle = GetSwitchHandle(name))
        {
            return SetSwitchState(handle, stateId);
        }

        CallLogFunc("Cannot update switch: Invalid name (%s).\n", name.c_str());
    }

    void Engine::SetSwitchState(const std::string name, const std::string& stateName)
    {
        if (SwitchHandle handle = GetSwitchHandle(name))
        {
            return SetSwitchState(handle, stateName);
        }

        CallLogFunc("Cannot update switch: Invalid name (%s).\n", name.c_str());
    }

    void Engine::SetSwitchState(const std::string name, const SwitchState& state)
    {
        if (SwitchHandle handle = GetSwitchHandle(name))
        {
            return SetSwitchState(handle, state);
        }

        CallLogFunc("Cannot update switch: Invalid name (%s).\n", name.c_str());
    }

    void Engine::SetRtpcValue(RtpcHandle handle, double value)
    {
        if (!handle)
        {
            CallLogFunc("[ERROR] Cannot update RTPC value: Invalid RTPC handle.\n");
            return;
        }

        handle->SetValue(value);
    }

    void Engine::SetRtpcValue(AmRtpcID id, double value)
    {
        if (RtpcHandle handle = GetRtpcHandle(id))
        {
            return SetRtpcValue(handle, value);
        }

        CallLogFunc("[ERROR] Cannot update RTPC value: Invalid RTPC ID (%u).\n", id);
    }

    void Engine::SetRtpcValue(const std::string& name, double value)
    {
        if (RtpcHandle handle = GetRtpcHandle(name))
        {
            return SetRtpcValue(handle, value);
        }

        CallLogFunc("Cannot update RTPC value: Invalid RTPC name (%s).\n", name.c_str());
    }

    SwitchContainerHandle Engine::GetSwitchContainerHandle(const std::string& name) const
    {
        const auto pair = std::find_if(
            _state->switch_container_map.begin(), _state->switch_container_map.end(),
            [name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        if (pair == _state->switch_container_map.end())
        {
            return nullptr;
        }

        return pair->second.get();
    }

    SwitchContainerHandle Engine::GetSwitchContainerHandle(AmSwitchContainerID id) const
    {
        const auto pair = _state->switch_container_map.find(id);
        if (pair == _state->switch_container_map.end())
        {
            return nullptr;
        }

        return pair->second.get();
    }

    SwitchContainerHandle Engine::GetSwitchContainerHandleFromFile(AmOsString filename) const
    {
        const auto pair = _state->switch_container_id_map.find(filename);
        if (pair == _state->switch_container_id_map.end())
        {
            return nullptr;
        }

        return GetSwitchContainerHandle(pair->second);
    }

    CollectionHandle Engine::GetCollectionHandle(const std::string& name) const
    {
        const auto pair = std::find_if(
            _state->collection_map.begin(), _state->collection_map.end(),
            [name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        if (pair == _state->collection_map.end())
        {
            return nullptr;
        }

        return pair->second.get();
    }

    CollectionHandle Engine::GetCollectionHandle(AmCollectionID id) const
    {
        const auto pair = _state->collection_map.find(id);
        if (pair == _state->collection_map.end())
        {
            return nullptr;
        }

        return pair->second.get();
    }

    CollectionHandle Engine::GetCollectionHandleFromFile(AmOsString filename) const
    {
        const auto pair = _state->collection_id_map.find(filename);
        if (pair == _state->collection_id_map.end())
        {
            return nullptr;
        }

        return GetCollectionHandle(pair->second);
    }

    SoundHandle Engine::GetSoundHandle(const std::string& name) const
    {
        const auto pair = std::find_if(
            _state->sound_map.begin(), _state->sound_map.end(),
            [name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        if (pair == _state->sound_map.end())
        {
            return nullptr;
        }

        return pair->second.get();
    }

    SoundHandle Engine::GetSoundHandle(AmSoundID id) const
    {
        const auto pair = _state->sound_map.find(id);
        if (pair == _state->sound_map.end())
        {
            return nullptr;
        }

        return pair->second.get();
    }

    SoundHandle Engine::GetSoundHandleFromFile(AmOsString filename) const
    {
        const auto pair = _state->sound_id_map.find(filename);
        if (pair == _state->sound_id_map.end())
        {
            return nullptr;
        }

        return GetSoundHandle(pair->second);
    }

    EventHandle Engine::GetEventHandle(const std::string& name) const
    {
        const auto pair = std::find_if(
            _state->event_map.begin(), _state->event_map.end(),
            [name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        if (pair == _state->event_map.end())
        {
            return nullptr;
        }

        return pair->second.get();
    }

    EventHandle Engine::GetEventHandle(AmEventID id) const
    {
        const auto pair = _state->event_map.find(id);
        if (pair == _state->event_map.end())
        {
            return nullptr;
        }

        return pair->second.get();
    }

    EventHandle Engine::GetEventHandleFromFile(AmOsString filename) const
    {
        const auto pair = _state->event_id_map.find(filename);
        if (pair == _state->event_id_map.end())
        {
            return nullptr;
        }

        return GetEventHandle(pair->second);
    }

    AttenuationHandle Engine::GetAttenuationHandle(const std::string& name) const
    {
        const auto pair = std::find_if(
            _state->attenuation_map.begin(), _state->attenuation_map.end(),
            [name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        if (pair == _state->attenuation_map.end())
        {
            return nullptr;
        }

        return pair->second.get();
    }

    AttenuationHandle Engine::GetAttenuationHandle(AmAttenuationID id) const
    {
        const auto pair = _state->attenuation_map.find(id);
        if (pair == _state->attenuation_map.end())
        {
            return nullptr;
        }

        return pair->second.get();
    }

    AttenuationHandle Engine::GetAttenuationHandleFromFile(AmOsString filename) const
    {
        const auto pair = _state->attenuation_id_map.find(filename);
        if (pair == _state->attenuation_id_map.end())
        {
            return nullptr;
        }

        return GetAttenuationHandle(pair->second);
    }

    SwitchHandle Engine::GetSwitchHandle(const std::string& name) const
    {
        const auto pair = std::find_if(
            _state->switch_map.begin(), _state->switch_map.end(),
            [name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        if (pair == _state->switch_map.end())
        {
            return nullptr;
        }

        return pair->second.get();
    }

    SwitchHandle Engine::GetSwitchHandle(AmSwitchID id) const
    {
        const auto pair = _state->switch_map.find(id);
        if (pair == _state->switch_map.end())
        {
            return nullptr;
        }

        return pair->second.get();
    }

    SwitchHandle Engine::GetSwitchHandleFromFile(AmOsString filename) const
    {
        const auto pair = _state->switch_id_map.find(filename);
        if (pair == _state->switch_id_map.end())
        {
            return nullptr;
        }

        return GetSwitchHandle(pair->second);
    }

    RtpcHandle Engine::GetRtpcHandle(const std::string& name) const
    {
        const auto pair = std::find_if(
            _state->rtpc_map.begin(), _state->rtpc_map.end(),
            [name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        if (pair == _state->rtpc_map.end())
        {
            return nullptr;
        }

        return pair->second.get();
    }

    RtpcHandle Engine::GetRtpcHandle(AmRtpcID id) const
    {
        const auto pair = _state->rtpc_map.find(id);
        if (pair == _state->rtpc_map.end())
        {
            return nullptr;
        }

        return pair->second.get();
    }

    RtpcHandle Engine::GetRtpcHandleFromFile(AmOsString filename) const
    {
        const auto pair = _state->rtpc_id_map.find(filename);
        if (pair == _state->rtpc_id_map.end())
        {
            return nullptr;
        }

        return GetRtpcHandle(pair->second);
    }

    EffectHandle Engine::GetEffectHandle(const std::string& name) const
    {
        const auto pair = std::find_if(
            _state->effect_map.begin(), _state->effect_map.end(),
            [name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        if (pair == _state->effect_map.end())
        {
            return nullptr;
        }

        return pair->second.get();
    }

    EffectHandle Engine::GetEffectHandle(AmEffectID id) const
    {
        const auto pair = _state->effect_map.find(id);
        if (pair == _state->effect_map.end())
        {
            return nullptr;
        }

        return pair->second.get();
    }

    EffectHandle Engine::GetEffectHandleFromFile(AmOsString filename) const
    {
        const auto pair = _state->effect_id_map.find(filename);
        if (pair == _state->effect_id_map.end())
        {
            return nullptr;
        }

        return GetEffectHandle(pair->second);
    }

    void Engine::SetMasterGain(const float gain)
    {
        _state->master_gain = gain;
        _state->mixer.SetMasterGain(gain);
    }

    float Engine::GetMasterGain() const
    {
        return _state->master_gain;
    }

    void Engine::SetMute(const bool mute)
    {
        _state->mute = mute;
    }

    bool Engine::GetMute() const
    {
        return _state->mute;
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

    void Engine::RemoveListener(const Listener* listener)
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

    void Engine::RemoveEntity(const Entity* entity)
    {
        AMPLITUDE_ASSERT(entity->Valid());
        entity->GetState()->SetId(kAmInvalidObjectId);
        entity->GetState()->node.remove();
        _state->entity_state_free_list.push_back(entity->GetState());
    }

    Bus Engine::FindBus(AmString name) const
    {
        return Bus(FindBusInternalState(_state, name));
    }

    Bus Engine::FindBus(AmBusID id) const
    {
        return Bus(FindBusInternalState(_state, id));
    }

    void Engine::Pause(bool pause)
    {
        _state->paused = pause;

        PriorityList& list = _state->playing_channel_list;
        for (auto&& state : list)
        {
            if (!state.Paused() && state.IsReal())
            {
                if (pause)
                {
                    // Pause the real channel underlying this virtual channel. This freezes
                    // playback of the channel without marking it as paused from the audio
                    // engine's point of view, so that we know to restart it when the audio
                    // engine is not paused.
                    state.GetRealChannel().Pause();
                }
                else
                {
                    // Resumed all channels that were not explicitly paused.
                    state.GetRealChannel().Resume();
                }
            }
        }
    }

    void EraseFinishedSounds(EngineInternalState* state)
    {
        PriorityList& list = state->playing_channel_list;
        for (auto channelInternalState = list.begin(); channelInternalState != list.end();)
        {
            auto current = channelInternalState++;
            current->UpdateState();
            if (current->Stopped())
            {
                InsertIntoFreeList(state, &*current);
            }
        }
    }

    static void UpdateChannel(ChannelInternalState* channel, const EngineInternalState* state)
    {
        if (channel->Stopped())
            return;

        if (const SwitchContainer* switchContainer = channel->GetSwitchContainer(); switchContainer != nullptr)
        {
            const SwitchContainerDefinition* definition = switchContainer->GetSwitchContainerDefinition();

            float gain;
            hmm_vec2 pan;
            CalculateGainAndPan(
                &gain, &pan, switchContainer->GetGain().GetValue(), switchContainer->GetBus(), definition->spatialization(),
                switchContainer->GetAttenuation(), channel->GetEntity(), channel->GetLocation(), state->listener_list,
                channel->GetUserGain());
            channel->SetGain(gain);
            channel->SetPan(pan);
        }
        else if (const Collection* collection = channel->GetCollection(); collection != nullptr)
        {
            const CollectionDefinition* definition = collection->GetCollectionDefinition();

            float gain;
            hmm_vec2 pan;
            CalculateGainAndPan(
                &gain, &pan, collection->GetGain().GetValue(), collection->GetBus(), definition->spatialization(),
                collection->GetAttenuation(), channel->GetEntity(), channel->GetLocation(), state->listener_list, channel->GetUserGain());
            channel->SetGain(gain);
            channel->SetPan(pan);
        }
        else if (const Sound* sound = channel->GetSound(); sound != nullptr)
        {
            const SoundDefinition* definition = sound->GetSoundDefinition();

            float gain;
            hmm_vec2 pan;
            CalculateGainAndPan(
                &gain, &pan, sound->GetGain().GetValue(), sound->GetBus(), definition->spatialization(), sound->GetAttenuation(),
                channel->GetEntity(), channel->GetLocation(), state->listener_list, channel->GetUserGain());
            channel->SetGain(gain);
            channel->SetPan(pan);
        }
        else
        {
            AMPLITUDE_ASSERT(false);
        }
    }

    // If there are any free real channels, assign those to virtual channels that
    // need them. If the priority list has gaps (i.e. if there are real channels
    // that are lower priority than virtual channels) then move the lower priority
    // real channels to the higher priority virtual channels.
    static void UpdateRealChannels(PriorityList* priorityList, FreeList* realFreeList, FreeList* virtualFreeList)
    {
        auto reverseIterator = priorityList->rbegin();
        for (auto state = priorityList->begin(); state != priorityList->end(); ++state)
        {
            if (!state->IsReal())
            {
                // First check if there are any free real channels.
                if (!realFreeList->empty())
                {
                    // We have a free real channel. Assign this channel id to the channel
                    // that is trying to resume, clear the free channel, and push it into
                    // the virtual free list.
                    ChannelInternalState* freeChannel = &realFreeList->front();
                    state->Devirtualize(freeChannel);

                    freeChannel->Remove();
                    virtualFreeList->push_front(*freeChannel);

                    state->Resume();
                }
                else if (&*reverseIterator != &*state)
                {
                    // If there aren't any free channels, then scan from the back of the
                    // list for low priority real channels.
                    reverseIterator = std::find_if(
                        reverseIterator, PriorityList::reverse_iterator(state),
                        [](const ChannelInternalState& channel)
                        {
                            return channel.GetRealChannel().Valid();
                        });
                    if (reverseIterator == priorityList->rend())
                    {
                        // There is no more swapping that can be done. Return.
                        return;
                    }
                    // Found a real channel that we can give to the higher priority
                    // channel.
                    state->Devirtualize(&*reverseIterator);
                }
            }
        }
    }

    void Engine::AdvanceFrame(AmTime delta)
    {
        EraseFinishedSounds(_state);

        for (auto&& rtpc : _state->rtpc_map)
        {
            rtpc.second->Update(delta);
        }

        for (auto&& effect : _state->effect_map)
        {
            effect.second->Update();
        }

        for (auto&& state : _state->listener_list)
        {
            state.Update();
        }

        for (auto&& bus : _state->buses)
        {
            bus.ResetDuckGain();
        }
        for (auto&& bus : _state->buses)
        {
            bus.UpdateDuckGain(delta);
        }

        if (_state->master_bus)
        {
            const float masterGain = _state->mute ? 0.0f : _state->master_gain;
            _state->master_bus->AdvanceFrame(delta, masterGain);
        }

        PriorityList& list = _state->playing_channel_list;
        for (auto&& state : list)
        {
            UpdateChannel(&state, _state);
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

            event->AdvanceFrame(delta);
        }

        ++_state->current_frame;
        _state->total_time += delta;
    }

    AmTime Engine::GetTotalTime() const
    {
        return _state->total_time;
    }

    const struct Version* Engine::Version() const
    {
        return _state->version;
    }

    EngineInternalState* Engine::GetState() const
    {
        return _state;
    }

    const EngineConfigDefinition* Engine::GetEngineConfigDefinition() const
    {
        return Amplitude::GetEngineConfigDefinition(_configSrc.c_str());
    }

    Driver* Engine::GetDriver() const
    {
        return _audioDriver;
    }

    Channel Engine::PlayScopedSwitchContainer(
        SwitchContainerHandle handle, const Entity& entity, const hmm_vec3& location, const float userGain) const
    {
        if (!handle)
        {
            CallLogFunc("[ERROR] Cannot play switch container: Invalid switch container handle.\n");
            return Channel(nullptr);
        }

        const SwitchContainerDefinition* definition = handle->GetSwitchContainerDefinition();

        if (definition->scope() == Scope_Entity && !entity.Valid())
        {
            CallLogFunc("[ERROR] Cannot play a switch container in Entity scope. No entity defined.\n");
            return Channel(nullptr);
        }

        if (entity.Valid())
        {
            // Process the first entity update
            entity.GetState()->Update();
        }

        // Find where it belongs in the list.
        float gain;
        hmm_vec2 pan;
        CalculateGainAndPan(
            &gain, &pan, handle->GetGain().GetValue(), handle->GetBus(), definition->spatialization(), handle->GetAttenuation(), entity,
            location, _state->listener_list, userGain);
        const float priority = gain * handle->GetPriority().GetValue();
        const auto insertionPoint = FindInsertionPoint(&_state->playing_channel_list, priority);

        // Decide which ChannelInternalState object to use.
        ChannelInternalState* newChannel = FindFreeChannelInternalState(
            insertionPoint, &_state->playing_channel_list, &_state->real_channel_free_list, &_state->virtual_channel_free_list,
            _state->paused);

        // The sound could not be added to the list; not high enough priority.
        if (newChannel == nullptr)
        {
            return Channel(nullptr);
        }

        // Now that we have our new sound, set the data on it and update the next
        // pointers.
        newChannel->SetEntity(entity);
        newChannel->SetSwitchContainer(handle);
        newChannel->SetUserGain(userGain);

        // Attempt to play the sound if the engine is not paused.
        if (!_state->paused)
        {
            if (!newChannel->Play())
            {
                // Error playing the sound, put it back in the free list.
                InsertIntoFreeList(_state, newChannel);
                return Channel(nullptr);
            }
        }

        newChannel->SetGain(gain);
        newChannel->SetPan(pan);
        newChannel->SetLocation(location);

        return Channel(newChannel);
    }

    Channel Engine::PlayScopedCollection(
        CollectionHandle handle, const Entity& entity, const hmm_vec3& location, const float userGain) const
    {
        if (!handle)
        {
            CallLogFunc("[ERROR] Cannot play collection: Invalid collection handle\n");
            return Channel(nullptr);
        }

        const CollectionDefinition* definition = handle->GetCollectionDefinition();

        if (definition->scope() == Scope_Entity && !entity.Valid())
        {
            CallLogFunc("[ERROR] Cannot play a collection in Entity scope. No entity defined.\n");
            return Channel(nullptr);
        }

        if (entity.Valid())
        {
            // Process the first entity update
            entity.GetState()->Update();
        }

        // Find where it belongs in the list.
        float gain;
        hmm_vec2 pan;
        CalculateGainAndPan(
            &gain, &pan, handle->GetGain().GetValue(), handle->GetBus(), definition->spatialization(), handle->GetAttenuation(), entity,
            location, _state->listener_list, userGain);
        const float priority = gain * handle->GetPriority().GetValue();
        const auto insertionPoint = FindInsertionPoint(&_state->playing_channel_list, priority);

        // Decide which ChannelInternalState object to use.
        ChannelInternalState* newChannel = FindFreeChannelInternalState(
            insertionPoint, &_state->playing_channel_list, &_state->real_channel_free_list, &_state->virtual_channel_free_list,
            _state->paused);

        // The sound could not be added to the list; not high enough priority.
        if (newChannel == nullptr)
        {
            return Channel(nullptr);
        }

        // Now that we have our new sound, set the data on it and update the next
        // pointers.
        newChannel->SetEntity(entity);
        newChannel->SetCollection(handle);
        newChannel->SetUserGain(userGain);

        // Attempt to play the sound if the engine is not paused.
        if (!_state->paused)
        {
            if (!newChannel->Play())
            {
                // Error playing the sound, put it back in the free list.
                InsertIntoFreeList(_state, newChannel);
                return Channel(nullptr);
            }
        }

        newChannel->SetGain(gain);
        newChannel->SetPan(pan);
        newChannel->SetLocation(location);

        return Channel(newChannel);
    }

    Channel Engine::PlayScopedSound(SoundHandle handle, const Entity& entity, const hmm_vec3& location, float userGain) const
    {
        if (!handle)
        {
            CallLogFunc("[ERROR] Cannot play sound: Invalid sound handle\n");
            return Channel(nullptr);
        }

        const SoundDefinition* definition = handle->GetSoundDefinition();

        if (definition->scope() == Scope_Entity && !entity.Valid())
        {
            CallLogFunc("[ERROR] Cannot play a sound in Entity scope. No entity defined.\n");
            return Channel(nullptr);
        }

        if (entity.Valid())
        {
            // Process the first entity update
            entity.GetState()->Update();
        }

        // Find where it belongs in the list.
        float gain;
        hmm_vec2 pan;
        CalculateGainAndPan(
            &gain, &pan, handle->GetGain().GetValue(), handle->GetBus(), definition->spatialization(), handle->GetAttenuation(), entity,
            location, _state->listener_list, userGain);
        const float priority = gain * handle->GetPriority().GetValue();
        const auto insertionPoint = FindInsertionPoint(&_state->playing_channel_list, priority);

        // Decide which ChannelInternalState object to use.
        ChannelInternalState* newChannel = FindFreeChannelInternalState(
            insertionPoint, &_state->playing_channel_list, &_state->real_channel_free_list, &_state->virtual_channel_free_list,
            _state->paused);

        // The sound could not be added to the list; not high enough priority.
        if (newChannel == nullptr)
        {
            return Channel(nullptr);
        }

        // Now that we have our new sound, set the data on it and update the next
        // pointers.
        newChannel->SetEntity(entity);
        newChannel->SetSound(handle);
        newChannel->SetUserGain(userGain);

        // Attempt to play the sound if the engine is not paused.
        if (!_state->paused)
        {
            if (!newChannel->Play())
            {
                // Error playing the sound, put it back in the free list.
                InsertIntoFreeList(_state, newChannel);
                return Channel(nullptr);
            }
        }

        newChannel->SetGain(gain);
        newChannel->SetPan(pan);
        newChannel->SetLocation(location);

        return Channel(newChannel);
    }
} // namespace SparkyStudios::Audio::Amplitude
