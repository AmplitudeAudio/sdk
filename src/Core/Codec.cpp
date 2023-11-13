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

#include <map>
#include <utility>

#include <SparkyStudios/Audio/Amplitude/Core/Codec.h>
#include <SparkyStudios/Audio/Amplitude/Core/Log.h>

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
        : m_name(std::move(name))
    {
        Register(this);
    }

    void Codec::Register(Codec* codec)
    {
        if (lockCodecs())
            return;

        if (Find(codec->GetName()) != nullptr)
        {
            CallLogFunc("Failed to register codec '%s' as it is already registered", codec->GetName().c_str());
            return;
        }

        CodecRegistry& codecs = codecRegistry();
        codecs.insert(CodecImpl(codec->GetName(), codec));
        codecsCount()++;
    }

    Codec* Codec::Find(const AmString& name)
    {
        const CodecRegistry& codecs = codecRegistry();
        if (const auto& it = codecs.find(name); it != codecs.end())
            return it->second;

        return nullptr;
    }

    Codec* Codec::FindCodecForFile(const AmOsString& filePath)
    {
        const CodecRegistry& codecs = codecRegistry();
        for (const auto& [_, codec] : codecs)
            if (codec->CanHandleFile(filePath))
                return codec;

        return nullptr;
    }

    void Codec::LockRegistry()
    {
        lockCodecs() = true;
    }
} // namespace SparkyStudios::Audio::Amplitude