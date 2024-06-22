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
        : _instances()
        , _parameters()
        , _filter(nullptr)
    {}

    Effect::~Effect()
    {
        for (auto&& instance : gEffectsList[_id])
            DestroyInstance(instance);

        _filter = nullptr;
        _parameters.clear();

        gEffectsList.erase(_id);
    }

    EffectInstance* Effect::CreateInstance() const
    {
        auto* effect = ampoolnew(MemoryPoolKind::Engine, EffectInstance, this);
        auto& list = gEffectsList[_id];
        list.push_back(effect);
        return effect;
    }

    void Effect::DestroyInstance(EffectInstance* instance) const
    {
        if (instance == nullptr)
            return;

        auto& list = gEffectsList[_id];
        list.erase(std::ranges::find(list, instance));

        ampooldelete(MemoryPoolKind::Engine, EffectInstance, instance);
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

    bool Effect::LoadDefinition(const EffectDefinition* definition, EngineInternalState* state)
    {
        _id = definition->id();
        _name = definition->name()->str();

        _filter = Filter::Find(definition->effect()->str());

        if (_filter == nullptr)
        {
            amLogError("Effect {} specifies an invalid effect type: {}.", definition->name()->str(), definition->effect()->str());
            return false;
        }

        const flatbuffers::uoffset_t paramCount = definition->parameters() ? definition->parameters()->size() : 0;
        _parameters.resize(paramCount);

        for (flatbuffers::uoffset_t i = 0; i < paramCount; ++i)
            _parameters[i].Init(definition->parameters()->Get(i));

        gEffectsList[_id] = {};

        return true;
    }

    const EffectDefinition* Effect::GetDefinition() const
    {
        return GetEffectDefinition(_source.c_str());
    }

    EffectInstance::EffectInstance(const Effect* parent)
        : _parent(parent)
    {
        _filterInstance = _parent->_filter->CreateInstance();

        // First initialization of filter parameters
        for (AmSize i = 0, l = _parent->_parameters.size(); i < l; ++i)
            _filterInstance->SetFilterParameter(i, _parent->_parameters[i].GetValue());
    }

    EffectInstance::~EffectInstance()
    {
        _parent->_filter->DestroyInstance(_filterInstance);
        _filterInstance = nullptr;

        _parent = nullptr;
    }

    FilterInstance* EffectInstance::GetFilter() const
    {
        return _filterInstance;
    }
} // namespace SparkyStudios::Audio::Amplitude
