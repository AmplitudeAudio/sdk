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

#include <SparkyStudios/Audio/Amplitude/Core/Asset.h>

#include <Core/EngineInternalState.h>

namespace SparkyStudios::Audio::Amplitude
{
    template<typename Id, typename Definition>
    Asset<Id, Definition>::~Asset()
    {
        AMPLITUDE_ASSERT(_refCounter.GetCount() == 0);

        _id = kAmInvalidObjectId;
        _source.clear();
    }

    template<typename Id, typename Definition>
    bool Asset<Id, Definition>::LoadDefinitionFromFile(std::shared_ptr<File> file, EngineInternalState* state)
    {
        // Ensure we do not load the asset more than once
        AMPLITUDE_ASSERT(_id == kAmInvalidObjectId);

        AmString source;
        if (!LoadFile(file, &source))
            return false;

        _source = source;

        return LoadDefinition(GetDefinition(), state);
    }

    template<typename Id, typename Definition>
    bool Asset<Id, Definition>::LoadDefinitionFromPath(const AmOsString& path, EngineInternalState* state)
    {
        const FileSystem* fs = amEngine->GetFileSystem();
        const AmOsString& rp = fs->ResolvePath(path);

        return LoadDefinitionFromFile(fs->OpenFile(rp), state);
    }

    template<typename Id, typename Definition>
    Id Asset<Id, Definition>::GetId() const
    {
        return _id;
    }

    template<typename Id, typename Definition>
    const AmString& Asset<Id, Definition>::GetName() const
    {
        return _name;
    }

    template<typename Id, typename Definition>
    void Asset<Id, Definition>::AcquireReferences(EngineInternalState* state)
    {}

    template<typename Id, typename Definition>
    void Asset<Id, Definition>::ReleaseReferences(EngineInternalState* state)
    {}

    template<typename Id, typename Definition>
    RefCounter* Asset<Id, Definition>::GetRefCounter()
    {
        return &_refCounter;
    }

    template<typename Id, typename Definition>
    const RefCounter* Asset<Id, Definition>::GetRefCounter() const
    {
        return &_refCounter;
    }

    template class Asset<AmAttenuationID, AttenuationDefinition>;
    template class Asset<AmCollectionID, CollectionDefinition>;
    template class Asset<AmEffectID, EffectDefinition>;
    template class Asset<AmEventID, EventDefinition>;
    template class Asset<AmRtpcID, RtpcDefinition>;
    template class Asset<AmSoundID, SoundDefinition>;
    template class Asset<AmSwitchContainerID, SwitchContainerDefinition>;
    template class Asset<AmSwitchID, SwitchDefinition>;
} // namespace SparkyStudios::Audio::Amplitude
