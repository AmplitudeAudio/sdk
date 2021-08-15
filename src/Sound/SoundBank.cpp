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

#include "sound_bank_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    static bool InitializeSoundCollection(const std::string& filename, Engine* audio_engine)
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
            collection->GetRefCounter()->Increment();

            std::string name = collection->GetSoundCollectionDefinition()->name()->c_str();
            audio_engine->GetState()->sound_collection_map[name] = std::move(collection);
        }
        return true;
    }

    bool SoundBank::Initialize(const std::string& filename, Engine* audio_engine)
    {
        bool success = true;
        if (!LoadFile(filename.c_str(), &_soundBankDefSource))
        {
            return false;
        }
        _soundBankDef = GetSoundBankDefinition(_soundBankDefSource.c_str());

        // Load each SoundCollection named in the sound bank.
        for (flatbuffers::uoffset_t i = 0; i < _soundBankDef->filenames()->size(); ++i)
        {
            const char* sound_filename = _soundBankDef->filenames()->Get(i)->c_str();
            success &= InitializeSoundCollection(sound_filename, audio_engine);
        }
        return success;
    }

    static bool DeinitializeSoundCollection(const char* filename, EngineInternalState* state)
    {
        auto id_iter = state->sound_id_map.find(filename);
        if (id_iter == state->sound_id_map.end())
        {
            return false;
        }

        std::string id = id_iter->second;
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

    void SoundBank::Deinitialize(Engine* audio_engine)
    {
        for (flatbuffers::uoffset_t i = 0; i < _soundBankDef->filenames()->size(); ++i)
        {
            const char* filename = _soundBankDef->filenames()->Get(i)->c_str();
            if (!DeinitializeSoundCollection(filename, audio_engine->GetState()))
            {
                CallLogFunc("Error while deinitializing SoundCollection %s in SoundBank.\n", filename);
                AMPLITUDE_ASSERT(0);
            }
        }
    }

} // namespace SparkyStudios::Audio::Amplitude
