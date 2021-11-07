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
#include <Sound/Filters/EchoFilter.h>
#include <Sound/Filters/EqualizerFilter.h>
#include <Sound/Filters/FlangerFilter.h>
#include <Sound/Filters/FreeverbFilter.h>
#include <Sound/Filters/RobotizeFilter.h>

#include "effect_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    static std::vector<EffectInstance*> gEffectsList = {};

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
        gEffectsList.push_back(effect);
        return effect;
    }

    void Effect::DeleteInstance(EffectInstance* instance) const
    {
        if (instance == nullptr)
            return;

        gEffectsList.erase(std::find(gEffectsList.begin(), gEffectsList.end(), instance));
    }

    void Effect::Update()
    {
        // Update effect parameters
        for (auto&& instance : gEffectsList)
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
