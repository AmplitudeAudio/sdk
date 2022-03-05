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
    class SoundProcessor
    {
    public:
        /**
         * @brief Create a new SoundProcessor instance.
         *
         * @param name The sound processor name. Recommended names are "xyzProcessor".
         * eg. "EnvironmentProcessor".
         */
        explicit SoundProcessor(std::string  name);

        /**
         * @brief Default SoundProcessor constructor.
         *
         * This will not automatically register the sound processor. It's meant for internal sound
         * processors only.
         */
        SoundProcessor();

        virtual ~SoundProcessor() = default;

        virtual void Process(
            AmInt16Buffer out,
            AmInt16Buffer in,
            AmUInt64 frames,
            AmUInt64 bufferSize,
            AmUInt16 channels,
            AmUInt32 sampleRate,
            SoundInstance* sound) = 0;

        virtual void ProcessInterleaved(
            AmInt16Buffer out,
            AmInt16Buffer in,
            AmUInt64 frames,
            AmUInt64 bufferSize,
            AmUInt16 channels,
            AmUInt32 sampleRate,
            SoundInstance* sound) = 0;

        virtual AmSize GetOutputBufferSize(AmUInt64 frames, AmUInt64 bufferSize, AmUInt16 channels, AmUInt32 sampleRate);

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
         * @brief Look up a sound processor by name.
         *
         * @return The sound processor with the given name, or NULL if none.
         */
        static SoundProcessor* Find(const std::string& name);

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
    };

    class ProcessorMixer : public SoundProcessor
    {
    public:
        ProcessorMixer();

        void SetWetProcessor(SoundProcessor* processor, AmReal32 wet);

        void SetDryProcessor(SoundProcessor* processor, AmReal32 dry);

        void Process(
            AmInt16Buffer out,
            AmInt16Buffer in,
            AmUInt64 frames,
            AmUInt64 bufferSize,
            AmUInt16 channels,
            AmUInt32 sampleRate,
            SoundInstance* sound) override;

        void ProcessInterleaved(
            AmInt16Buffer out,
            AmInt16Buffer in,
            AmUInt64 frames,
            AmUInt64 bufferSize,
            AmUInt16 channels,
            AmUInt32 sampleRate,
            SoundInstance* sound) override;

    private:
        SoundProcessor* _wetProcessor;
        SoundProcessor* _dryProcessor;

        AmReal32 _wet;
        AmReal32 _dry;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_SOUND_PROCESSOR_H
