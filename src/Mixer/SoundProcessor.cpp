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
#include <SparkyStudios/Audio/Amplitude/Mixer/Amplimix.h>
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

    void SoundProcessorInstance::Cleanup(const AmplimixLayer* layer)
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
            amLogWarning("Failed to register sound processor '%s' as it is already registered", processor->GetName().c_str());
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

    void ProcessorMixer::Process(const AmplimixLayer* layer, const AudioBuffer& in, AudioBuffer& out)
    {
        if (_dryProcessor == nullptr || _wetProcessor == nullptr)
        {
            if (&out != &in)
                AudioBuffer::Copy(in, 0, out, 0, out.GetFrameCount());

            return;
        }

        const AmSize frames = out.GetFrameCount();
        const AmSize channels = out.GetChannelCount();

        const auto dryOut = SoundChunk::CreateChunk(frames, channels, MemoryPoolKind::Amplimix);
        const auto wetOut = SoundChunk::CreateChunk(frames, channels, MemoryPoolKind::Amplimix);

        AudioBuffer::Copy(in, 0, *dryOut->buffer, 0, frames);
        AudioBuffer::Copy(in, 0, *wetOut->buffer, 0, frames);

        _dryProcessor->Process(layer, in, *dryOut->buffer);
        _wetProcessor->Process(layer, in, *wetOut->buffer);

        AmSize remaining = frames;

#if defined(AM_SIMD_INTRINSICS)
        const AmSize end = GetNumSimdChunks(frames);
        constexpr AmSize blockSize = GetSimdBlockSize();

        remaining = remaining - end;

        const auto dry = xsimd::batch(_dry);
        const auto wet = xsimd::batch(_wet);
#endif // AM_SIMD_INTRINSICS

        for (AmSize c = 0; c < channels; c++)
        {
            const auto& dryChannel = (*dryOut->buffer)[c];
            const auto& wetChannel = (*wetOut->buffer)[c];
            auto& outChannel = out[c];

#if defined(AM_SIMD_INTRINSICS)
            for (AmSize i = 0; i < end; i += blockSize)
            {
                const auto& bd = xsimd::load_aligned(&dryChannel[i]);
                const auto& bw = xsimd::load_aligned(&wetChannel[i]);

                xsimd::store_aligned(&outChannel[i], xsimd::fma(bd, dry, (bw - bd) * wet));
            }
#endif // AM_SIMD_INTRINSICS

            for (AmSize i = frames - remaining; i < frames; i++)
            {
                outChannel[i] = dryChannel[i] * _dry + (wetChannel[i] - dryChannel[i]) * _wet;
            }
        }

        SoundChunk::DestroyChunk(dryOut);
        SoundChunk::DestroyChunk(wetOut);
    }
} // namespace SparkyStudios::Audio::Amplitude
