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

#include <Core/Engine.h>
#include <Sound/Rtpc.h>

namespace SparkyStudios::Audio::Amplitude
{
    RtpcImpl::RtpcImpl()
        : _minValue(0.0)
        , _maxValue(1.0)
        , _defValue(0.0)
        , _currentValue(0.0)
        , _targetValue(0.0)
        , _faderAttack(nullptr)
        , _faderRelease(nullptr)
    {}

    RtpcImpl::~RtpcImpl()
    {
        _faderAttack = nullptr;
        _faderRelease = nullptr;
    }

    void RtpcImpl::Update(AmTime deltaTime)
    {
        if (_faderRelease && _currentValue > _targetValue)
            _currentValue = _faderRelease->GetFromTime(Engine::GetInstance()->GetTotalTime());

        if (_faderAttack && _currentValue < _targetValue)
            _currentValue = _faderAttack->GetFromTime(Engine::GetInstance()->GetTotalTime());

        _currentValue = std::clamp(_currentValue, _minValue, _maxValue);
    }

    void RtpcImpl::SetValue(AmReal64 value)
    {
        _targetValue = std::clamp(value, _minValue, _maxValue);

        if (_faderAttack == nullptr || _faderRelease == nullptr)
        {
            _currentValue = _targetValue;
        }
        else
        {
            _faderAttack->Set(_currentValue, _targetValue);
            _faderRelease->Set(_currentValue, _targetValue);

            _faderAttack->Start(Engine::GetInstance()->GetTotalTime());
            _faderRelease->Start(Engine::GetInstance()->GetTotalTime());
        }
    }

    void RtpcImpl::Reset()
    {
        SetValue(_defValue);
    }

    bool RtpcImpl::LoadDefinition(const RtpcDefinition* definition, std::shared_ptr<EngineInternalState> state)
    {
        m_id = definition->id();
        m_name = definition->name()->str();

        _minValue = definition->min_value();
        _maxValue = definition->max_value();
        _defValue = definition->default_value();

        if (definition->fade_settings() && definition->fade_settings()->enabled())
        {
            const auto faderAttackFactory = Fader::Find(definition->fade_settings()->fade_attack()->fader()->str());
            const auto faderReleaseFactory = Fader::Find(definition->fade_settings()->fade_release()->fader()->str());

            if (faderAttackFactory != nullptr)
            {
                _faderAttack = faderAttackFactory->CreateInstance();
                _faderAttack->SetDuration(definition->fade_settings()->fade_attack()->duration());
            }

            if (faderReleaseFactory != nullptr)
            {
                _faderRelease = faderReleaseFactory->CreateInstance();
                _faderRelease->SetDuration(definition->fade_settings()->fade_release()->duration());
            }
        }

        return true;
    }

    const RtpcDefinition* RtpcImpl::GetDefinition() const
    {
        return GetRtpcDefinition(m_source.c_str());
    }

    void RtpcValue::Init(RtpcValue& value, const RtpcCompatibleValue* definition, AmReal32 staticValue)
    {
        if (definition == nullptr)
            value.Init(staticValue);
        else
            value.Init(definition);
    }

    RtpcValue::RtpcValue()
        : _valueKind(ValueKind_None)
        , _value(0.0f)
        , _curve(nullptr)
        , _rtpc(nullptr)
        , _initialized(false)
    {}

    RtpcValue::RtpcValue(const RtpcValue& other)
    {
        _valueKind = other._valueKind;
        _value = other._value;
        _curve = other._curve;
        _rtpc = other._rtpc;
        _initialized = true;
    }

    void RtpcValue::Init(AmReal32 value)
    {
        _valueKind = ValueKind_Static;
        _value = value;
        _curve = nullptr;
        _rtpc = nullptr;
        _initialized = true;
    }

    void RtpcValue::Init(const Rtpc* rtpc, std::shared_ptr<Curve> curve)
    {
        _valueKind = ValueKind_RTPC;
        _value = 0.0f;
        _curve = curve;
        _rtpc = rtpc;
        _initialized = false;
    }

    void RtpcValue::Init(const RtpcCompatibleValue* definition)
    {
        _valueKind = definition->kind();
        _value = definition->value();
        _curve = nullptr;
        _rtpc = nullptr;

        if (definition->kind() == ValueKind_RTPC)
        {
            _rtpc = amEngine->GetRtpcHandle(definition->rtpc()->id());

            if (_rtpc == nullptr)
            {
                amLogError("Linking a parameter to an invalid or uninitialized RTPC handle.");
            }

            auto curve = AmSharedPtr<Curve, eMemoryPoolKind_Engine>::Make();
            curve->Initialize(definition->rtpc()->curve());

            _curve = std::move(curve);
        }

        _initialized = true;
    }

    RtpcValue::~RtpcValue()
    {
        _rtpc = nullptr;
        _curve = nullptr;
    }

    float RtpcValue::GetValue() const
    {
        if (_valueKind == ValueKind_RTPC && _rtpc != nullptr)
            return _curve->Get((_rtpc->GetValue() - _rtpc->GetMinValue()) / (_rtpc->GetMaxValue() - _rtpc->GetMinValue()));

        if (_valueKind == ValueKind_Static)
            return _value;

        return 0.0f;
    }

    bool RtpcValue::IsStatic() const
    {
        return _valueKind == ValueKind_Static;
    }
} // namespace SparkyStudios::Audio::Amplitude
