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

#include <SparkyStudios/Audio/Amplitude/Sound/SoundObject.h>

namespace SparkyStudios::Audio::Amplitude
{
    SoundObject::SoundObject()
        : m_bus(nullptr)
        , m_gain()
        , m_priority()
        , m_effect(nullptr)
        , m_attenuation(nullptr)
    {}

    const RtpcValue& SoundObject::GetGain() const
    {
        return m_gain;
    }

    const RtpcValue& SoundObject::GetPriority() const
    {
        return m_priority;
    }

    const Effect* SoundObject::GetEffect() const
    {
        return m_effect;
    }

    const Attenuation* SoundObject::GetAttenuation() const
    {
        return m_attenuation;
    }

    Bus SoundObject::GetBus() const
    {
        return Bus(m_bus);
    }
} // namespace SparkyStudios::Audio::Amplitude
