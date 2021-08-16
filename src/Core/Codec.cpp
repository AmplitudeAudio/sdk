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

#include <SparkyStudios/Audio/Amplitude/Core/Codec.h>

namespace SparkyStudios::Audio::Amplitude
{
    typedef std::map<AmString, Codec*> CodecRegistry;
    typedef CodecRegistry::value_type CodecImpl;

    static CodecRegistry& codecRegistry()
    {
        static CodecRegistry r;
        return r;
    }

    static bool& lockCodecs()
    {
        static bool b = false;
        return b;
    }

    static AmUInt32& codecsCount()
    {
        static AmUInt32 c = 0;
        return c;
    }

    Codec::Codec(AmString name)
        : m_name(name)
    {
        Codec::Register(this);
    }

    void Codec::Register(Codec* codec)
    {
        if (lockCodecs())
            return;

        if (!Find(codec->GetName()))
        {
            CodecRegistry& codecs = codecRegistry();
            codecs.insert(CodecImpl(codec->GetName(), codec));
            codecsCount()++;
        }
    }

    Codec* Codec::Find(AmString name)
    {
        CodecRegistry& codecs = codecRegistry();
        for (auto&& codec : codecs)
        {
            if (strcmp(codec.second->m_name, name) == 0)
                return codec.second;
        }
        return nullptr;
    }

    Codec* Codec::FindCodecForFile(AmString filePath)
    {
        CodecRegistry& codecs = codecRegistry();
        for (auto&& codec : codecs)
        {
            if (codec.second->CanHandleFile(filePath))
                return codec.second;
        }
        return nullptr;
    }

    void Codec::LockRegistry()
    {
        lockCodecs() = true;
    }
} // namespace SparkyStudios::Audio::Amplitude