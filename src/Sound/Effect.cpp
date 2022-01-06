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

#include <Sound/Filters/BassBoostFilter.h>
#include <Sound/Filters/BiquadResonantFilter.h>
#include <Sound/Filters/EchoFilter.h>
#include <Sound/Filters/EqualizerFilter.h>
#include <Sound/Filters/FlangerFilter.h>
#include <Sound/Filters/FreeverbFilter.h>
#include <Sound/Filters/RobotizeFilter.h>

#include "effect_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    static std::map<const Effect*, std::vector<EffectInstance*>> gEffectsList = {};

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
        delete _filter;
        _filter = nullptr;
        _parameters.clear();

        for (auto&& instance : gEffectsList[this])
            DeleteInstance(instance);

        gEffectsList.erase(this);
    }

    bool Effect::LoadEffectDefinition(const std::string& source)
    {
        // Ensure we do not load the collection more than once
        AMPLITUDE_ASSERT(_id == kAmInvalidObjectId);

        _source = source;
        const EffectDefinition* def = GetEffectDefinition();

        _id = def->id();
        _name = def->name()->str();

        switch (def->effect())
        {
        case EffectKind_BassBoost:
            _filter = new BassBoostFilter();
            break;

        case EffectKind_Echo:
            _filter = new EchoFilter();
            break;

        case EffectKind_Equalizer:
            _filter = new EqualizerFilter();
            break;

        case EffectKind_Flanger:
            _filter = new FlangerFilter();
            break;

        case EffectKind_Freeverb:
            _filter = new FreeverbFilter();
            break;

        case EffectKind_Robotize:
            _filter = new RobotizeFilter();
            break;

        case EffectKind_BiquadResonant:
            _filter = new BiquadResonantFilter();
            break;

        default:
            CallLogFunc("[ERROR] Effect %s specifies an invalid effect type: %s.", def->name()->c_str(), EnumNameEffectKind(def->effect()));
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
        auto& list = gEffectsList[this];
        list.push_back(effect);
        return effect;
    }

    void Effect::DeleteInstance(EffectInstance* instance) const
    {
        if (instance == nullptr)
            return;

        auto& list = gEffectsList[this];
        list.erase(std::find(list.begin(), list.end(), instance));
    }

    void Effect::Update()
    {
        // Update effect parameters
        for (auto&& instance : gEffectsList[this])
        {
            for (AmUInt32 i = 0; i < _parameters.size(); ++i)
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

    EffectInstance::EffectInstance(const Effect* parent)
        : _parent(parent)
    {
        _filterInstance = parent->_filter->CreateInstance();

        // First initialization of filter parameters
        for (AmUInt32 i = 0; i < _parent->_parameters.size(); ++i)
        {
            _filterInstance->SetFilterParameter(i, _parent->_parameters[i].GetValue());
        }
    }

    EffectInstance::~EffectInstance()
    {
        _parent->DeleteInstance(this);
        _parent = nullptr;

        delete _filterInstance;
        _filterInstance = nullptr;
    }

    FilterInstance* EffectInstance::GetFilter() const
    {
        return _filterInstance;
    }
} // namespace SparkyStudios::Audio::Amplitude
