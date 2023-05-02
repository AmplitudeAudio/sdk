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

#include <cstring>
#include <map>
#include <utility>

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Resampler.h>

#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    typedef std::map<std::string, Resampler*> ResamplerRegistry;
    typedef ResamplerRegistry::value_type ResamplerImpl;

    static ResamplerRegistry& resamplerRegistry()
    {
        static ResamplerRegistry r;
        return r;
    }

    static bool& lockResamplers()
    {
        static bool b = false;
        return b;
    }

    static AmUInt32& resamplersCount()
    {
        static AmUInt32 c = 0;
        return c;
    }

    Resampler::Resampler(std::string name)
        : m_name(std::move(name))
    {
        Resampler::Register(this);
    }

    Resampler::Resampler()
        : m_name()
    {}

    const std::string& Resampler::GetName() const
    {
        return m_name;
    }

    void Resampler::Register(Resampler* codec)
    {
        if (lockResamplers())
            return;

        if (Find(codec->GetName()) != nullptr)
            return;

        ResamplerRegistry& resamplers = resamplerRegistry();
        resamplers.insert(ResamplerImpl(codec->GetName(), codec));
        resamplersCount()++;
    }

    Resampler* Resampler::Find(const std::string& name)
    {
        ResamplerRegistry& resamplers = resamplerRegistry();
        for (auto&& resampler : resamplers)
            if (resampler.second->m_name == name)
                return resampler.second;

        return nullptr;
    }

    ResamplerInstance* Resampler::Construct(const std::string& name)
    {
        Resampler* resampler = Find(name);
        if (resampler == nullptr)
            return nullptr;

        return resampler->CreateInstance();
    }

    void Resampler::Destruct(const std::string& name, ResamplerInstance* instance)
    {
        if (instance == nullptr)
            return;

        Resampler* resampler = Find(name);
        if (resampler == nullptr)
            return;

        resampler->DestroyInstance(instance);
    }

    void Resampler::LockRegistry()
    {
        lockResamplers() = true;
    }
} // namespace SparkyStudios::Audio::Amplitude