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

#include <SparkyStudios/Audio/Amplitude/Sound/Fader.h>

namespace SparkyStudios::Audio::Amplitude
{
    typedef std::map<std::string, Fader*> FaderRegistry;
    typedef FaderRegistry::value_type FaderImpl;

    FaderInstance::FaderInstance()
    {
        m_from = m_to = m_delta = 0;
        m_time = m_startTime = m_endTime = 0;
        m_state = AM_FADER_STATE_DISABLED;
    }

    void FaderInstance::Set(AmReal64 from, AmReal64 to, AmTime duration)
    {
        SetDuration(duration);
        Set(from, to);
        Start(0.0);
    }

    void FaderInstance::Set(AmReal64 from, AmReal64 to)
    {
        m_from = from;
        m_to = to;
        m_delta = to - from;
    }

    void FaderInstance::SetDuration(AmTime duration)
    {
        m_time = duration;
    }

    AmReal64 FaderInstance::GetFromTime(AmTime time)
    {
        if (m_state != AM_FADER_STATE_ACTIVE)
            return 0.0f;

        if (m_startTime >= time)
            return m_from;

        if (time >= m_endTime)
            return m_to;

        return GetFromPercentage((time - m_startTime) / (m_endTime - m_startTime));
    }

    void FaderInstance::Start(AmTime time)
    {
        m_startTime = time;
        m_endTime = m_startTime + m_time;
        m_state = AM_FADER_STATE_ACTIVE;
    }

    static FaderRegistry& faderRegistry()
    {
        static FaderRegistry r;
        return r;
    }

    static bool& lockFaders()
    {
        static bool b = false;
        return b;
    }

    static AmUInt32& fadersCount()
    {
        static AmUInt32 c = 0;
        return c;
    }

    Fader::Fader(std::string name)
        : m_name(std::move(name))
    {
        Fader::Register(this);
    }

    Fader::Fader()
        : m_name()
    {}

    const std::string& Fader::GetName() const
    {
        return m_name;
    }

    void Fader::Register(Fader* codec)
    {
        if (lockFaders())
            return;

        if (Find(codec->GetName()) != nullptr)
            return;

        FaderRegistry& faders = faderRegistry();
        faders.insert(FaderImpl(codec->GetName(), codec));
        fadersCount()++;
    }

    Fader* Fader::Find(const std::string& name)
    {
        FaderRegistry& faders = faderRegistry();
        for (auto&& fader : faders)
            if (fader.second->m_name == name)
                return fader.second;

        return nullptr;
    }

    FaderInstance* Fader::Construct(const std::string& name)
    {
        Fader* fader = Find(name);
        if (fader == nullptr)
            return nullptr;

        return fader->CreateInstance();
    }

    void Fader::Destruct(const std::string& name, FaderInstance* instance)
    {
        if (instance == nullptr)
            return;

        Fader* fader = Find(name);
        if (fader == nullptr)
            return;

        fader->DestroyInstance(instance);
    }

    void Fader::LockRegistry()
    {
        lockFaders() = true;
    }
} // namespace SparkyStudios::Audio::Amplitude