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

#include <SparkyStudios/Audio/Amplitude/Core/Log.h>
#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/SoundProcessor.h>

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

    static AmUInt32& resamplersCount()
    {
        static AmUInt32 c = 0;
        return c;
    }

    AmSize SoundProcessorInstance::GetOutputBufferSize(AmUInt64 frames, AmSize bufferSize, AmUInt16 channels, AmUInt32 sampleRate)
    {
        return bufferSize;
    }

    void SoundProcessorInstance::Cleanup(SoundInstance* sound)
    {
        // Do nothing in base class
    }

    SoundProcessor::SoundProcessor(std::string name)
        : m_name(std::move(name))
    {
        Register(this);
    }

    SoundProcessor::SoundProcessor()
        : m_name()
    {}

    const std::string& SoundProcessor::GetName() const
    {
        return m_name;
    }

    void SoundProcessor::Register(SoundProcessor* codec)
    {
        if (lockSoundProcessors())
            return;

        if (Find(codec->GetName()) != nullptr)
        {
            CallLogFunc("Failed to register sound processor '%s' as it is already registered", codec->GetName().c_str());
            return;
        }

        SoundProcessorRegistry& processors = soundProcessorRegistry();
        processors.insert(SoundProcessorImpl(codec->GetName(), codec));
        resamplersCount()++;
    }

    SoundProcessor* SoundProcessor::Find(const std::string& name)
    {
        SoundProcessorRegistry& processors = soundProcessorRegistry();
        if (const auto& it = processors.find(name); it != processors.end())
            return it->second;

        return nullptr;
    }

    SoundProcessorInstance* SoundProcessor::Construct(const AmString& name)
    {
        SoundProcessor* processor = Find(name);
        if (processor == nullptr)
            return nullptr;

        return processor->CreateInstance();
    }

    void SoundProcessor::Destruct(const std::string& name, SoundProcessorInstance* instance)
    {
        SoundProcessor* processor = Find(name);
        if (processor == nullptr)
            return;

        processor->DestroyInstance(instance);
    }

    void SoundProcessor::LockRegistry()
    {
        lockSoundProcessors() = true;
    }

    ProcessorMixer::ProcessorMixer()
        : _wetProcessor(nullptr)
        , _dryProcessor(nullptr)
        , _wet(0)
        , _dry(0)
    {}

    ProcessorMixer::~ProcessorMixer()
    {
        if (_dryProcessor != nullptr)
            ampooldelete(MemoryPoolKind::Filtering, SoundProcessorInstance, _dryProcessor);

        if (_wetProcessor != nullptr)
            ampooldelete(MemoryPoolKind::Filtering, SoundProcessorInstance, _wetProcessor);
    }

    void ProcessorMixer::SetDryProcessor(SoundProcessorInstance* processor, AmReal32 dry)
    {
        _dryProcessor = processor;
        _dry = AM_CLAMP(dry, 0, 1);
    }

    void ProcessorMixer::SetWetProcessor(SoundProcessorInstance* processor, AmReal32 wet)
    {
        _wetProcessor = processor;
        _wet = AM_CLAMP(wet, 0, 1);
    }

    void ProcessorMixer::Process(
        AmAudioSampleBuffer out,
        AmConstAudioSampleBuffer in,
        AmUInt64 frames,
        AmSize bufferSize,
        AmUInt16 channels,
        AmUInt32 sampleRate,
        SoundInstance* sound)
    {
        if (_dryProcessor == nullptr || _wetProcessor == nullptr)
        {
            if (out != in)
                std::memcpy(out, in, bufferSize);

            return;
        }

        const auto dryOut = static_cast<AmAudioSampleBuffer>(ampoolmalign(MemoryPoolKind::Amplimix, bufferSize, AM_SIMD_ALIGNMENT));
        const auto wetOut = static_cast<AmAudioSampleBuffer>(ampoolmalign(MemoryPoolKind::Amplimix, bufferSize, AM_SIMD_ALIGNMENT));

        std::memcpy(dryOut, in, bufferSize);
        std::memcpy(wetOut, in, bufferSize);

        _dryProcessor->Process(dryOut, in, frames, bufferSize, channels, sampleRate, sound);
        _wetProcessor->Process(wetOut, in, frames, bufferSize, channels, sampleRate, sound);

        for (AmUInt64 i = 0, l = frames * channels; i < l; i++)
        {
            out[i] = dryOut[i] * _dry + (wetOut[i] - dryOut[i]) * _wet;
            out[i] = AM_CLAMP_AUDIO_SAMPLE(out[i]);
        }

        ampoolfree(MemoryPoolKind::Amplimix, dryOut);
        ampoolfree(MemoryPoolKind::Amplimix, wetOut);
    }
} // namespace SparkyStudios::Audio::Amplitude
