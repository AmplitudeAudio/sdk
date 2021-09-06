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
#include "event_definition_generated.h"
#include "sound_bank_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    SoundBank::SoundBank()
        : _id(kAmInvalidObjectId)
        , _name()
        , _refCounter()
        , _soundBankDefSource()
    {}

    static bool InitializeCollection(AmOsString filename, Engine* audio_engine)
    {
        // Find the ID.
        CollectionHandle handle = audio_engine->GetCollectionHandleFromFile(filename);
        if (handle)
        {
            // We've seen this ID before, update it.
            handle->GetRefCounter()->Increment();
        }
        else
        {
            // This is a new collection, load it and update it.
            std::unique_ptr<Collection> collection(new Collection());
            if (!collection->LoadCollectionDefinitionFromFile(filename, audio_engine->GetState()))
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

            collection->GetRefCounter()->Increment();

            audio_engine->GetState()->collection_map[id] = std::move(collection);
            audio_engine->GetState()->collection_id_map[filename] = id;
        }

        return true;
    }

    static bool InitializeEvent(AmOsString filename, Engine* audio_engine)
    {
        // Find the ID.
        EventHandle handle = audio_engine->GetEventHandleFromFile(filename);
        if (handle)
        {
            // We've seen this ID before, update it.
            handle->GetRefCounter()->Increment();
        }
        else
        {
            // This is a new event, load it and update it.
            std::unique_ptr<Event> event(new Event());
            if (!event->LoadEventDefinitionFromFile(filename))
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

            audio_engine->GetState()->event_map[id] = std::move(event);
            audio_engine->GetState()->event_id_map[filename] = id;
        }

        return true;
    }

    static bool InitializeAttenuation(AmOsString filename, Engine* audio_engine)
    {
        // Find the ID.
        AttenuationHandle handle = audio_engine->GetAttenuationHandleFromFile(filename);
        if (handle)
        {
            // We've seen this ID before, update it.
            handle->GetRefCounter()->Increment();
        }
        else
        {
            // This is a new event, load it and update it.
            std::unique_ptr<Attenuation> attenuation(new Attenuation());
            if (!attenuation->LoadAttenuationDefinitionFromFile(filename))
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

            audio_engine->GetState()->attenuation_map[id] = std::move(attenuation);
            audio_engine->GetState()->attenuation_id_map[filename] = id;
        }

        return true;
    }

    bool SoundBank::Initialize(AmOsString filename, Engine* engine)
    {
        bool success = true;
        if (!LoadFile(filename, &_soundBankDefSource))
        {
            return false;
        }

        const SoundBankDefinition* definition = GetSoundBankDefinition();

        _id = definition->id();
        _name = definition->name()->str();

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

        // Load each Collection named in the sound bank.
        for (flatbuffers::uoffset_t i = 0; success && i < definition->collections()->size(); ++i)
        {
            AmString sound_filename = definition->collections()->Get(i)->c_str();
            success &= InitializeCollection(AM_STRING_TO_OS_STRING(sound_filename), engine);
        }

        return success;
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

        if (collection_iter->second->GetRefCounter()->Decrement() == 0)
        {
            state->collection_map.erase(collection_iter);
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

    void SoundBank::Deinitialize(Engine* engine)
    {
        const SoundBankDefinition* definition = GetSoundBankDefinition();

        for (flatbuffers::uoffset_t i = 0; i < definition->collections()->size(); ++i)
        {
            AmString filename = definition->collections()->Get(i)->c_str();
            if (!DeinitializeCollection(AM_STRING_TO_OS_STRING(filename), engine->GetState()))
            {
                CallLogFunc("Error while deinitializing collection %s in sound bank.\n", filename);
                AMPLITUDE_ASSERT(0);
            }
        }

        for (flatbuffers::uoffset_t i = 0; i < definition->events()->size(); ++i)
        {
            AmString filename = definition->events()->Get(i)->c_str();
            if (!DeinitializeEvent(AM_STRING_TO_OS_STRING(filename), engine->GetState()))
            {
                CallLogFunc("Error while deinitializing event %s in sound bank.\n", filename);
                AMPLITUDE_ASSERT(0);
            }
        }

        for (flatbuffers::uoffset_t i = 0; i < definition->attenuators()->size(); ++i)
        {
            AmString filename = definition->attenuators()->Get(i)->c_str();
            if (!DeinitializeAttenuation(AM_STRING_TO_OS_STRING(filename), engine->GetState()))
            {
                CallLogFunc("Error while deinitializing attenuation %s in sound bank.\n", filename);
                AMPLITUDE_ASSERT(0);
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
} // namespace SparkyStudios::Audio::Amplitude
