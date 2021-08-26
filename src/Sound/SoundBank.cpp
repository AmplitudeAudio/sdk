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

    static bool InitializeSoundCollection(AmOsString filename, Engine* audio_engine)
    {
        // Find the ID.
        SoundHandle handle = audio_engine->GetSoundHandleFromFile(filename);
        if (handle)
        {
            // We've seen this ID before, update it.
            handle->GetRefCounter()->Increment();
        }
        else
        {
            // This is a new sound collection, load it and update it.
            std::unique_ptr<SoundCollection> collection(new SoundCollection());
            if (!collection->LoadSoundCollectionDefinitionFromFile(filename, audio_engine->GetState()))
            {
                return false;
            }

            const SoundCollectionDefinition* definition = collection->GetSoundCollectionDefinition();
            AmSoundCollectionID id = definition->id();
            if (id == kAmInvalidObjectId)
            {
                CallLogFunc(
                    "[ERROR] Cannot load sound collection \'" AM_OS_CHAR_FMT "\'. Invalid ID.\n",
                    AM_STRING_TO_OS_STRING(definition->name()->c_str()));
                return false;
            }

            collection->GetRefCounter()->Increment();
            audio_engine->GetState()->sound_collection_map[id] = std::move(collection);
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
            event->GetRefCounter()->Increment();

            std::string name = event->GetEventDefinition()->name()->c_str();
            audio_engine->GetState()->event_map[name] = std::move(event);
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

        // Load each SoundCollection named in the sound bank.
        for (flatbuffers::uoffset_t i = 0; i < definition->sounds()->size(); ++i)
        {
            AmString sound_filename = definition->sounds()->Get(i)->c_str();
            success &= InitializeSoundCollection(AM_STRING_TO_OS_STRING(sound_filename), engine);
        }

        // Load each Event named in the sound bank.
        for (flatbuffers::uoffset_t i = 0; i < definition->events()->size(); ++i)
        {
            AmString event_filename = definition->events()->Get(i)->c_str();
            success &= InitializeEvent(AM_STRING_TO_OS_STRING(event_filename), engine);
        }

        return success;
    }

    static bool DeinitializeSoundCollection(AmOsString filename, EngineInternalState* state)
    {
        auto id_iter = state->sound_id_map.find(filename);
        if (id_iter == state->sound_id_map.end())
        {
            return false;
        }

        AmSoundCollectionID id = id_iter->second;
        auto collection_iter = state->sound_collection_map.find(id);
        if (collection_iter == state->sound_collection_map.end())
        {
            return false;
        }

        if (collection_iter->second->GetRefCounter()->Decrement() == 0)
        {
            state->sound_collection_map.erase(collection_iter);
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

        std::string id = id_iter->second;
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

    void SoundBank::Deinitialize(Engine* engine)
    {
        const SoundBankDefinition* definition = GetSoundBankDefinition();

        for (flatbuffers::uoffset_t i = 0; i < definition->sounds()->size(); ++i)
        {
            AmString filename = definition->sounds()->Get(i)->c_str();
            if (!DeinitializeSoundCollection(AM_STRING_TO_OS_STRING(filename), engine->GetState()))
            {
                CallLogFunc("Error while deinitializing sound collection %s in sound bank.\n", filename);
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
