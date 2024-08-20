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

#ifndef _AM_MIXER_SOUND_PROCESSOR_H
#define _AM_MIXER_SOUND_PROCESSOR_H

#include <SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h>
#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    class AM_API_PUBLIC SoundProcessorInstance
    {
    public:
        SoundProcessorInstance() = default;
        virtual ~SoundProcessorInstance() = default;

        virtual void Process(const AmplimixLayer* layer, const AudioBuffer& in, AudioBuffer& out) = 0;

        virtual AmSize GetOutputBufferSize(AmUInt64 frames, AmSize bufferSize, AmUInt16 channels, AmUInt32 sampleRate);

        /**
         * @brief Cleans up all the memory allocated when the given
         * sound instance was processed.
         *
         * @note This means that the sound instance is stopped and will
         * be removed from the Engine.
         *
         * @param layer The sound instance to clean up.
         */
        virtual void Cleanup(const AmplimixLayer* layer);
    };

    class AM_API_PUBLIC SoundProcessor
    {
    public:
        /**
         * @brief Create a new SoundProcessor instance.
         *
         * @param name The sound processor name. Recommended names are "xyzProcessor".
         * eg. "EnvironmentProcessor".
         */
        explicit SoundProcessor(AmString name);

        /**
         * @brief Default SoundProcessor constructor.
         *
         * This will not automatically register the sound processor. It's meant for internal sound
         * processors only.
         */
        SoundProcessor();

        virtual ~SoundProcessor();

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
        [[nodiscard]] const AmString& GetName() const;

        /**
         * @brief Registers a new sound processor.
         *
         * @param processor The sound processor to add in the registry.
         */
        static void Register(SoundProcessor* processor);

        /**
         * @brief  Unregisters a sound processor.
         *
         * @param processor The sound processor to unregister.
         */
        static void Unregister(const SoundProcessor* processor);

        /**
         * @brief Constructs a new sound processor instance.
         *
         * @param name The name of the sound processor to instantiate.
         *
         * @return A new sound processor instance.
         */
        static SoundProcessorInstance* Construct(const AmString& name);

        /**
         * @brief Destructs a sound processor instance.
         *
         * @param name The name of the sound processor which created the instance.
         * @param instance The sound processor instance to destroy.
         */
        static void Destruct(const AmString& name, SoundProcessorInstance* instance);

        /**
         * @brief Locks the sound processors registry.
         *
         * This function is mainly used for internal purposes. Its
         * called before the Engine initialization, to discard the
         * registration of new sound processors after the engine is fully loaded.
         */
        static void LockRegistry();

        /**
         * @brief Unlocks the sound processors registry.
         *
         * This function is mainly used for internal purposes. Its
         * called after the Engine deinitialization, to allow the
         * registration of new sound processors after the engine is fully unloaded.
         */
        static void UnlockRegistry();

    protected:
        /**
         * @brief The name of this processor.
         */
        AmString m_name;

    private:
        /**
         * @brief Look up a sound processor by name.
         *
         * @return The sound processor with the given name, or NULL if none.
         */
        static SoundProcessor* Find(const AmString& name);
    };

    class AM_API_PUBLIC ProcessorMixer : public SoundProcessorInstance
    {
    public:
        ProcessorMixer();
        ~ProcessorMixer() override;

        void SetWetProcessor(SoundProcessorInstance* processor, AmReal32 wet);

        void SetDryProcessor(SoundProcessorInstance* processor, AmReal32 dry);

        void Process(const AmplimixLayer* layer, const AudioBuffer& in, AudioBuffer& out) override;

    private:
        SoundProcessorInstance* _wetProcessor;
        SoundProcessorInstance* _dryProcessor;

        AmReal32 _wet;
        AmReal32 _dry;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_MIXER_SOUND_PROCESSOR_H
