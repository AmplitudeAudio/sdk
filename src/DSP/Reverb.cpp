// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/DSP/Reverb.h>

namespace SparkyStudios::Audio::Amplitude
{
    typedef std::map<AmString, std::shared_ptr<Reverb>> ReverbRegistry;
    typedef ReverbRegistry::value_type ReverbImpl;

    static ReverbRegistry& reverbRegistry()
    {
        static ReverbRegistry r;
        return r;
    }

    static bool& lockReverbs()
    {
        static bool b = false;
        return b;
    }

    static AmUInt32& reverbsCount()
    {
        static AmUInt32 c = 0;
        return c;
    }

    ReverbInstance::ReverbInstance(Reverb* parent)
        : _parent(parent)
    {}

    Reverb* ReverbInstance::GetParent() const
    {
        return _parent;
    }

    Reverb::Reverb(AmString name)
        : m_name(std::move(name))
    {}

    const AmString& Reverb::GetName() const
    {
        return m_name;
    }

    void Reverb::Register(std::shared_ptr<Reverb> reverb)
    {
        if (lockReverbs() || reverb == nullptr)
            return;

        if (Find(reverb->GetName()) != nullptr)
            return;

        ReverbRegistry& reverbs = reverbRegistry();
        reverbs.insert(ReverbImpl(reverb->GetName(), reverb));
        reverbsCount()++;
    }

    void Reverb::Unregister(std::shared_ptr<const Reverb> reverb)
    {
        if (lockReverbs() || reverb == nullptr)
            return;

        ReverbRegistry& reverbs = reverbRegistry();
        if (const auto& it = reverbs.find(reverb->GetName()); it != reverbs.end())
        {
            reverbs.erase(it);
            reverbsCount()--;
        }
    }

    std::shared_ptr<Reverb> Reverb::Find(const AmString& name)
    {
        for (const ReverbRegistry& reverbs = reverbRegistry(); auto&& reverb : reverbs)
            if (reverb.second->m_name == name)
                return reverb.second;

        return nullptr;
    }

    void Reverb::LockRegistry()
    {
        lockReverbs() = true;
    }

    void Reverb::UnlockRegistry()
    {
        lockReverbs() = false;
    }

    const std::map<AmString, std::shared_ptr<Reverb>>& Reverb::GetRegistry()
    {
        return reverbRegistry();
    }
} // namespace SparkyStudios::Audio::Amplitude
