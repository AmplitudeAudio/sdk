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

// clang-format off
#include <Core/Codecs/WAV/Codec.h>
#include <Core/Codecs/OGG/Codec.h>
#include <Core/Codecs/FLAC/Codec.h>
#include <Core/Codecs/MP3/Codec.h>
// clang-format on

#pragma endregion

#pragma region Default Drivers

#include <Core/Drivers/MiniAudio/Driver.h>

#pragma endregion

#pragma region Default Sound Processors

#include <Mixer/SoundProcessors/EffectProcessor.h>
#include <Mixer/SoundProcessors/EnvironmentProcessor.h>
#include <Mixer/SoundProcessors/ObstructionProcessor.h>
#include <Mixer/SoundProcessors/OcclusionProcessor.h>
#include <Mixer/SoundProcessors/PassThroughProcessor.h>

#pragma endregion

namespace SparkyStudios::Audio::Amplitude
{
    typedef flatbuffers::Vector<uint64_t> BusIdList;
    typedef flatbuffers::Vector<flatbuffers::Offset<DuckBusDefinition>> DuckBusDefinitionList;

    bool LoadFile(const AmOsString& filename, AmString* dest)
    {
        if (filename.empty())
        {
            CallLogFunc("[ERROR] The filename is empty");
            return false;
        }

        // load into memory:
        MemoryFile mf;
        if (const AmResult result = mf.OpenToMem(filename); result != AM_ERROR_NO_ERROR)
        {
            CallLogFunc("[ERROR] LoadFile fail on '" AM_OS_CHAR_FMT "'", filename.c_str());
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
        , _defaultListener(nullptr)
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
                [&id](const BusInternalState& bus)
                {
                    return bus.GetId() == id;
                });
            it != state->buses.end())
        {
            return &*it;
        }

        return nullptr;
    }

    BusInternalState* FindBusInternalState(EngineInternalState* state, const AmString& name)
    {
        if (const auto it = std::find_if(
                state->buses.begin(), state->buses.end(),
                [&name](const BusInternalState& bus)
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

    static void InitializeEnvironmentFreeList(
        std::vector<EnvironmentInternalState*>* environmentStateFreeList, EnvironmentStateVector* environmentList, const AmUInt32 listSize)
    {
        environmentList->resize(listSize);
        environmentStateFreeList->reserve(listSize);
        for (size_t i = 0; i < listSize; ++i)
        {
            EnvironmentInternalState& environment = (*environmentList)[i];
            environmentStateFreeList->push_back(&environment);
        }
    }

    bool Engine::Initialize(const AmOsString& configFile)
    {
        const std::filesystem::path configFilePath = _loader.ResolvePath(configFile);
        if (!LoadFile(configFilePath.c_str(), &_configSrc))
        {
            CallLogFunc("[ERROR] Could not load audio config file at path '" AM_OS_CHAR_FMT "'.\n", configFile.c_str());
            return false;
        }
        return Initialize(GetEngineConfigDefinition());
    }

    bool Engine::Initialize(const EngineConfigDefinition* config)
    {
        // Lock drivers, codecs and processors registries
        Driver::LockRegistry();
        Codec::LockRegistry();
        SoundProcessor::LockRegistry();

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
            Deinitialize();
            return false;
        }

        // Initialize audio mixer
        if (!_state->mixer.Init(config))
        {
            CallLogFunc("[ERROR] Could not initialize the audio mixer.\n");
            Deinitialize();
            return false;
        }

        // Initialize the channel internal data.
        InitializeChannelFreeLists(
            &_state->real_channel_free_list, &_state->virtual_channel_free_list, &_state->channel_state_memory,
            config->mixer()->virtual_channels(), config->mixer()->active_channels());

        // Initialize the listener internal data.
        InitializeListenerFreeList(&_state->listener_state_free_list, &_state->listener_state_memory, config->game()->listeners());

        // Initialize the entity internal data.
        InitializeEntityFreeList(&_state->entity_state_free_list, &_state->entity_state_memory, config->game()->entities());

        // Initialize the environment internal data.
        InitializeEnvironmentFreeList(
            &_state->environment_state_free_list, &_state->environment_state_memory, config->game()->environments());

        // Load the audio buses.
        const std::filesystem::path busesFilePath = _loader.ResolvePath(config->buses_file()->c_str());
        if (!LoadFile(busesFilePath.c_str(), &_state->buses_source))
        {
            CallLogFunc("[ERROR] Could not load audio bus file.\n");
            Deinitialize();
            return false;
        }
        const BusDefinitionList* busDefList = Amplitude::GetBusDefinitionList(_state->buses_source.c_str());
        const auto busCount = busDefList->buses()->size();
        _state->buses.resize(busCount);
        for (flatbuffers::uoffset_t i = 0; i < busCount; ++i)
        {
            _state->buses[i].Initialize(busDefList->buses()->Get(i));
        }

        // Set up the children and ducking pointers.
        for (auto& bus : _state->buses)
        {
            const BusDefinition* def = bus.GetBusDefinition();
            if (!PopulateChildBuses(_state, &bus, def->child_buses()))
            {
                Deinitialize();
                return false;
            }
            if (!PopulateDuckBuses(_state, &bus, def->duck_buses()))
            {
                Deinitialize();
                return false;
            }
        }

        // Fetch the master bus with name
        _state->master_bus = FindBusInternalState(_state, kAmMasterBusId);
        if (!_state->master_bus)
        {
            // Fetch the master bus by ID
            _state->master_bus = FindBusInternalState(_state, "master");
            if (!_state->master_bus)
            {
                CallLogFunc("[ERROR] Unable to find a master bus.\n");
                Deinitialize();
                return false;
            }
        }

        // Set the listener fetch mode
        _state->listener_fetch_mode = config->game()->listener_fetch_mode();

        // Doppler effect settings
        _state->sound_speed = config->game()->sound_speed();
        _state->doppler_factor = config->game()->doppler_factor();

        // Samples per streams
        _state->samples_per_stream = config->mixer()->samples_per_stream();

        // Dynamic sample rate conversion
        _state->sample_rate_conversion_quality = config->mixer()->sample_rate_conversion_quality();

        // Set the game engine up axis
        _state->up_axis = config->game()->up_axis();

        // Save obstruction/occlusion configurations
        _state->obstruction_config.Init(config->game()->obstruction());
        _state->occlusion_config.Init(config->game()->occlusion());

        _state->paused = false;
        _state->mute = false;
        _state->master_gain = 1.0f;

        // Open the audio device through the driver
        return _audioDriver->Open(_state->mixer.GetDeviceDescription());
    }

    bool Engine::Deinitialize()
    {
        _state->stopping = true;

        if (_state->mixer.IsInitialized())
            _state->mixer.StopAll();

        // Unload sound banks
        UnloadSoundBanks();

        delete _state;
        _state = nullptr;

        if (_audioDriver)
            // Close the audio device through the driver
            return _audioDriver->Close();
        else
            return true;
    }

    bool Engine::IsInitialized() const
    {
        // An initialized engine have a running state
        return _state != nullptr && !_state->stopping;
    }

    void Engine::SetFileLoader(const FileLoader& loader)
    {
        _loader = loader;
    }

    const FileLoader* Engine::GetFileLoader() const
    {
        return &_loader;
    }

    bool Engine::LoadSoundBank(const AmOsString& filename)
    {
        AmBankID outID = kAmInvalidObjectId;
        return LoadSoundBank(filename, outID);
    }

    bool Engine::LoadSoundBank(const AmOsString& filename, AmBankID& outID)
    {
        outID = kAmInvalidObjectId;
        bool success = true;

        if (const auto findIt = _state->sound_bank_id_map.find(filename); findIt == _state->sound_bank_id_map.end() ||
            (findIt != _state->sound_bank_id_map.end() && _state->sound_bank_map.find(findIt->second) == _state->sound_bank_map.end()))
        {
            auto soundBank = std::make_unique<SoundBank>();
            success = soundBank->Initialize(filename, this);

            if (success)
            {
                soundBank->GetRefCounter()->Increment();

                AmBankID id = soundBank->GetId();
                _state->sound_bank_id_map[filename] = id;
                _state->sound_bank_map[id] = std::move(soundBank);
                outID = id;
            }
        }
        else
        {
            _state->sound_bank_map[findIt->second]->GetRefCounter()->Increment();
        }

        return success;
    }

    bool Engine::LoadSoundBankFromMemory(const char* fileData)
    {
        AmBankID outID = kAmInvalidObjectId;
        return LoadSoundBankFromMemory(fileData, outID);
    }

    bool Engine::LoadSoundBankFromMemory(const char* fileData, AmBankID& outID)
    {
        outID = kAmInvalidObjectId;
        bool success = true;

        auto soundBank = std::make_unique<SoundBank>(fileData);
        AmOsString filename = AM_STRING_TO_OS_STRING(soundBank->GetName());
        if (const auto findIt = _state->sound_bank_id_map.find(filename); findIt == _state->sound_bank_id_map.end() ||
            (findIt != _state->sound_bank_id_map.end() && _state->sound_bank_map.find(findIt->second) == _state->sound_bank_map.end()))
        {
            success = soundBank->InitializeFromMemory(fileData, this);

            if (success)
            {
                soundBank->GetRefCounter()->Increment();

                AmBankID id = soundBank->GetId();
                _state->sound_bank_id_map[filename] = id;
                _state->sound_bank_map[id] = std::move(soundBank);
                outID = id;
            }
        }
        else
        {
            _state->sound_bank_map[findIt->second]->GetRefCounter()->Increment();
        }

        return success;
    }

    bool Engine::LoadSoundBankFromMemoryView(void* ptr, AmSize size)
    {
        AmBankID outID = kAmInvalidObjectId;
        return LoadSoundBankFromMemoryView(ptr, size, outID);
    }

    bool Engine::LoadSoundBankFromMemoryView(void* ptr, AmSize size, AmBankID& outID)
    {
        outID = kAmInvalidObjectId;
        bool success = true;

        MemoryFile mf;
        AmString dst;

        mf.OpenMem(static_cast<AmConstUInt8Buffer>(ptr), size, false, false);
        dst.assign(size + 1, 0);

        const AmUInt32 len = mf.Read(reinterpret_cast<AmUInt8Buffer>(&dst[0]), mf.Length());
        if (len != size)
        {
            return false;
        }

        auto soundBank = std::make_unique<SoundBank>(dst);
        AmOsString filename = AM_STRING_TO_OS_STRING(soundBank->GetName());
        if (const auto findIt = _state->sound_bank_id_map.find(filename); findIt == _state->sound_bank_id_map.end() ||
            (findIt != _state->sound_bank_id_map.end() && _state->sound_bank_map.find(findIt->second) == _state->sound_bank_map.end()))
        {
            success = soundBank->InitializeFromMemory(dst.c_str(), this);

            if (success)
            {
                soundBank->GetRefCounter()->Increment();

                const AmBankID id = soundBank->GetId();
                _state->sound_bank_id_map[filename] = id;
                _state->sound_bank_map[id] = std::move(soundBank);
                outID = id;
            }
        }
        else
        {
            _state->sound_bank_map[findIt->second]->GetRefCounter()->Increment();
        }

        return success;
    }

    void Engine::UnloadSoundBank(const AmOsString& filename)
    {
        if (const auto findIt = _state->sound_bank_id_map.find(filename); findIt == _state->sound_bank_id_map.end())
        {
            CallLogFunc("[ERROR] Error while deinitializing SoundBank " AM_OS_CHAR_FMT " - sound bank not loaded.\n", filename.c_str());
            AMPLITUDE_ASSERT(0);
        }
        else
        {
            UnloadSoundBank(findIt->second);
        }
    }

    void Engine::UnloadSoundBank(AmBankID id)
    {
        if (const auto findIt = _state->sound_bank_map.find(id); findIt == _state->sound_bank_map.end())
        {
            CallLogFunc("[ERROR] Error while deinitializing SoundBank with ID %d - sound bank not loaded.\n", id);
            AMPLITUDE_ASSERT(0);
        }
        else if (findIt->second->GetRefCounter()->Decrement() == 0)
        {
            findIt->second->Deinitialize(this);
            _state->sound_bank_map.erase(id);
        }
    }

    void Engine::UnloadSoundBanks()
    {
        for (const auto& item : _state->sound_bank_map)
        {
            item.second->Deinitialize(this);
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
        const hmm_vec3& location,
        eListenerFetchMode fetchMode)
    {
        if (listeners.empty())
        {
            return false;
        }

        const hmm_vec4 location4 = AM_Vec4v(location, 1.0f);

        switch (fetchMode)
        {
        case eListenerFetchMode_None:
            return false;

        case eListenerFetchMode_Nearest:
            [[fallthrough]];
        case eListenerFetchMode_Farest:
            {
                auto listener = listeners.cbegin();
                *listenerSpaceLocation = AM_Multiply(listener->GetInverseMatrix(), location4).XYZ;
                *distanceSquared = AM_LengthSquared(*listenerSpaceLocation);
                *bestListener = listener;

                for (++listener; listener != listeners.cend(); ++listener)
                {
                    const hmm_vec3 transformedLocation = AM_Multiply(listener->GetInverseMatrix(), location4).XYZ;
                    if (const float magnitudeSquared = AM_LengthSquared(transformedLocation);
                        fetchMode == eListenerFetchMode_Nearest ? magnitudeSquared < *distanceSquared : magnitudeSquared > *distanceSquared)
                    {
                        *bestListener = listener;
                        *distanceSquared = magnitudeSquared;
                        *listenerSpaceLocation = transformedLocation;
                    }
                }
            }
            break;

        case eListenerFetchMode_First:
            {
                auto listener = listeners.cbegin();
                const hmm_mat4& mat = listener->GetInverseMatrix();
                *listenerSpaceLocation = AM_Multiply(mat, location4).XYZ;
                *distanceSquared = AM_LengthSquared(*listenerSpaceLocation);
                *bestListener = listener;
            }
            break;

        case eListenerFetchMode_Last:
            {
                auto listener = listeners.cend();
                const hmm_mat4& mat = listener->GetInverseMatrix();
                *listenerSpaceLocation = AM_Multiply(mat, location4).XYZ;
                *distanceSquared = AM_LengthSquared(*listenerSpaceLocation);
                *bestListener = listener;
            }
            break;

        case eListenerFetchMode_Default:
            {
                ListenerInternalState* state = Engine::GetInstance()->GetDefaultListener().GetState();
                if (state == nullptr)
                {
                    return false;
                }

                for (auto listener = listeners.cbegin(); listener != listeners.cend(); ++listener)
                {
                    if (listener->GetId() == state->GetId())
                    {
                        const hmm_mat4& mat = state->GetInverseMatrix();
                        *listenerSpaceLocation = AM_Multiply(mat, location4).XYZ;
                        *distanceSquared = AM_LengthSquared(*listenerSpaceLocation);
                        *bestListener = listener;
                        return true;
                    }
                }

                return false;
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

        switch (amEngine->GetState()->up_axis)
        {
        default:
        case eGameEngineUpAxis_Y:
            return AM_Vec2(AM_Dot(AM_Vec3(1, 0, 0), direction), AM_Dot(AM_Vec3(0, 0, 1), direction));

        case eGameEngineUpAxis_Z:
            return AM_Vec2(AM_Dot(AM_Vec3(1, 0, 0), direction), AM_Dot(AM_Vec3(0, 1, 0), direction));
        }
    }

    static void CalculateGainPanPitch(
        float* gain,
        hmm_vec2* pan,
        AmReal32* pitch,
        const ChannelInternalState* channel,
        const float soundGain,
        const BusInternalState* bus,
        const Spatialization spatialization,
        const Attenuation* attenuation,
        const Entity& entity,
        const hmm_vec3& location,
        const ListenerList& listeners,
        const float userGain,
        eListenerFetchMode fetchMode)
    {
        *gain = soundGain * bus->GetGain() * userGain;
        if (spatialization == Spatialization_Position || spatialization == Spatialization_PositionOrientation)
        {
            ListenerList::const_iterator listener;
            float distanceSquared;
            hmm_vec3 listenerSpaceLocation;
            if (BestListener(&listener, &distanceSquared, &listenerSpaceLocation, listeners, location, fetchMode))
            {
                if (attenuation != nullptr)
                {
                    if (spatialization == Spatialization_PositionOrientation)
                    {
                        AMPLITUDE_ASSERT(entity.Valid());
                        *gain *= attenuation->GetGain(entity, Listener(const_cast<ListenerInternalState*>(&*listener)));
                    }
                    else
                    {
                        *gain *= attenuation->GetGain(location, Listener(const_cast<ListenerInternalState*>(&*listener)));
                    }
                }

                *pan = CalculatePan(listenerSpaceLocation);
                *pitch = channel != nullptr ? channel->GetDopplerFactor(listener->GetId()) : 1.0f;
            }
            else
            {
                *gain = 0.0f;
                *pan = AM_Vec2(0, 0);
                *pitch = 1.0f;
            }
        }
        else
        {
            *pan = AM_Vec2(0, 0);
            *pitch = 1.0f;
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
        PriorityList::reverse_iterator it;
        for (it = list->rbegin(); it != list->rend(); ++it)
        {
            if (const float p = it->Priority(); p > priority)
            {
                break;
            }
        }
        return it.base();
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

    Channel Engine::Play(SwitchContainerHandle handle) const
    {
        return Play(handle, AM_Vec3(0, 0, 0), 1.0f);
    }

    Channel Engine::Play(SwitchContainerHandle handle, const hmm_vec3& location) const
    {
        return Play(handle, location, 1.0f);
    }

    Channel Engine::Play(SwitchContainerHandle handle, const hmm_vec3& location, const float userGain) const
    {
        return PlayScopedSwitchContainer(handle, Entity(nullptr), location, userGain);
    }

    Channel Engine::Play(SwitchContainerHandle handle, const Entity& entity) const
    {
        return Play(handle, entity, 1.0f);
    }

    Channel Engine::Play(SwitchContainerHandle handle, const Entity& entity, const float userGain) const
    {
        return PlayScopedSwitchContainer(handle, entity, entity.GetLocation(), userGain);
    }

    Channel Engine::Play(CollectionHandle handle) const
    {
        return Play(handle, AM_Vec3(0, 0, 0), 1.0f);
    }

    Channel Engine::Play(CollectionHandle handle, const hmm_vec3& location) const
    {
        return Play(handle, location, 1.0f);
    }

    Channel Engine::Play(CollectionHandle handle, const hmm_vec3& location, const float userGain) const
    {
        return PlayScopedCollection(handle, Entity(nullptr), location, userGain);
    }

    Channel Engine::Play(CollectionHandle handle, const Entity& entity) const
    {
        return Play(handle, entity, 1.0f);
    }

    Channel Engine::Play(CollectionHandle handle, const Entity& entity, const float userGain) const
    {
        return PlayScopedCollection(handle, entity, entity.GetLocation(), userGain);
    }

    Channel Engine::Play(SoundHandle handle) const
    {
        return Play(handle, AM_Vec3(0, 0, 0), 1.0f);
    }

    Channel Engine::Play(SoundHandle handle, const hmm_vec3& location) const
    {
        return Play(handle, location, 1.0f);
    }

    Channel Engine::Play(SoundHandle handle, const hmm_vec3& location, float userGain) const
    {
        return PlayScopedSound(handle, Entity(nullptr), location, userGain);
    }

    Channel Engine::Play(SoundHandle handle, const Entity& entity) const
    {
        return Play(handle, entity, 1.0f);
    }

    Channel Engine::Play(SoundHandle handle, const Entity& entity, float userGain) const
    {
        return PlayScopedSound(handle, entity, entity.GetLocation(), userGain);
    }

    Channel Engine::Play(const AmString& name) const
    {
        return Play(name, AM_Vec3(0, 0, 0), 1.0f);
    }

    Channel Engine::Play(const AmString& name, const hmm_vec3& location) const
    {
        return Play(name, location, 1.0f);
    }

    Channel Engine::Play(const AmString& name, const hmm_vec3& location, const float userGain) const
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

    Channel Engine::Play(const AmString& name, const Entity& entity) const
    {
        return Play(name, entity, 1.0f);
    }

    Channel Engine::Play(const AmString& name, const Entity& entity, const float userGain) const
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

    Channel Engine::Play(AmObjectID id) const
    {
        return Play(id, AM_Vec3(0, 0, 0), 1.0f);
    }

    Channel Engine::Play(AmObjectID id, const hmm_vec3& location) const
    {
        return Play(id, location, 1.0f);
    }

    Channel Engine::Play(AmObjectID id, const hmm_vec3& location, const float userGain) const
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

    Channel Engine::Play(AmObjectID id, const Entity& entity) const
    {
        return Play(id, entity, 1.0f);
    }

    Channel Engine::Play(AmObjectID id, const Entity& entity, const float userGain) const
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

    void Engine::StopAll() const
    {
        for (auto&& channel : _state->channel_state_memory)
        {
            if (channel.Valid() && channel.Playing())
            {
                channel.Halt();
            }
        }
    }

    EventCanceler Engine::Trigger(EventHandle handle, const Entity& entity) const
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

    EventCanceler Engine::Trigger(const AmString& name, const Entity& entity) const
    {
        if (EventHandle handle = GetEventHandle(name))
        {
            return Trigger(handle, entity);
        }

        CallLogFunc("Cannot trigger event: invalid name (%s).\n", name.c_str());
        return EventCanceler(nullptr);
    }

    void Engine::SetSwitchState(SwitchHandle handle, AmObjectID stateId) const
    {
        if (!handle)
        {
            CallLogFunc("[ERROR] Cannot update switch state: Invalid switch handle.\n");
            return;
        }

        handle->SetState(stateId);
    }

    void Engine::SetSwitchState(SwitchHandle handle, const AmString& stateName) const
    {
        if (!handle)
        {
            CallLogFunc("[ERROR] Cannot update switch state: Invalid switch handle.\n");
            return;
        }

        handle->SetState(stateName);
    }

    void Engine::SetSwitchState(SwitchHandle handle, const SwitchState& state) const
    {
        if (!handle)
        {
            CallLogFunc("[ERROR] Cannot update switch state: Invalid switch handle.\n");
            return;
        }

        handle->SetState(state);
    }

    void Engine::SetSwitchState(AmSwitchID id, AmObjectID stateId) const
    {
        if (SwitchHandle handle = GetSwitchHandle(id))
        {
            return SetSwitchState(handle, stateId);
        }

        CallLogFunc("Cannot update switch: Invalid ID (%u).\n", id);
    }

    void Engine::SetSwitchState(AmSwitchID id, const AmString& stateName) const
    {
        if (SwitchHandle handle = GetSwitchHandle(id))
        {
            return SetSwitchState(handle, stateName);
        }

        CallLogFunc("Cannot update switch: Invalid ID (%u).\n", id);
    }

    void Engine::SetSwitchState(AmSwitchID id, const SwitchState& state) const
    {
        if (SwitchHandle handle = GetSwitchHandle(id))
        {
            return SetSwitchState(handle, state);
        }

        CallLogFunc("Cannot update switch: Invalid ID (%u).\n", id);
    }

    void Engine::SetSwitchState(const AmString& name, AmObjectID stateId) const
    {
        if (SwitchHandle handle = GetSwitchHandle(name))
        {
            return SetSwitchState(handle, stateId);
        }

        CallLogFunc("Cannot update switch: Invalid name (%s).\n", name.c_str());
    }

    void Engine::SetSwitchState(const AmString& name, const AmString& stateName) const
    {
        if (SwitchHandle handle = GetSwitchHandle(name))
        {
            return SetSwitchState(handle, stateName);
        }

        CallLogFunc("Cannot update switch: Invalid name (%s).\n", name.c_str());
    }

    void Engine::SetSwitchState(const AmString& name, const SwitchState& state) const
    {
        if (SwitchHandle handle = GetSwitchHandle(name))
        {
            return SetSwitchState(handle, state);
        }

        CallLogFunc("Cannot update switch: Invalid name (%s).\n", name.c_str());
    }

    void Engine::SetRtpcValue(RtpcHandle handle, double value) const
    {
        if (!handle)
        {
            CallLogFunc("[ERROR] Cannot update RTPC value: Invalid RTPC handle.\n");
            return;
        }

        handle->SetValue(value);
    }

    void Engine::SetRtpcValue(AmRtpcID id, double value) const
    {
        if (RtpcHandle handle = GetRtpcHandle(id))
        {
            return SetRtpcValue(handle, value);
        }

        CallLogFunc("[ERROR] Cannot update RTPC value: Invalid RTPC ID (%u).\n", id);
    }

    void Engine::SetRtpcValue(const AmString& name, double value) const
    {
        if (RtpcHandle handle = GetRtpcHandle(name))
        {
            return SetRtpcValue(handle, value);
        }

        CallLogFunc("Cannot update RTPC value: Invalid RTPC name (%s).\n", name.c_str());
    }

    SwitchContainerHandle Engine::GetSwitchContainerHandle(const AmString& name) const
    {
        const auto pair = std::find_if(
            _state->switch_container_map.begin(), _state->switch_container_map.end(),
            [&name](const auto& item)
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

    SwitchContainerHandle Engine::GetSwitchContainerHandleFromFile(const AmOsString& filename) const
    {
        const auto pair = _state->switch_container_id_map.find(filename);
        if (pair == _state->switch_container_id_map.end())
        {
            return nullptr;
        }

        return GetSwitchContainerHandle(pair->second);
    }

    CollectionHandle Engine::GetCollectionHandle(const AmString& name) const
    {
        const auto pair = std::find_if(
            _state->collection_map.begin(), _state->collection_map.end(),
            [&name](const auto& item)
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

    CollectionHandle Engine::GetCollectionHandleFromFile(const AmOsString& filename) const
    {
        const auto pair = _state->collection_id_map.find(filename);
        if (pair == _state->collection_id_map.end())
        {
            return nullptr;
        }

        return GetCollectionHandle(pair->second);
    }

    SoundHandle Engine::GetSoundHandle(const AmString& name) const
    {
        const auto pair = std::find_if(
            _state->sound_map.begin(), _state->sound_map.end(),
            [&name](const auto& item)
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

    SoundHandle Engine::GetSoundHandleFromFile(const AmOsString& filename) const
    {
        const auto pair = _state->sound_id_map.find(filename);
        if (pair == _state->sound_id_map.end())
        {
            return nullptr;
        }

        return GetSoundHandle(pair->second);
    }

    SoundObjectHandle Engine::GetSoundObjectHandle(const AmString& name) const
    {
        if (const SoundHandle handle = GetSoundHandle(name))
        {
            return handle;
        }

        if (const CollectionHandle handle = GetCollectionHandle(name))
        {
            return handle;
        }

        if (const SwitchContainerHandle handle = GetSwitchContainerHandle(name))
        {
            return handle;
        }
    }

    SoundObjectHandle Engine::GetSoundObjectHandle(AmSoundID id) const
    {
        if (const SoundHandle handle = GetSoundHandle(id))
        {
            return handle;
        }

        if (const CollectionHandle handle = GetCollectionHandle(id))
        {
            return handle;
        }

        if (const SwitchContainerHandle handle = GetSwitchContainerHandle(id))
        {
            return handle;
        }
    }

    SoundObjectHandle Engine::GetSoundObjectHandleFromFile(const AmOsString& filename) const
    {
        if (const SoundHandle handle = GetSoundHandleFromFile(filename))
        {
            return handle;
        }

        if (const CollectionHandle handle = GetCollectionHandleFromFile(filename))
        {
            return handle;
        }

        if (const SwitchContainerHandle handle = GetSwitchContainerHandleFromFile(filename))
        {
            return handle;
        }
    }

    EventHandle Engine::GetEventHandle(const AmString& name) const
    {
        const auto pair = std::find_if(
            _state->event_map.begin(), _state->event_map.end(),
            [&name](const auto& item)
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

    EventHandle Engine::GetEventHandleFromFile(const AmOsString& filename) const
    {
        const auto pair = _state->event_id_map.find(filename);
        if (pair == _state->event_id_map.end())
        {
            return nullptr;
        }

        return GetEventHandle(pair->second);
    }

    AttenuationHandle Engine::GetAttenuationHandle(const AmString& name) const
    {
        const auto pair = std::find_if(
            _state->attenuation_map.begin(), _state->attenuation_map.end(),
            [&name](const auto& item)
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

    SwitchHandle Engine::GetSwitchHandle(const AmString& name) const
    {
        const auto pair = std::find_if(
            _state->switch_map.begin(), _state->switch_map.end(),
            [&name](const auto& item)
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

    RtpcHandle Engine::GetRtpcHandle(const AmString& name) const
    {
        const auto pair = std::find_if(
            _state->rtpc_map.begin(), _state->rtpc_map.end(),
            [&name](const auto& item)
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

    EffectHandle Engine::GetEffectHandle(const AmString& name) const
    {
        const auto pair = std::find_if(
            _state->effect_map.begin(), _state->effect_map.end(),
            [&name](const auto& item)
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

    void Engine::SetMasterGain(const float gain) const
    {
        _state->master_gain = gain;
        _state->mixer.SetMasterGain(gain);
    }

    float Engine::GetMasterGain() const
    {
        return _state->master_gain;
    }

    void Engine::SetMute(const bool mute) const
    {
        _state->mute = mute;
    }

    bool Engine::GetMute() const
    {
        return _state->mute;
    }

    void Engine::SetDefaultListener(const Listener* listener)
    {
        if (listener->Valid())
        {
            _defaultListener = listener->GetState();
        }
    }

    void Engine::SetDefaultListener(AmListenerID id)
    {
        if (id != kAmInvalidObjectId)
        {
            if (const auto findIt = std::find_if(
                    _state->listener_state_memory.begin(), _state->listener_state_memory.end(),
                    [&id](const ListenerInternalState& state)
                    {
                        return state.GetId() == id;
                    });
                findIt != _state->listener_state_memory.end())
            {
                _defaultListener = (&*findIt);
            }
        }
    }

    Listener Engine::GetDefaultListener() const
    {
        return Listener(_defaultListener);
    }

    Listener Engine::AddListener(AmListenerID id) const
    {
        if (_state->listener_state_free_list.empty())
        {
            return Listener(nullptr);
        }

        if (const Listener item = GetListener(id); item.Valid())
        {
            return item;
        }
        else
        {
            ListenerInternalState* listener = _state->listener_state_free_list.back();
            listener->SetId(id);
            _state->listener_state_free_list.pop_back();
            _state->listener_list.push_back(*listener);
            return Listener(listener);
        }
    }

    Listener Engine::GetListener(AmListenerID id) const
    {
        if (const auto findIt = std::find_if(
                _state->listener_state_memory.begin(), _state->listener_state_memory.end(),
                [&id](const ListenerInternalState& state)
                {
                    return state.GetId() == id;
                });
            findIt != _state->listener_state_memory.end())
        {
            return Listener(&*findIt);
        }

        return Listener(nullptr);
    }

    void Engine::RemoveListener(AmListenerID id) const
    {
        if (const auto findIt = std::find_if(
                _state->listener_state_memory.begin(), _state->listener_state_memory.end(),
                [&id](const ListenerInternalState& state)
                {
                    return state.GetId() == id;
                });
            findIt == _state->listener_state_memory.end())
        {
            findIt->SetId(kAmInvalidObjectId);
            findIt->node.remove();
            _state->listener_state_free_list.push_back(&*findIt);
        }
    }

    void Engine::RemoveListener(const Listener* listener) const
    {
        if (listener->Valid())
        {
            listener->GetState()->SetId(kAmInvalidObjectId);
            listener->GetState()->node.remove();
            _state->listener_state_free_list.push_back(listener->GetState());
        }
    }

    Entity Engine::AddEntity(AmEntityID id) const
    {
        if (_state->entity_state_free_list.empty())
        {
            return Entity(nullptr);
        }

        if (const Entity item = GetEntity(id); item.Valid())
        {
            return item;
        }
        else
        {
            EntityInternalState* entity = _state->entity_state_free_list.back();
            entity->SetId(id);
            _state->entity_state_free_list.pop_back();
            _state->entity_list.push_back(*entity);
            return Entity(entity);
        }
    }

    Entity Engine::GetEntity(AmEntityID id) const
    {
        if (const auto findIt = std::find_if(
                _state->entity_state_memory.begin(), _state->entity_state_memory.end(),
                [&id](const EntityInternalState& state)
                {
                    return state.GetId() == id;
                });
            findIt != _state->entity_state_memory.end())
        {
            return Entity(&*findIt);
        }

        return Entity(nullptr);
    }

    void Engine::RemoveEntity(const Entity* entity) const
    {
        if (entity->Valid())
        {
            entity->GetState()->SetId(kAmInvalidObjectId);
            entity->GetState()->node.remove();
            _state->entity_state_free_list.push_back(entity->GetState());
        }
    }

    void Engine::RemoveEntity(AmEntityID id) const
    {
        if (const auto findIt = std::find_if(
                _state->entity_state_memory.begin(), _state->entity_state_memory.end(),
                [&id](const EntityInternalState& state)
                {
                    return state.GetId() == id;
                });
            findIt == _state->entity_state_memory.end())
        {
            findIt->SetId(kAmInvalidObjectId);
            findIt->node.remove();
            _state->entity_state_free_list.push_back(&*findIt);
        }
    }

    Environment Engine::AddEnvironment(AmEnvironmentID id) const
    {
        if (_state->environment_state_free_list.empty())
        {
            return Environment(nullptr);
        }

        if (const Environment item = GetEnvironment(id); item.Valid())
        {
            return item;
        }
        else
        {
            EnvironmentInternalState* environment = _state->environment_state_free_list.back();
            environment->SetId(id);
            _state->environment_state_free_list.pop_back();
            _state->environment_list.push_back(*environment);
            return Environment(environment);
        }
    }

    Environment Engine::GetEnvironment(AmEnvironmentID id) const
    {
        if (const auto findIt = std::find_if(
                _state->environment_state_memory.begin(), _state->environment_state_memory.end(),
                [&id](const EnvironmentInternalState& state)
                {
                    return state.GetId() == id;
                });
            findIt != _state->environment_state_memory.end())
        {
            return Environment(&*findIt);
        }

        return Environment(nullptr);
    }

    void Engine::RemoveEnvironment(const Environment* environment) const
    {
        if (environment->Valid())
        {
            environment->GetState()->SetId(kAmInvalidObjectId);
            environment->GetState()->node.remove();
            _state->environment_state_free_list.push_back(environment->GetState());
        }
    }

    void Engine::RemoveEnvironment(AmEnvironmentID id) const
    {
        if (const auto findIt = std::find_if(
                _state->environment_state_memory.begin(), _state->environment_state_memory.end(),
                [&id](const EnvironmentInternalState& state)
                {
                    return state.GetId() == id;
                });
            findIt == _state->environment_state_memory.end())
        {
            findIt->SetId(kAmInvalidObjectId);
            findIt->node.remove();
            _state->environment_state_free_list.push_back(&*findIt);
        }
    }

    Bus Engine::FindBus(const AmString& name) const
    {
        return Bus(FindBusInternalState(_state, name));
    }

    Bus Engine::FindBus(AmBusID id) const
    {
        return Bus(FindBusInternalState(_state, id));
    }

    void Engine::Pause(bool pause) const
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

    bool Engine::IsPaused() const
    {
        return _state->paused;
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
            AmReal32 pitch;
            CalculateGainPanPitch(
                &gain, &pan, &pitch, channel, switchContainer->GetGain().GetValue(), switchContainer->GetBus().GetState(),
                definition->spatialization(), switchContainer->GetAttenuation(), channel->GetEntity(), channel->GetLocation(),
                state->listener_list, channel->GetUserGain(), state->listener_fetch_mode);
            channel->SetGain(gain);
            channel->SetPan(pan);
            channel->SetPitch(pitch);
        }
        else if (const Collection* collection = channel->GetCollection(); collection != nullptr)
        {
            const CollectionDefinition* definition = collection->GetCollectionDefinition();

            float gain;
            hmm_vec2 pan;
            AmReal32 pitch;
            CalculateGainPanPitch(
                &gain, &pan, &pitch, channel, collection->GetGain().GetValue(), collection->GetBus().GetState(),
                definition->spatialization(), collection->GetAttenuation(), channel->GetEntity(), channel->GetLocation(),
                state->listener_list, channel->GetUserGain(), state->listener_fetch_mode);
            channel->SetGain(gain);
            channel->SetPan(pan);
            channel->SetPitch(pitch);
        }
        else if (const Sound* sound = channel->GetSound(); sound != nullptr)
        {
            const SoundDefinition* definition = sound->GetSoundDefinition();

            float gain;
            hmm_vec2 pan;
            AmReal32 pitch;
            CalculateGainPanPitch(
                &gain, &pan, &pitch, channel, sound->GetGain().GetValue(), sound->GetBus().GetState(), definition->spatialization(),
                sound->GetAttenuation(), channel->GetEntity(), channel->GetLocation(), state->listener_list, channel->GetUserGain(),
                state->listener_fetch_mode);
            channel->SetGain(gain);
            channel->SetPan(pan);
            channel->SetPitch(pitch);
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

    void Engine::AdvanceFrame(AmTime delta) const
    {
        if (_state->paused)
            return;

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

        for (auto&& state : _state->environment_list)
        {
            state.Update();
        }

        for (auto&& state : _state->entity_list)
        {
            state.Update();

            if (!_state->track_environments)
            {
                for (auto&& env : _state->environment_list)
                {
                    state.SetEnvironmentFactor(env.GetId(), env.GetFactor(Entity(&state)));
                }
            }
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
        UpdateRealChannels(&_state->playing_channel_list, &_state->real_channel_free_list, &_state->virtual_channel_free_list);

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

    const EngineConfigDefinition* Engine::GetEngineConfigDefinition() const
    {
        return Amplitude::GetEngineConfigDefinition(_configSrc.c_str());
    }

    Driver* Engine::GetDriver() const
    {
        return _audioDriver;
    }

#pragma region Amplimix

    Mixer* Engine::GetMixer() const
    {
        return &_state->mixer;
    }

#pragma endregion

#pragma region Engine State

    EngineInternalState* Engine::GetState() const
    {
        return _state;
    }

    AmReal32 Engine::GetSoundSpeed() const
    {
        return _state->sound_speed;
    }

    AmReal32 Engine::GetDopplerFactor() const
    {
        return _state->doppler_factor;
    }

    AmUInt32 Engine::GetSamplesPerStream() const
    {
        return _state->samples_per_stream;
    }

    AmUInt16 Engine::GetSampleRateConversionQuality() const
    {
        return static_cast<AmUInt16>(_state->sample_rate_conversion_quality);
    }

    bool Engine::IsGameTrackingEnvironmentAmounts() const
    {
        return _state->track_environments;
    }

    AmUInt32 Engine::GetMaxListenersCount() const
    {
        return GetEngineConfigDefinition()->game()->listeners();
    }

    AmUInt32 Engine::GetMaxEntitiesCount() const
    {
        return GetEngineConfigDefinition()->game()->entities();
    }

#pragma endregion

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
        AmReal32 pitch;
        CalculateGainPanPitch(
            &gain, &pan, &pitch, nullptr, handle->GetGain().GetValue(), handle->GetBus().GetState(), definition->spatialization(),
            handle->GetAttenuation(), entity, location, _state->listener_list, userGain, _state->listener_fetch_mode);
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
        newChannel->SetPitch(pitch);
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
        AmReal32 pitch;
        CalculateGainPanPitch(
            &gain, &pan, &pitch, nullptr, handle->GetGain().GetValue(), handle->GetBus().GetState(), definition->spatialization(),
            handle->GetAttenuation(), entity, location, _state->listener_list, userGain, _state->listener_fetch_mode);
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
        newChannel->SetPitch(pitch);
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
        AmReal32 pitch;
        CalculateGainPanPitch(
            &gain, &pan, &pitch, nullptr, handle->GetGain().GetValue(), handle->GetBus().GetState(), definition->spatialization(),
            handle->GetAttenuation(), entity, location, _state->listener_list, userGain, _state->listener_fetch_mode);
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
        newChannel->SetPitch(pitch);
        newChannel->SetLocation(location);

        return Channel(newChannel);
    }
} // namespace SparkyStudios::Audio::Amplitude
