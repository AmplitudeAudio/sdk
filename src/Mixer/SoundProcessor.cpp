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

    SoundProcessor::~SoundProcessor()
    {
        Unregister(this);
    }

    const std::string& SoundProcessor::GetName() const
    {
        return m_name;
    }

    void SoundProcessor::Register(SoundProcessor* processor)
    {
        if (lockSoundProcessors())
            return;

        if (Find(processor->GetName()) != nullptr)
        {
            CallLogFunc("Failed to register sound processor '%s' as it is already registered", processor->GetName().c_str());
            return;
        }

        SoundProcessorRegistry& processors = soundProcessorRegistry();
        processors.insert(SoundProcessorImpl(processor->GetName(), processor));
        soundProcessorsCount()++;
    }

    void SoundProcessor::Unregister(const SoundProcessor* processor)
    {
        if (lockSoundProcessors())
            return;

        SoundProcessorRegistry& processors = soundProcessorRegistry();
        if (const auto& it = processors.find(processor->GetName()); it != processors.end())
        {
            processors.erase(it);
            soundProcessorsCount()--;
        }
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

    void SoundProcessor::UnlockRegistry()
    {
        lockSoundProcessors() = false;
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
            ampooldelete(MemoryPoolKind::Amplimix, SoundProcessorInstance, _dryProcessor);

        if (_wetProcessor != nullptr)
            ampooldelete(MemoryPoolKind::Amplimix, SoundProcessorInstance, _wetProcessor);
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

        const auto dryOut = SoundChunk::CreateChunk(frames, channels, MemoryPoolKind::Amplimix);
        const auto wetOut = SoundChunk::CreateChunk(frames, channels, MemoryPoolKind::Amplimix);

        std::memcpy(dryOut->buffer, in, bufferSize);
        std::memcpy(wetOut->buffer, in, bufferSize);

        _dryProcessor->Process(reinterpret_cast<AmAudioSampleBuffer>(dryOut->buffer), in, frames, bufferSize, channels, sampleRate, sound);
        _wetProcessor->Process(reinterpret_cast<AmAudioSampleBuffer>(wetOut->buffer), in, frames, bufferSize, channels, sampleRate, sound);

#if defined(AM_SIMD_INTRINSICS)
        const AmSize length = frames / dryOut->samplesPerVector;
        const AmSize end = length * dryOut->samplesPerVector;
        const AmSize remaining = (end - frames) * channels;

        const auto dry = xsimd::batch(_dry);
        const auto wet = xsimd::batch(_wet);

        for (AmSize i = 0; i < length; i++)
        {
            const auto& bd = dryOut->buffer[i];
            const auto& bw = wetOut->buffer[i];

            xsimd::store_aligned(&out[i * AmAudioFrame::size], xsimd::fma(bd, dry, (bw - bd) * wet));
        }

        for (AmSize i = 0; i < remaining; i++)
        {
            const auto& bd = reinterpret_cast<AmAudioSampleBuffer>(dryOut->buffer)[i + end];
            const auto& bw = reinterpret_cast<AmAudioSampleBuffer>(wetOut->buffer)[i + end];

            out[i + end] = bd * _dry + (bw - bd) * _wet;
        }
#else
        const AmSize length = frames * channels;

        for (AmSize i = 0; i < length; i++)
        {
            out[i] = dryOut->buffer[i] * _dry + (wetOut->buffer[i] - dryOut->buffer[i]) * _wet;
        }
#endif // AM_SIMD_INTRINSICS

        SoundChunk::DestroyChunk(dryOut);
        SoundChunk::DestroyChunk(wetOut);
    }
} // namespace SparkyStudios::Audio::Amplitude
