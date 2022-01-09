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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Sound/SoundProcessor.h>

#include <Mixer/SoundData.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    typedef std::map<std::string, SoundProcessor*> SoundProcessorRegistry;
    typedef SoundProcessorRegistry::value_type SoundProcessorImpl;

    static SoundProcessorRegistry& soundProcessorRegistry()
    {
        static SoundProcessorRegistry r;
        return r;
    }

    static bool& lockSoundProcessors()
    {
        static bool b = false;
        return b;
    }

    static AmUInt32& soundProcessorsCount()
    {
        static AmUInt32 c = 0;
        return c;
    }

    SoundProcessor::SoundProcessor(const std::string& name)
        : m_name(name)
    {
        SoundProcessor::Register(this);
    }

    SoundProcessor::SoundProcessor()
        : m_name()
    {}

    AmSize SoundProcessor::GetOutputBufferSize(AmUInt64 frames, AmUInt64 bufferSize, AmUInt16 channels, AmUInt32 sampleRate)
    {
        return bufferSize;
    }

    const std::string& SoundProcessor::GetName() const
    {
        return m_name;
    }

    void SoundProcessor::Register(SoundProcessor* codec)
    {
        if (lockSoundProcessors())
            return;

        if (!Find(codec->GetName()))
        {
            SoundProcessorRegistry& processors = soundProcessorRegistry();
            processors.insert(SoundProcessorImpl(codec->GetName(), codec));
            soundProcessorsCount()++;
        }
    }

    SoundProcessor* SoundProcessor::Find(const std::string& name)
    {
        SoundProcessorRegistry& processors = soundProcessorRegistry();
        for (auto&& processor : processors)
        {
            if (processor.second->m_name.compare(name) == 0)
                return processor.second;
        }
        return nullptr;
    }

    void SoundProcessor::LockRegistry()
    {
        lockSoundProcessors() = true;
    }

    ProcessorMixer::ProcessorMixer()
        : _dryProcessor(nullptr)
        , _wetProcessor(nullptr)
        , _dry(0)
        , _wet(0)
    {}

    void ProcessorMixer::SetDryProcessor(SoundProcessor* processor, AmReal32 dry)
    {
        _dryProcessor = processor;
        _dry = AM_CLAMP(dry, 0, 1);
    }

    void ProcessorMixer::SetWetProcessor(SoundProcessor* processor, AmReal32 wet)
    {
        _wetProcessor = processor;
        _wet = AM_CLAMP(wet, 0, 1);
    }

    void ProcessorMixer::Process(
        AmInt16Buffer out,
        AmInt16Buffer in,
        AmUInt64 frames,
        AmUInt64 bufferSize,
        AmUInt16 channels,
        AmUInt32 sampleRate,
        SoundInstance* sound)
    {
        if (_dryProcessor == nullptr && _wetProcessor == nullptr)
        {
            if (out != in)
            {
                memcpy(out, in, bufferSize);
            }

            return;
        }

        AmInt16Buffer dryOut = static_cast<AmInt16Buffer>(amMemory->Malign(MemoryPoolKind::Amplimix, bufferSize, AM_SIMD_ALIGNMENT));
        AmInt16Buffer wetOut = static_cast<AmInt16Buffer>(amMemory->Malign(MemoryPoolKind::Amplimix, bufferSize, AM_SIMD_ALIGNMENT));

        _dryProcessor->Process(dryOut, in, frames, bufferSize, channels, sampleRate, sound);
        _wetProcessor->Process(wetOut, in, frames, bufferSize, channels, sampleRate, sound);

        for (AmUInt64 i = 0; i < bufferSize; i++)
        {
            // clang-format off
            out[i] = (dryOut[i] * AmFloatToFixedPoint(_dry) >> kAmFixedPointShift) + ((wetOut[i] - dryOut[i]) * AmFloatToFixedPoint(_wet) >> kAmFixedPointShift);
            out[i] = AM_CLAMP(out[i], INT16_MIN, INT16_MAX);
            // clang-format on
        }

        amMemory->Free(MemoryPoolKind::Amplimix, dryOut);
        amMemory->Free(MemoryPoolKind::Amplimix, wetOut);
    }

    void ProcessorMixer::ProcessInterleaved(
        AmInt16Buffer out,
        AmInt16Buffer in,
        AmUInt64 frames,
        AmUInt64 bufferSize,
        AmUInt16 channels,
        AmUInt32 sampleRate,
        SoundInstance* sound)
    {
        auto* snd = static_cast<SoundData*>(sound->GetUserData());

        if (_dryProcessor == nullptr && _wetProcessor == nullptr)
        {
            if (out != in)
            {
                memcpy(out, in, bufferSize);
            }

            return;
        }

        AmInt16Buffer dryOut = static_cast<AmInt16Buffer>(amMemory->Malign(MemoryPoolKind::Amplimix, bufferSize, AM_SIMD_ALIGNMENT));
        AmInt16Buffer wetOut = static_cast<AmInt16Buffer>(amMemory->Malign(MemoryPoolKind::Amplimix, bufferSize, AM_SIMD_ALIGNMENT));

        _dryProcessor->Process(dryOut, in, frames, bufferSize, channels, sampleRate, sound);
        _wetProcessor->Process(wetOut, in, frames, bufferSize, channels, sampleRate, sound);

        for (AmUInt64 i = 0; i < bufferSize; i++)
        {
            // clang-format off
            out[i] = (dryOut[i] * AmFloatToFixedPoint(_dry) >> kAmFixedPointShift) + ((wetOut[i] - dryOut[i]) * AmFloatToFixedPoint(_wet) >> kAmFixedPointShift);
            out[i] = AM_CLAMP(out[i], INT16_MIN, INT16_MAX);
            // clang-format on
        }

        amMemory->Free(MemoryPoolKind::Amplimix, dryOut);
        amMemory->Free(MemoryPoolKind::Amplimix, wetOut);
    }
} // namespace SparkyStudios::Audio::Amplitude
