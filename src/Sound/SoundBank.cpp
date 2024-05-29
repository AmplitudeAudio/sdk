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

#include <SparkyStudios/Audio/Amplitude/Core/Log.h>
#include <SparkyStudios/Audio/Amplitude/Sound/SoundBank.h>

#include <Core/EngineInternalState.h>

#include "attenuation_definition_generated.h"
#include "collection_definition_generated.h"
#include "effect_definition_generated.h"
#include "event_definition_generated.h"
#include "rtpc_definition_generated.h"
#include "sound_bank_definition_generated.h"
#include "sound_definition_generated.h"
#include "switch_container_definition_generated.h"
#include "switch_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    SoundBank::SoundBank()
        : _refCounter()
        , _soundBankDefSource()
        , _name()
        , _id(kAmInvalidObjectId)
    {}

    SoundBank::SoundBank(const std::string& source)
        : SoundBank()
    {
        _soundBankDefSource = source;

        const SoundBankDefinition* definition = GetSoundBankDefinition();

        _id = definition->id();
        _name = definition->name()->str();
    }

    static bool InitializeSwitchContainer(const AmOsString& filename, const Engine* engine)
    {
        // Find the ID.
        if (SwitchContainerHandle handle = engine->GetSwitchContainerHandleFromFile(filename))
        {
            // We've seen this ID before, update it.
            handle->GetRefCounter()->Increment();
        }
        else
        {
            const FileSystem* fs = engine->GetFileSystem();
            const AmOsString& filePath = fs->ResolvePath(fs->Join({ AM_OS_STRING("switch_containers"), filename }));

            // This is a new switch container, load it and update it.
            AmUniquePtr<MemoryPoolKind::Engine, SwitchContainer> switch_container(ampoolnew(MemoryPoolKind::Engine, SwitchContainer));
            if (!switch_container->LoadDefinitionFromPath(filePath, engine->GetState()))
                return false;

            const SwitchContainerDefinition* definition = switch_container->GetDefinition();
            const AmSwitchContainerID id = definition->id();
            if (id == kAmInvalidObjectId)
            {
                CallLogFunc(
                    "[ERROR] Cannot load switch container \'" AM_OS_CHAR_FMT "\'. Invalid ID.\n",
                    AM_STRING_TO_OS_STRING(definition->name()->c_str()));
                return false;
            }

            switch_container->AcquireReferences(engine->GetState());
            switch_container->GetRefCounter()->Increment();

            engine->GetState()->switch_container_map[id] = std::move(switch_container);
            engine->GetState()->switch_container_id_map[filename] = id;
        }

        return true;
    }

    static bool InitializeCollection(const AmOsString& filename, const Engine* engine)
    {
        // Find the ID.
        if (CollectionHandle handle = engine->GetCollectionHandleFromFile(filename))
        {
            // We've seen this ID before, update it.
            handle->GetRefCounter()->Increment();
        }
        else
        {
            const FileSystem* fs = engine->GetFileSystem();
            const AmOsString& filePath = fs->ResolvePath(fs->Join({ AM_OS_STRING("collections"), filename }));

            // This is a new collection, load it and update it.
            AmUniquePtr<MemoryPoolKind::Engine, Collection> collection(ampoolnew(MemoryPoolKind::Engine, Collection));
            if (!collection->LoadDefinitionFromPath(filePath, engine->GetState()))
                return false;

            const CollectionDefinition* definition = collection->GetDefinition();
            const AmCollectionID id = definition->id();
            if (id == kAmInvalidObjectId)
            {
                CallLogFunc(
                    "[ERROR] Cannot load collection \'" AM_OS_CHAR_FMT "\'. Invalid ID.\n",
                    AM_STRING_TO_OS_STRING(definition->name()->c_str()));
                return false;
            }

            collection->AcquireReferences(engine->GetState());
            collection->GetRefCounter()->Increment();

            engine->GetState()->collection_map[id] = std::move(collection);
            engine->GetState()->collection_id_map[filename] = id;
        }

        return true;
    }

    static bool InitializeSound(const AmOsString& filename, const Engine* engine, AmSoundID& outId)
    {
        // Find the ID
        if (SoundHandle handle = engine->GetSoundHandleFromFile(filename))
        {
            // We've seen this id before, update it
            handle->GetRefCounter()->Increment();
        }
        else
        {
            const FileSystem* fs = engine->GetFileSystem();
            const AmOsString& filePath = fs->ResolvePath(fs->Join({ AM_OS_STRING("sounds"), filename }));

            // This is a new sound, load it and update it.
            AmUniquePtr<MemoryPoolKind::Engine, Sound> sound(ampoolnew(MemoryPoolKind::Engine, Sound));
            if (!sound->LoadDefinitionFromPath(filePath, engine->GetState()))
                return false;

            const SoundDefinition* definition = sound->GetDefinition();
            const AmSoundID id = definition->id();
            if (id == kAmInvalidObjectId)
            {
                CallLogFunc(
                    "[ERROR] Cannot load sound \'" AM_OS_CHAR_FMT "'. Invalid ID.\n", AM_STRING_TO_OS_STRING(definition->name()->c_str()));
                return false;
            }

            sound->AcquireReferences(engine->GetState());
            sound->GetRefCounter()->Increment();

            engine->GetState()->sound_map[id] = std::move(sound);
            engine->GetState()->sound_id_map[filename] = id;

            outId = id;
        }

        return true;
    }

    static bool InitializeEvent(const AmOsString& filename, const Engine* engine)
    {
        // Find the ID.
        if (EventHandle handle = engine->GetEventHandleFromFile(filename))
        {
            // We've seen this ID before, update it.
            handle->GetRefCounter()->Increment();
        }
        else
        {
            const FileSystem* fs = engine->GetFileSystem();
            const AmOsString& filePath = fs->ResolvePath(fs->Join({ AM_OS_STRING("events"), filename }));

            // This is a new event, load it and update it.
            AmUniquePtr<MemoryPoolKind::Engine, Event> event(ampoolnew(MemoryPoolKind::Engine, Event));
            if (!event->LoadDefinitionFromPath(filePath, engine->GetState()))
                return false;

            const EventDefinition* definition = event->GetDefinition();
            const AmEventID id = definition->id();
            if (id == kAmInvalidObjectId)
            {
                CallLogFunc(
                    "[ERROR] Cannot load event \'" AM_OS_CHAR_FMT "\'. Invalid ID.\n", AM_STRING_TO_OS_STRING(definition->name()->c_str()));
                return false;
            }

            event->AcquireReferences(engine->GetState());
            event->GetRefCounter()->Increment();

            engine->GetState()->event_map[id] = std::move(event);
            engine->GetState()->event_id_map[filename] = id;
        }

        return true;
    }

    static bool InitializeAttenuation(const AmOsString& filename, const Engine* engine)
    {
        // Find the ID.
        if (AttenuationHandle handle = engine->GetAttenuationHandleFromFile(filename))
        {
            // We've seen this ID before, update it.
            handle->GetRefCounter()->Increment();
        }
        else
        {
            const FileSystem* fs = engine->GetFileSystem();
            const AmOsString& filePath = fs->ResolvePath(fs->Join({ AM_OS_STRING("attenuators"), filename }));

            // This is a new event, load it and update it.
            AmUniquePtr<MemoryPoolKind::Engine, Attenuation> attenuation(ampoolnew(MemoryPoolKind::Engine, Attenuation));
            if (!attenuation->LoadDefinitionFromPath(filePath, engine->GetState()))
                return false;

            const AttenuationDefinition* definition = attenuation->GetDefinition();
            const AmAttenuationID id = definition->id();
            if (id == kAmInvalidObjectId)
            {
                CallLogFunc(
                    "[ERROR] Cannot load attenuation \'" AM_OS_CHAR_FMT "\'. Invalid ID.\n",
                    AM_STRING_TO_OS_STRING(definition->name()->c_str()));
                return false;
            }

            attenuation->AcquireReferences(engine->GetState());
            attenuation->GetRefCounter()->Increment();

            engine->GetState()->attenuation_map[id] = std::move(attenuation);
            engine->GetState()->attenuation_id_map[filename] = id;
        }

        return true;
    }

    static bool InitializeSwitch(const AmOsString& filename, const Engine* engine)
    {
        // Find the ID.
        SwitchHandle handle = engine->GetSwitchHandleFromFile(filename);
        if (handle)
        {
            // We've seen this ID before, update it.
            handle->GetRefCounter()->Increment();
        }
        else
        {
            const FileSystem* fs = engine->GetFileSystem();
            const AmOsString& filePath = fs->ResolvePath(fs->Join({ AM_OS_STRING("switches"), filename }));

            // This is a new event, load it and update it.
            AmUniquePtr<MemoryPoolKind::Engine, Switch> _switch(ampoolnew(MemoryPoolKind::Engine, Switch));
            if (!_switch->LoadDefinitionFromPath(filePath, engine->GetState()))
            {
                return false;
            }

            const SwitchDefinition* definition = _switch->GetDefinition();
            const AmSwitchID id = definition->id();
            if (id == kAmInvalidObjectId)
            {
                CallLogFunc(
                    "[ERROR] Cannot load switch \'" AM_OS_CHAR_FMT "\'. Invalid ID.\n",
                    AM_STRING_TO_OS_STRING(definition->name()->c_str()));
                return false;
            }

            _switch->AcquireReferences(engine->GetState());
            _switch->GetRefCounter()->Increment();

            engine->GetState()->switch_map[id] = std::move(_switch);
            engine->GetState()->switch_id_map[filename] = id;
        }

        return true;
    }

    static bool InitializeRtpc(const AmOsString& filename, const Engine* engine)
    {
        // Find the ID.
        RtpcHandle handle = engine->GetRtpcHandleFromFile(filename);
        if (handle)
        {
            // We've seen this ID before, update it.
            handle->GetRefCounter()->Increment();
        }
        else
        {
            const FileSystem* fs = engine->GetFileSystem();
            const AmOsString& filePath = fs->ResolvePath(fs->Join({ AM_OS_STRING("rtpc"), filename }));

            // This is a new event, load it and update it.
            AmUniquePtr<MemoryPoolKind::Engine, Rtpc> rtpc(ampoolnew(MemoryPoolKind::Engine, Rtpc));
            if (!rtpc->LoadDefinitionFromPath(filePath, engine->GetState()))
            {
                return false;
            }

            const RtpcDefinition* definition = rtpc->GetDefinition();
            const AmRtpcID id = definition->id();
            if (id == kAmInvalidObjectId)
            {
                CallLogFunc(
                    "[ERROR] Cannot load RTPC \'" AM_OS_CHAR_FMT "\'. Invalid ID.\n", AM_STRING_TO_OS_STRING(definition->name()->c_str()));
                return false;
            }

            rtpc->AcquireReferences(engine->GetState());
            rtpc->GetRefCounter()->Increment();

            engine->GetState()->rtpc_map[id] = std::move(rtpc);
            engine->GetState()->rtpc_id_map[filename] = id;
        }

        return true;
    }

    static bool InitializeEffect(const AmOsString& filename, const Engine* engine)
    {
        // Find the ID.
        EffectHandle handle = engine->GetEffectHandleFromFile(filename);
        if (handle)
        {
            // We've seen this ID before, update it.
            handle->GetRefCounter()->Increment();
        }
        else
        {
            const FileSystem* fs = engine->GetFileSystem();
            const AmOsString& filePath = fs->ResolvePath(fs->Join({ AM_OS_STRING("effects"), filename }));

            // This is a new event, load it and update it.
            AmUniquePtr<MemoryPoolKind::Engine, Effect> effect(ampoolnew(MemoryPoolKind::Engine, Effect));
            if (!effect->LoadDefinitionFromPath(filePath, engine->GetState()))
            {
                return false;
            }

            const EffectDefinition* definition = effect->GetDefinition();
            const AmEffectID id = definition->id();
            if (id == kAmInvalidObjectId)
            {
                CallLogFunc(
                    "[ERROR] Cannot load effect \'" AM_OS_CHAR_FMT "\'. Invalid ID.\n",
                    AM_STRING_TO_OS_STRING(definition->name()->c_str()));
                return false;
            }

            effect->AcquireReferences(engine->GetState());
            effect->GetRefCounter()->Increment();

            engine->GetState()->effect_map[id] = std::move(effect);
            engine->GetState()->effect_id_map[filename] = id;
        }

        return true;
    }

    bool SoundBank::Initialize(const AmOsString& filename, Engine* engine)
    {
        const FileSystem* fs = engine->GetFileSystem();
        const AmOsString& filePath = fs->ResolvePath(fs->Join({ AM_OS_STRING("soundbanks"), filename }));

        if (!LoadFile(fs->OpenFile(filePath), &_soundBankDefSource))
            return false;

        return InitializeInternal(engine);
    }

    bool SoundBank::InitializeFromMemory(const char* fileData, Engine* engine)
    {
        if (!fileData)
            return false;

        _soundBankDefSource = fileData;

        return InitializeInternal(engine);
    }

    static bool DeinitializeSwitchContainer(const AmOsString& filename, EngineInternalState* state)
    {
        const auto id_iter = state->switch_container_id_map.find(filename);
        if (id_iter == state->switch_container_id_map.end())
            return false;

        const AmSwitchContainerID id = id_iter->second;

        const auto switch_container_iter = state->switch_container_map.find(id);
        if (switch_container_iter == state->switch_container_map.end())
            return false;

        switch_container_iter->second->ReleaseReferences(state);

        if (switch_container_iter->second->GetRefCounter()->Decrement() == 0)
            state->switch_container_map.erase(switch_container_iter);

        return true;
    }

    static bool DeinitializeCollection(const AmOsString& filename, EngineInternalState* state)
    {
        const auto id_iter = state->collection_id_map.find(filename);
        if (id_iter == state->collection_id_map.end())
            return false;

        const AmCollectionID id = id_iter->second;

        const auto collection_iter = state->collection_map.find(id);
        if (collection_iter == state->collection_map.end())
            return false;

        collection_iter->second->ReleaseReferences(state);

        if (collection_iter->second->GetRefCounter()->Decrement() == 0)
            state->collection_map.erase(collection_iter);

        return true;
    }

    static bool DeinitializeSound(const AmOsString& filename, EngineInternalState* state)
    {
        const auto id_iter = state->sound_id_map.find(filename);
        if (id_iter == state->sound_id_map.end())
            return false;

        const AmSoundID id = id_iter->second;

        const auto sound_iter = state->sound_map.find(id);
        if (sound_iter == state->sound_map.end())
            return false;

        sound_iter->second->ReleaseReferences(state);

        if (sound_iter->second->GetRefCounter()->Decrement() == 0)
            state->sound_map.erase(sound_iter);

        return true;
    }

    static bool DeinitializeEvent(const AmOsString& filename, EngineInternalState* state)
    {
        const auto id_iter = state->event_id_map.find(filename);
        if (id_iter == state->event_id_map.end())
            return false;

        const AmEventID id = id_iter->second;

        const auto event_iter = state->event_map.find(id);
        if (event_iter == state->event_map.end())
            return false;

        event_iter->second->ReleaseReferences(state);

        if (event_iter->second->GetRefCounter()->Decrement() == 0)
            state->event_map.erase(event_iter);

        return true;
    }

    static bool DeinitializeAttenuation(const AmOsString& filename, EngineInternalState* state)
    {
        const auto id_iter = state->attenuation_id_map.find(filename);
        if (id_iter == state->attenuation_id_map.end())
            return false;

        const AmAttenuationID id = id_iter->second;

        const auto attenuation_iter = state->attenuation_map.find(id);
        if (attenuation_iter == state->attenuation_map.end())
            return false;

        attenuation_iter->second->ReleaseReferences(state);

        if (attenuation_iter->second->GetRefCounter()->Decrement() == 0)
            state->attenuation_map.erase(attenuation_iter);

        return true;
    }

    static bool DeinitializeSwitch(const AmOsString& filename, EngineInternalState* state)
    {
        const auto id_iter = state->switch_id_map.find(filename);
        if (id_iter == state->switch_id_map.end())
            return false;

        const AmSwitchID id = id_iter->second;

        const auto switch_iter = state->switch_map.find(id);
        if (switch_iter == state->switch_map.end())
            return false;

        switch_iter->second->ReleaseReferences(state);

        if (switch_iter->second->GetRefCounter()->Decrement() == 0)
            state->switch_map.erase(switch_iter);

        return true;
    }

    static bool DeinitializeEffect(const AmOsString& filename, EngineInternalState* state)
    {
        const auto id_iter = state->effect_id_map.find(filename);
        if (id_iter == state->effect_id_map.end())
            return false;

        const AmSwitchID id = id_iter->second;

        const auto effect_iter = state->effect_map.find(id);
        if (effect_iter == state->effect_map.end())
            return false;

        effect_iter->second->ReleaseReferences(state);

        if (effect_iter->second->GetRefCounter()->Decrement() == 0)
            state->effect_map.erase(effect_iter);

        return true;
    }

    static bool DeinitializeRtpc(const AmOsString& filename, EngineInternalState* state)
    {
        const auto id_iter = state->rtpc_id_map.find(filename);
        if (id_iter == state->rtpc_id_map.end())
            return false;

        const AmSwitchID id = id_iter->second;

        const auto rtpc_iter = state->rtpc_map.find(id);
        if (rtpc_iter == state->rtpc_map.end())
            return false;

        rtpc_iter->second->ReleaseReferences(state);

        if (rtpc_iter->second->GetRefCounter()->Decrement() == 0)
            state->rtpc_map.erase(rtpc_iter);

        return true;
    }

    void SoundBank::Deinitialize(Engine* engine)
    {
        const SoundBankDefinition* definition = GetSoundBankDefinition();

        for (flatbuffers::uoffset_t i = 0; i < definition->switch_containers()->size(); ++i)
        {
            AmString filename = definition->switch_containers()->Get(i)->c_str();
            if (!DeinitializeSwitchContainer(AM_STRING_TO_OS_STRING(filename), engine->GetState()))
            {
                CallLogFunc("Error while deinitializing switch container " AM_OS_CHAR_FMT " in sound bank.\n", filename.c_str());
                AMPLITUDE_ASSERT(false);
            }
        }

        for (flatbuffers::uoffset_t i = 0; i < definition->collections()->size(); ++i)
        {
            AmString filename = definition->collections()->Get(i)->c_str();
            if (!DeinitializeCollection(AM_STRING_TO_OS_STRING(filename), engine->GetState()))
            {
                CallLogFunc("Error while deinitializing collection " AM_OS_CHAR_FMT " in sound bank.\n", filename.c_str());
                AMPLITUDE_ASSERT(false);
            }
        }

        for (flatbuffers::uoffset_t i = 0; i < definition->sounds()->size(); ++i)
        {
            AmString filename = definition->sounds()->Get(i)->c_str();
            if (!DeinitializeSound(AM_STRING_TO_OS_STRING(filename), engine->GetState()))
            {
                CallLogFunc("Error while deinitializing sound " AM_OS_CHAR_FMT " in sound bank.\n", filename.c_str());
                AMPLITUDE_ASSERT(false);
            }
        }

        for (flatbuffers::uoffset_t i = 0; i < definition->events()->size(); ++i)
        {
            AmString filename = definition->events()->Get(i)->c_str();
            if (!DeinitializeEvent(AM_STRING_TO_OS_STRING(filename), engine->GetState()))
            {
                CallLogFunc("Error while deinitializing event " AM_OS_CHAR_FMT " in sound bank.\n", filename.c_str());
                AMPLITUDE_ASSERT(false);
            }
        }

        for (flatbuffers::uoffset_t i = 0; i < definition->attenuators()->size(); ++i)
        {
            AmString filename = definition->attenuators()->Get(i)->c_str();
            if (!DeinitializeAttenuation(AM_STRING_TO_OS_STRING(filename), engine->GetState()))
            {
                CallLogFunc("Error while deinitializing attenuation " AM_OS_CHAR_FMT " in sound bank.\n", filename.c_str());
                AMPLITUDE_ASSERT(false);
            }
        }

        for (flatbuffers::uoffset_t i = 0; i < definition->switches()->size(); ++i)
        {
            AmString filename = definition->switches()->Get(i)->c_str();
            if (!DeinitializeSwitch(AM_STRING_TO_OS_STRING(filename), engine->GetState()))
            {
                CallLogFunc("Error while deinitializing switch " AM_OS_CHAR_FMT " in sound bank.\n", filename.c_str());
                AMPLITUDE_ASSERT(false);
            }
        }

        for (flatbuffers::uoffset_t i = 0; i < definition->effects()->size(); ++i)
        {
            AmString filename = definition->effects()->Get(i)->c_str();
            if (!DeinitializeEffect(AM_STRING_TO_OS_STRING(filename), engine->GetState()))
            {
                CallLogFunc("Error while deinitializing effect " AM_OS_CHAR_FMT " in sound bank.\n", filename.c_str());
                AMPLITUDE_ASSERT(false);
            }
        }

        for (flatbuffers::uoffset_t i = 0; i < definition->rtpc()->size(); ++i)
        {
            AmString filename = definition->rtpc()->Get(i)->c_str();
            if (!DeinitializeRtpc(AM_STRING_TO_OS_STRING(filename), engine->GetState()))
            {
                CallLogFunc("Error while deinitializing RTPC " AM_OS_CHAR_FMT " in sound bank.\n", filename.c_str());
                AMPLITUDE_ASSERT(false);
            }
        }
    }

    AmBankID SoundBank::GetId() const
    {
        return _id;
    }

    const std::string& SoundBank::GetName() const
    {
        return _name;
    }

    const SoundBankDefinition* SoundBank::GetSoundBankDefinition() const
    {
        return Amplitude::GetSoundBankDefinition(_soundBankDefSource.c_str());
    }

    RefCounter* SoundBank::GetRefCounter()
    {
        return &_refCounter;
    }

    void SoundBank::LoadSoundFiles(const Engine* engine)
    {
        while (!_pendingSoundsToLoad.empty())
        {
            const auto id = _pendingSoundsToLoad.front();
            _pendingSoundsToLoad.pop();

            if (!engine->GetState()->sound_map.contains(id))
                continue;

            engine->GetState()->sound_map[id]->Load(engine->GetFileSystem());
        }
    }

    bool SoundBank::InitializeInternal(Engine* engine)
    {
        bool success = true;
        const SoundBankDefinition* definition = GetSoundBankDefinition();

        _id = definition->id();
        _name = definition->name()->str();

        // Load each Rtpc named in the sound bank.
        for (flatbuffers::uoffset_t i = 0; success && i < definition->rtpc()->size(); ++i)
        {
            AmString filename = definition->rtpc()->Get(i)->c_str();
            success &= InitializeRtpc(AM_STRING_TO_OS_STRING(filename), engine);
        }

        // Load each effect named in the sound bank.
        for (flatbuffers::uoffset_t i = 0; success && i < definition->effects()->size(); ++i)
        {
            AmString filename = definition->effects()->Get(i)->c_str();
            success &= InitializeEffect(AM_STRING_TO_OS_STRING(filename), engine);
        }

        // Load each Switch named in the sound bank.
        for (flatbuffers::uoffset_t i = 0; success && i < definition->switches()->size(); ++i)
        {
            AmString switch_filename = definition->switches()->Get(i)->c_str();
            success &= InitializeSwitch(AM_STRING_TO_OS_STRING(switch_filename), engine);
        }

        // Load each Attenuation named in the sound bank.
        for (flatbuffers::uoffset_t i = 0; success && i < definition->attenuators()->size(); ++i)
        {
            AmString attenuation_filename = definition->attenuators()->Get(i)->c_str();
            success &= InitializeAttenuation(AM_STRING_TO_OS_STRING(attenuation_filename), engine);
        }

        // Load each Event named in the sound bank.
        for (flatbuffers::uoffset_t i = 0; success && i < definition->events()->size(); ++i)
        {
            AmString event_filename = definition->events()->Get(i)->c_str();
            success &= InitializeEvent(AM_STRING_TO_OS_STRING(event_filename), engine);
        }

        // Load each Sound named in the sound bank.
        for (flatbuffers::uoffset_t i = 0; success && i < definition->sounds()->size(); ++i)
        {
            AmSoundID id = kAmInvalidObjectId;
            AmString filename = definition->sounds()->Get(i)->c_str();
            success &= InitializeSound(AM_STRING_TO_OS_STRING(filename), engine, id);
            _pendingSoundsToLoad.push(id);
        }

        // Load each Collection named in the sound bank.
        for (flatbuffers::uoffset_t i = 0; success && i < definition->collections()->size(); ++i)
        {
            AmString sound_filename = definition->collections()->Get(i)->c_str();
            success &= InitializeCollection(AM_STRING_TO_OS_STRING(sound_filename), engine);
        }

        // Load each SwitchContainer named in the sound bank.
        for (flatbuffers::uoffset_t i = 0; success && i < definition->switch_containers()->size(); ++i)
        {
            AmString filename = definition->switch_containers()->Get(i)->c_str();
            success &= InitializeSwitchContainer(AM_STRING_TO_OS_STRING(filename), engine);
        }

        return success;
    }
} // namespace SparkyStudios::Audio::Amplitude
