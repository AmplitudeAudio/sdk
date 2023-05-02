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

#pragma once

#ifndef SS_AMPLITUDE_AUDIO_SOUND_PROCESSOR_H
#define SS_AMPLITUDE_AUDIO_SOUND_PROCESSOR_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>

namespace SparkyStudios::Audio::Amplitude
{
    class SoundProcessorInstance
    {
    public:
        SoundProcessorInstance() = default;
        virtual ~SoundProcessorInstance() = default;

        virtual void Process(
            AmAudioSampleBuffer out,
            AmConstAudioSampleBuffer in,
            AmUInt64 frames,
            AmSize bufferSize,
            AmUInt16 channels,
            AmUInt32 sampleRate,
            SoundInstance* sound) = 0;

        virtual void ProcessInterleaved(
            AmAudioSampleBuffer out,
            AmConstAudioSampleBuffer in,
            AmUInt64 frames,
            AmSize bufferSize,
            AmUInt16 channels,
            AmUInt32 sampleRate,
            SoundInstance* sound) = 0;

        virtual AmSize GetOutputBufferSize(AmUInt64 frames, AmSize bufferSize, AmUInt16 channels, AmUInt32 sampleRate);

        /**
         * @brief Cleans up all the memory allocated when the given
         * sound instance was processed.
         *
         * @note This means that the sound instance is stopped and will
         * be removed from the Engine.
         *
         * @param sound The sound instance to clean up.
         */
        virtual void Cleanup(SoundInstance* sound);
    };

    class SoundProcessor
    {
    public:
        /**
         * @brief Create a new SoundProcessor instance.
         *
         * @param name The sound processor name. Recommended names are "xyzProcessor".
         * eg. "EnvironmentProcessor".
         */
        explicit SoundProcessor(std::string name);

        /**
         * @brief Default SoundProcessor constructor.
         *
         * This will not automatically register the sound processor. It's meant for internal sound
         * processors only.
         */
        SoundProcessor();

        virtual ~SoundProcessor() = default;

        /**
         * @brief Creates a new instance of the sound processor.
         * @return A new instance of the sound processor.
         */
        virtual SoundProcessorInstance* CreateInstance() = 0;

        /**
         * @brief Destroys an instance of the sound processor. The instance should have
         * been created with CreateInstance().
         * @param instance The sound processor instance to be destroyed.
         */
        virtual void DestroyInstance(SoundProcessorInstance* instance) = 0;

        /**
         * @brief Gets the name of this sound processor.
         *
         * @return The name of this sound processor.
         */
        [[nodiscard]] const std::string& GetName() const;

        /**
         * @brief Registers a new sound processor.
         *
         * @param processor The sound processor to add in the registry.
         */
        static void Register(SoundProcessor* processor);

        /**
         * @brief Constructs a new sound processor instance.
         *
         * @param name The name of the sound processor to instantiate.
         *
         * @return A new sound processor instance.
         */
        static SoundProcessorInstance* Construct(const std::string& name);

        /**
         * @brief Destructs a sound processor instance.
         *
         * @param name The name of the sound processor which created the instance.
         * @param instance The sound processor instance to destroy.
         */
        static void Destruct(const std::string& name, SoundProcessorInstance* instance);

        /**
         * @brief Locks the codecs registry.
         *
         * This function is mainly used for internal purposes. Its
         * called before the Engine initialization, to discard the
         * registration of new codecs after the engine is fully loaded.
         */
        static void LockRegistry();

    protected:
        /**
         * @brief The name of this processor.
         */
        std::string m_name;

    private:
        /**
         * @brief Look up a sound processor by name.
         *
         * @return The sound processor with the given name, or NULL if none.
         */
        static SoundProcessor* Find(const std::string& name);
    };

    class ProcessorMixer : public SoundProcessorInstance
    {
    public:
        ProcessorMixer();
        ~ProcessorMixer() override;

        void SetWetProcessor(SoundProcessorInstance* processor, AmReal32 wet);

        void SetDryProcessor(SoundProcessorInstance* processor, AmReal32 dry);

        void Process(
            AmAudioSampleBuffer out,
            AmConstAudioSampleBuffer in,
            AmUInt64 frames,
            AmSize bufferSize,
            AmUInt16 channels,
            AmUInt32 sampleRate,
            SoundInstance* sound) override;

        void ProcessInterleaved(
            AmAudioSampleBuffer out,
            AmConstAudioSampleBuffer in,
            AmUInt64 frames,
            AmSize bufferSize,
            AmUInt16 channels,
            AmUInt32 sampleRate,
            SoundInstance* sound) override;

    private:
        SoundProcessorInstance* _wetProcessor;
        SoundProcessorInstance* _dryProcessor;

        AmReal32 _wet;
        AmReal32 _dry;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_SOUND_PROCESSOR_H
