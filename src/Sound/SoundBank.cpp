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
        : _id(kAmInvalidObjectId)
        , _name()
        , _refCounter()
        , _soundBankDefSource()
    {}

    SoundBank::SoundBank(const std::string& source)
        : SoundBank()
    {
        _soundBankDefSource = source;

        const SoundBankDefinition* definition = GetSoundBankDefinition();

        _id = definition->id();
        _name = definition->name()->str();
    }

    static bool InitializeSwitchContainer(AmOsString filename, Engine* engine)
    {
        // Find the ID.
        SwitchContainerHandle handle = engine->GetSwitchContainerHandleFromFile(filename);
        if (handle)
        {
            // We've seen this ID before, update it.
            handle->GetRefCounter()->Increment();
        }
        else
        {
            std::filesystem::path filePath = engine->GetFileLoader()->ResolvePath(std::filesystem::path("switch_containers") / filename);

            // This is a new switch container, load it and update it.
            std::unique_ptr<SwitchContainer> switch_container(new SwitchContainer());
            if (!switch_container->LoadSwitchContainerDefinitionFromFile(filePath.c_str(), engine->GetState()))
            {
                return false;
            }

            const SwitchContainerDefinition* definition = switch_container->GetSwitchContainerDefinition();
            AmSwitchContainerID id = definition->id();
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

    static bool InitializeCollection(AmOsString filename, Engine* engine)
    {
        // Find the ID.
        CollectionHandle handle = engine->GetCollectionHandleFromFile(filename);
        if (handle)
        {
            // We've seen this ID before, update it.
            handle->GetRefCounter()->Increment();
        }
        else
        {
            std::filesystem::path filePath = engine->GetFileLoader()->ResolvePath(std::filesystem::path("collections") / filename);

            // This is a new collection, load it and update it.
            std::unique_ptr<Collection> collection(new Collection());
            if (!collection->LoadCollectionDefinitionFromFile(filePath.c_str(), engine->GetState()))
            {
                return false;
            }

            const CollectionDefinition* definition = collection->GetCollectionDefinition();
            AmCollectionID id = definition->id();
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

    static bool InitializeSound(AmOsString filename, Engine* engine)
    {
        // Find the ID
        SoundHandle handle = engine->GetSoundHandleFromFile(filename);
        if (handle)
        {
            // We've seen this id before, update it
            handle->GetRefCounter()->Increment();
        }
        else
        {
            std::filesystem::path filePath = engine->GetFileLoader()->ResolvePath(std::filesystem::path("sounds") / filename);

            // This is a new sound, load it and update it.
            std::unique_ptr<Sound> sound(new Sound());
            if (!sound->LoadSoundDefinitionFromFile(filePath.c_str(), engine->GetState()))
            {
                return false;
            }

            const SoundDefinition* definition = sound->GetSoundDefinition();
            AmSoundID id = definition->id();
            if (id == kAmInvalidObjectId)
            {
                CallLogFunc(
                    "[ERROR] Cannot load sound \'" AM_OS_CHAR_FMT "'. Invalid ID.\n", AM_STRING_TO_OS_STRING(definition->name()->c_str()));
                return false;
            }

            sound->LoadFile(
                engine->GetFileLoader()->ResolvePath(std::filesystem::path("sounds") / std::filesystem::path(definition->path()->c_str())),
                engine->GetFileLoader());

            sound->AcquireReferences(engine->GetState());
            sound->GetRefCounter()->Increment();

            engine->GetState()->sound_map[id] = std::move(sound);
            engine->GetState()->sound_id_map[filename] = id;
        }

        return true;
    }

    static bool InitializeEvent(AmOsString filename, Engine* engine)
    {
        // Find the ID.
        EventHandle handle = engine->GetEventHandleFromFile(filename);
        if (handle)
        {
            // We've seen this ID before, update it.
            handle->GetRefCounter()->Increment();
        }
        else
        {
            std::filesystem::path filePath = engine->GetFileLoader()->ResolvePath(std::filesystem::path("events") / filename);

            // This is a new event, load it and update it.
            std::unique_ptr<Event> event(new Event());
            if (!event->LoadEventDefinitionFromFile(filePath.c_str()))
            {
                return false;
            }

            const EventDefinition* definition = event->GetEventDefinition();
            AmEventID id = definition->id();
            if (id == kAmInvalidObjectId)
            {
                CallLogFunc(
                    "[ERROR] Cannot load event \'" AM_OS_CHAR_FMT "\'. Invalid ID.\n", AM_STRING_TO_OS_STRING(definition->name()->c_str()));
                return false;
            }

            event->GetRefCounter()->Increment();

            engine->GetState()->event_map[id] = std::move(event);
            engine->GetState()->event_id_map[filename] = id;
        }

        return true;
    }

    static bool InitializeAttenuation(AmOsString filename, Engine* engine)
    {
        // Find the ID.
        AttenuationHandle handle = engine->GetAttenuationHandleFromFile(filename);
        if (handle)
        {
            // We've seen this ID before, update it.
            handle->GetRefCounter()->Increment();
        }
        else
        {
            std::filesystem::path filePath = engine->GetFileLoader()->ResolvePath(std::filesystem::path("attenuators") / filename);

            // This is a new event, load it and update it.
            std::unique_ptr<Attenuation> attenuation(new Attenuation());
            if (!attenuation->LoadAttenuationDefinitionFromFile(filePath.c_str()))
            {
                return false;
            }

            const AttenuationDefinition* definition = attenuation->GetAttenuationDefinition();
            AmAttenuationID id = definition->id();
            if (id == kAmInvalidObjectId)
            {
                CallLogFunc(
                    "[ERROR] Cannot load attenuation \'" AM_OS_CHAR_FMT "\'. Invalid ID.\n",
                    AM_STRING_TO_OS_STRING(definition->name()->c_str()));
                return false;
            }

            attenuation->GetRefCounter()->Increment();

            engine->GetState()->attenuation_map[id] = std::move(attenuation);
            engine->GetState()->attenuation_id_map[filename] = id;
        }

        return true;
    }

    static bool InitializeSwitch(AmOsString filename, Engine* engine)
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
            std::filesystem::path filePath = engine->GetFileLoader()->ResolvePath(std::filesystem::path("switches") / filename);

            // This is a new event, load it and update it.
            std::unique_ptr<Switch> _switch(new Switch());
            if (!_switch->LoadSwitchDefinitionFromFile(filePath.c_str()))
            {
                return false;
            }

            const SwitchDefinition* definition = _switch->GetSwitchDefinition();
            AmSwitchID id = definition->id();
            if (id == kAmInvalidObjectId)
            {
                CallLogFunc(
                    "[ERROR] Cannot load switch \'" AM_OS_CHAR_FMT "\'. Invalid ID.\n",
                    AM_STRING_TO_OS_STRING(definition->name()->c_str()));
                return false;
            }

            _switch->GetRefCounter()->Increment();

            engine->GetState()->switch_map[id] = std::move(_switch);
            engine->GetState()->switch_id_map[filename] = id;
        }

        return true;
    }

    static bool InitializeRtpc(AmOsString filename, Engine* engine)
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
            std::filesystem::path filePath = engine->GetFileLoader()->ResolvePath(std::filesystem::path("rtpc") / filename);

            // This is a new event, load it and update it.
            std::unique_ptr<Rtpc> rtpc(new Rtpc());
            if (!rtpc->LoadRtpcDefinitionFromFile(filePath.c_str()))
            {
                return false;
            }

            const RtpcDefinition* definition = rtpc->GetRtpcDefinition();
            AmRtpcID id = definition->id();
            if (id == kAmInvalidObjectId)
            {
                CallLogFunc(
                    "[ERROR] Cannot load RTPC \'" AM_OS_CHAR_FMT "\'. Invalid ID.\n", AM_STRING_TO_OS_STRING(definition->name()->c_str()));
                return false;
            }

            rtpc->GetRefCounter()->Increment();

            engine->GetState()->rtpc_map[id] = std::move(rtpc);
            engine->GetState()->rtpc_id_map[filename] = id;
        }

        return true;
    }

    static bool InitializeEffect(AmOsString filename, Engine* engine)
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
            std::filesystem::path filePath = engine->GetFileLoader()->ResolvePath(std::filesystem::path("effects") / filename);

            // This is a new event, load it and update it.
            std::unique_ptr<Effect> effect(new Effect());
            if (!effect->LoadEffectDefinitionFromFile(filePath.c_str()))
            {
                return false;
            }

            const EffectDefinition* definition = effect->GetEffectDefinition();
            AmEffectID id = definition->id();
            if (id == kAmInvalidObjectId)
            {
                CallLogFunc(
                    "[ERROR] Cannot load effect \'" AM_OS_CHAR_FMT "\'. Invalid ID.\n",
                    AM_STRING_TO_OS_STRING(definition->name()->c_str()));
                return false;
            }

            effect->GetRefCounter()->Increment();

            engine->GetState()->effect_map[id] = std::move(effect);
            engine->GetState()->effect_id_map[filename] = id;
        }

        return true;
    }

    bool SoundBank::Initialize(AmOsString filename, Engine* engine)
    {
        std::filesystem::path filePath = engine->GetFileLoader()->ResolvePath(std::filesystem::path("soundbanks") / filename);

        if (!LoadFile(filePath.c_str(), &_soundBankDefSource))
        {
            return false;
        }

        return InitializeInternal(engine);
    }

    bool SoundBank::InitializeFromMemory(const char* fileData, Engine* engine)
    {
        if (!fileData)
        {
            return false;
        }

        _soundBankDefSource = fileData;

        return InitializeInternal(engine);
    }

    static bool DeinitializeSwitchContainer(AmOsString filename, EngineInternalState* state)
    {
        auto id_iter = state->switch_container_id_map.find(filename);
        if (id_iter == state->switch_container_id_map.end())
        {
            return false;
        }

        AmSwitchContainerID id = id_iter->second;
        auto switch_container_iter = state->switch_container_map.find(id);
        if (switch_container_iter == state->switch_container_map.end())
        {
            return false;
        }

        switch_container_iter->second->ReleaseReferences(state);

        if (switch_container_iter->second->GetRefCounter()->Decrement() == 0)
        {
            state->switch_container_map.erase(switch_container_iter);
        }

        return true;
    }

    static bool DeinitializeCollection(AmOsString filename, EngineInternalState* state)
    {
        auto id_iter = state->collection_id_map.find(filename);
        if (id_iter == state->collection_id_map.end())
        {
            return false;
        }

        AmCollectionID id = id_iter->second;
        auto collection_iter = state->collection_map.find(id);
        if (collection_iter == state->collection_map.end())
        {
            return false;
        }

        collection_iter->second->ReleaseReferences(state);

        if (collection_iter->second->GetRefCounter()->Decrement() == 0)
        {
            state->collection_map.erase(collection_iter);
        }

        return true;
    }

    static bool DeinitializeSound(AmOsString filename, EngineInternalState* state)
    {
        auto id_iter = state->sound_id_map.find(filename);
        if (id_iter == state->sound_id_map.end())
        {
            return false;
        }

        AmSoundID id = id_iter->second;
        auto sound_iter = state->sound_map.find(id);
        if (sound_iter == state->sound_map.end())
        {
            return false;
        }

        sound_iter->second->ReleaseReferences(state);

        if (sound_iter->second->GetRefCounter()->Decrement() == 0)
        {
            state->sound_map.erase(sound_iter);
        }

        return true;
    }

    static bool DeinitializeEvent(AmOsString filename, EngineInternalState* state)
    {
        auto id_iter = state->event_id_map.find(filename);
        if (id_iter == state->event_id_map.end())
        {
            return false;
        }

        AmEventID id = id_iter->second;
        auto event_iter = state->event_map.find(id);
        if (event_iter == state->event_map.end())
        {
            return false;
        }

        if (event_iter->second->GetRefCounter()->Decrement() == 0)
        {
            state->event_map.erase(event_iter);
        }

        return true;
    }

    static bool DeinitializeAttenuation(AmOsString filename, EngineInternalState* state)
    {
        auto id_iter = state->attenuation_id_map.find(filename);
        if (id_iter == state->attenuation_id_map.end())
        {
            return false;
        }

        AmAttenuationID id = id_iter->second;
        auto attenuation_iter = state->attenuation_map.find(id);
        if (attenuation_iter == state->attenuation_map.end())
        {
            return false;
        }

        if (attenuation_iter->second->GetRefCounter()->Decrement() == 0)
        {
            state->attenuation_map.erase(attenuation_iter);
        }

        return true;
    }

    static bool DeinitializeSwitch(AmOsString filename, EngineInternalState* state)
    {
        auto id_iter = state->switch_id_map.find(filename);
        if (id_iter == state->switch_id_map.end())
        {
            return false;
        }

        AmSwitchID id = id_iter->second;
        auto switch_iter = state->switch_map.find(id);
        if (switch_iter == state->switch_map.end())
        {
            return false;
        }

        if (switch_iter->second->GetRefCounter()->Decrement() == 0)
        {
            state->switch_map.erase(switch_iter);
        }

        return true;
    }

    static bool DeinitializeEffect(AmOsString filename, EngineInternalState* state)
    {
        auto id_iter = state->effect_id_map.find(filename);
        if (id_iter == state->effect_id_map.end())
        {
            return false;
        }

        AmSwitchID id = id_iter->second;
        auto effect_iter = state->effect_map.find(id);
        if (effect_iter == state->effect_map.end())
        {
            return false;
        }

        if (effect_iter->second->GetRefCounter()->Decrement() == 0)
        {
            state->effect_map.erase(effect_iter);
        }

        return true;
    }

    static bool DeinitializeRtpc(AmOsString filename, EngineInternalState* state)
    {
        auto id_iter = state->rtpc_id_map.find(filename);
        if (id_iter == state->rtpc_id_map.end())
        {
            return false;
        }

        AmSwitchID id = id_iter->second;
        auto rtpc_iter = state->rtpc_map.find(id);
        if (rtpc_iter == state->rtpc_map.end())
        {
            return false;
        }

        if (rtpc_iter->second->GetRefCounter()->Decrement() == 0)
        {
            state->rtpc_map.erase(rtpc_iter);
        }

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
                CallLogFunc("Error while deinitializing switch container %s in sound bank.\n", filename);
                AMPLITUDE_ASSERT(false);
            }
        }

        for (flatbuffers::uoffset_t i = 0; i < definition->collections()->size(); ++i)
        {
            AmString filename = definition->collections()->Get(i)->c_str();
            if (!DeinitializeCollection(AM_STRING_TO_OS_STRING(filename), engine->GetState()))
            {
                CallLogFunc("Error while deinitializing collection %s in sound bank.\n", filename);
                AMPLITUDE_ASSERT(false);
            }
        }

        for (flatbuffers::uoffset_t i = 0; i < definition->sounds()->size(); ++i)
        {
            AmString filename = definition->sounds()->Get(i)->c_str();
            if (!DeinitializeSound(AM_STRING_TO_OS_STRING(filename), engine->GetState()))
            {
                CallLogFunc("Error while deinitializing sound %s in sound bank.\n", filename);
                AMPLITUDE_ASSERT(false);
            }
        }

        for (flatbuffers::uoffset_t i = 0; i < definition->events()->size(); ++i)
        {
            AmString filename = definition->events()->Get(i)->c_str();
            if (!DeinitializeEvent(AM_STRING_TO_OS_STRING(filename), engine->GetState()))
            {
                CallLogFunc("Error while deinitializing event %s in sound bank.\n", filename);
                AMPLITUDE_ASSERT(false);
            }
        }

        for (flatbuffers::uoffset_t i = 0; i < definition->attenuators()->size(); ++i)
        {
            AmString filename = definition->attenuators()->Get(i)->c_str();
            if (!DeinitializeAttenuation(AM_STRING_TO_OS_STRING(filename), engine->GetState()))
            {
                CallLogFunc("Error while deinitializing attenuation %s in sound bank.\n", filename);
                AMPLITUDE_ASSERT(false);
            }
        }

        for (flatbuffers::uoffset_t i = 0; i < definition->switches()->size(); ++i)
        {
            AmString filename = definition->switches()->Get(i)->c_str();
            if (!DeinitializeSwitch(AM_STRING_TO_OS_STRING(filename), engine->GetState()))
            {
                CallLogFunc("Error while deinitializing switch %s in sound bank.\n", filename);
                AMPLITUDE_ASSERT(false);
            }
        }

        for (flatbuffers::uoffset_t i = 0; i < definition->effects()->size(); ++i)
        {
            AmString filename = definition->effects()->Get(i)->c_str();
            if (!DeinitializeEffect(AM_STRING_TO_OS_STRING(filename), engine->GetState()))
            {
                CallLogFunc("Error while deinitializing effect %s in sound bank.\n", filename);
                AMPLITUDE_ASSERT(false);
            }
        }

        for (flatbuffers::uoffset_t i = 0; i < definition->rtpc()->size(); ++i)
        {
            AmString filename = definition->rtpc()->Get(i)->c_str();
            if (!DeinitializeRtpc(AM_STRING_TO_OS_STRING(filename), engine->GetState()))
            {
                CallLogFunc("Error while deinitializing RTPC %s in sound bank.\n", filename);
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
            AmString filename = definition->sounds()->Get(i)->c_str();
            success &= InitializeSound(AM_STRING_TO_OS_STRING(filename), engine);
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
