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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/EngineInternalState.h>

#include "effect_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    static std::unordered_map<AmEffectID, std::vector<EffectInstance*>> gEffectsList = {};

    Effect::Effect()
        : _source()
        , _id(kAmInvalidObjectId)
        , _name()
        , _parameters()
        , _refCounter()
        , _filter(nullptr)
    {}

    Effect::~Effect()
    {
        _filter = nullptr;
        _parameters.clear();

        for (auto&& instance : gEffectsList[_id])
            DestroyInstance(instance);

        gEffectsList.erase(_id);
    }

    bool Effect::LoadEffectDefinition(const std::string& source)
    {
        // Ensure we do not load the collection more than once
        AMPLITUDE_ASSERT(_id == kAmInvalidObjectId);

        _source = source;
        const EffectDefinition* def = GetEffectDefinition();

        _id = def->id();
        _name = def->name()->str();

        _filter = Filter::Find(def->effect()->str());

        if (_filter == nullptr)
        {
            CallLogFunc("[ERROR] Effect %s specifies an invalid effect type: %s.", def->name()->c_str(), def->effect()->c_str());
            return false;
        }

        flatbuffers::uoffset_t paramCount = def->parameters() ? def->parameters()->size() : 0;

        _parameters.resize(paramCount);

        for (flatbuffers::uoffset_t i = 0; i < paramCount; ++i)
        {
            _parameters[i] = RtpcValue(def->parameters()->Get(i));
        }

        return true;
    }

    bool Effect::LoadEffectDefinitionFromFile(AmOsString filename)
    {
        std::string source;
        return Amplitude::LoadFile(filename, &source) && LoadEffectDefinition(source);
    }

    void Effect::AcquireReferences(EngineInternalState* state)
    {}

    void Effect::ReleaseReferences(EngineInternalState* state)
    {}

    const EffectDefinition* Effect::GetEffectDefinition() const
    {
        return Amplitude::GetEffectDefinition(_source.c_str());
    }

    EffectInstance* Effect::CreateInstance() const
    {
        auto* effect = new EffectInstance(this);
        auto& list = gEffectsList[_id];
        list.push_back(effect);
        return effect;
    }

    void Effect::DestroyInstance(EffectInstance* instance) const
    {
        if (instance == nullptr)
            return;

        auto& list = gEffectsList[_id];
        list.erase(std::find(list.begin(), list.end(), instance));
    }

    void Effect::Update()
    {
        // Update effect parameters
        for (auto&& instance : gEffectsList[_id])
        {
            for (AmSize i = 0, l = _parameters.size(); i < l; ++i)
            {
                instance->GetFilter()->SetFilterParameter(i, _parameters[i].GetValue());
            }
        }
    }

    AmEffectID Effect::GetId() const
    {
        return _id;
    }

    const std::string& Effect::GetName() const
    {
        return _name;
    }

    RefCounter* Effect::GetRefCounter()
    {
        return &_refCounter;
    }

    const RefCounter* Effect::GetRefCounter() const
    {
        return &_refCounter;
    }

    EffectInstance::EffectInstance(const Effect* parent)
        : _parent(parent)
    {
        _filterInstance = _parent->_filter->CreateInstance();

        // First initialization of filter parameters
        for (AmUInt32 i = 0; i < _parent->_parameters.size(); ++i)
        {
            _filterInstance->SetFilterParameter(i, _parent->_parameters[i].GetValue());
        }
    }

    EffectInstance::~EffectInstance()
    {
        _parent->_filter->DestroyInstance(_filterInstance);
        _filterInstance = nullptr;

        _parent->DestroyInstance(this);
        _parent = nullptr;
    }

    FilterInstance* EffectInstance::GetFilter() const
    {
        return _filterInstance;
    }
} // namespace SparkyStudios::Audio::Amplitude
