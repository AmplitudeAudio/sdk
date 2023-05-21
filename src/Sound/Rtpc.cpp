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

#include <SparkyStudios/Audio/Amplitude/Math/Curve.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Rtpc.h>

#include <Core/EngineInternalState.h>

#include "rtpc_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    Rtpc::Rtpc()
        : _source()
        , _id(kAmInvalidObjectId)
        , _name()
        , _currentValue(0.0)
        , _targetValue(0.0)
        , _minValue(0.0)
        , _maxValue(1.0)
        , _defValue(0.0)
        , _faderAttackFactory(nullptr)
        , _faderReleaseFactory(nullptr)
        , _faderAttack(nullptr)
        , _faderRelease(nullptr)
        , _refCounter()
    {}

    Rtpc::~Rtpc()
    {
        if (_faderAttackFactory != nullptr)
            _faderAttackFactory->DestroyInstance(_faderAttack);

        if (_faderReleaseFactory != nullptr)
            _faderReleaseFactory->DestroyInstance(_faderRelease);

        _faderAttackFactory = nullptr;
        _faderReleaseFactory = nullptr;

        _faderAttack = nullptr;
        _faderRelease = nullptr;

        _source.clear();
        _name.clear();
    }

    bool Rtpc::LoadRtpcDefinition(const std::string& source)
    {
        AMPLITUDE_ASSERT(_id == kAmInvalidObjectId);

        _source = source;
        const RtpcDefinition* definition = GetRtpcDefinition();

        _id = definition->id();
        _name = definition->name()->str();

        _minValue = definition->min_value();
        _maxValue = definition->max_value();
        _defValue = definition->default_value();

        if (definition->fade_settings() && definition->fade_settings()->enabled())
        {
            _faderAttackFactory = Fader::Find(definition->fade_settings()->fade_attack()->fader()->str());
            _faderReleaseFactory = Fader::Find(definition->fade_settings()->fade_release()->fader()->str());

            _faderAttack = _faderAttackFactory->CreateInstance();
            _faderAttack->SetDuration(definition->fade_settings()->fade_attack()->duration());

            _faderRelease = _faderReleaseFactory->CreateInstance();
            _faderRelease->SetDuration(definition->fade_settings()->fade_release()->duration());
        }

        return true;
    }

    bool Rtpc::LoadRtpcDefinitionFromFile(const AmOsString& filename)
    {
        std::string source;
        return Amplitude::LoadFile(filename, &source) && LoadRtpcDefinition(source);
    }

    const RtpcDefinition* Rtpc::GetRtpcDefinition() const
    {
        return Amplitude::GetRtpcDefinition(_source.c_str());
    }

    void Rtpc::Update(AmTime deltaTime)
    {
        if (_faderRelease && _currentValue > _targetValue)
        {
            _currentValue = _faderRelease->GetFromTime(Engine::GetInstance()->GetTotalTime());
        }

        if (_faderAttack && _currentValue < _targetValue)
        {
            _currentValue = _faderAttack->GetFromTime(Engine::GetInstance()->GetTotalTime());
        }
    }

    AmRtpcID Rtpc::GetId() const
    {
        return _id;
    }

    const std::string& Rtpc::GetName() const
    {
        return _name;
    }

    double Rtpc::GetMinValue() const
    {
        return _minValue;
    }

    double Rtpc::GetMaxValue() const
    {
        return _maxValue;
    }

    double Rtpc::GetValue() const
    {
        return _currentValue;
    }

    void Rtpc::SetValue(double value)
    {
        _targetValue = AM_CLAMP(value, _minValue, _maxValue);

        if (_faderAttack == nullptr || _faderRelease == nullptr)
        {
            _currentValue = value;
        }
        else
        {
            _faderAttack->Set(_currentValue, _targetValue);
            _faderRelease->Set(_currentValue, _targetValue);

            _faderAttack->Start(Engine::GetInstance()->GetTotalTime());
            _faderRelease->Start(Engine::GetInstance()->GetTotalTime());
        }
    }

    AmReal64 Rtpc::GetDefaultValue() const
    {
        return _defValue;
    }

    void Rtpc::Reset()
    {
        SetValue(_defValue);
    }

    RefCounter* Rtpc::GetRefCounter()
    {
        return &_refCounter;
    }

    RtpcValue::RtpcValue()
        : _valueKind(ValueKind_None)
        , _value(0.0f)
        , _rtpc(nullptr)
        , _curve(nullptr)
    {}

    RtpcValue::RtpcValue(AmReal32 value)
        : _valueKind(ValueKind_Static)
        , _value(value)
        , _rtpc(nullptr)
        , _curve(nullptr)
    {}

    RtpcValue::RtpcValue(const Rtpc* rtpc, const Curve* curve)
        : _valueKind(ValueKind_RTPC)
        , _value(0.0f)
        , _rtpc(rtpc)
        , _curve(curve)
    {}

    RtpcValue::RtpcValue(const RtpcCompatibleValue* definition)
        : _valueKind(definition->kind())
        , _value(definition->value())
        , _rtpc(nullptr)
        , _curve(nullptr)
    {
        if (definition->kind() == ValueKind_RTPC)
        {
            _rtpc = amEngine->GetRtpcHandle(definition->rtpc()->id());

            if (_rtpc == nullptr)
            {
                CallLogFunc("[ERROR] Linking a parameter to an invalid or uninitialized RTP handle.");
            }

            auto* curve = new Curve();
            curve->Initialize(definition->rtpc()->curve());

            _curve = curve;
        }
    }

    float RtpcValue::GetValue() const
    {
        if (_valueKind == ValueKind_RTPC && _rtpc != nullptr)
        {
            return _curve->Get(_rtpc->GetValue());
        }
        else if (_valueKind == ValueKind_Static)
        {
            return _value;
        }

        return 0.0f;
    }
} // namespace SparkyStudios::Audio::Amplitude
