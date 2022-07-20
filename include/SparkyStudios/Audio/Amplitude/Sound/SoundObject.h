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

#ifndef SS_AMPLITUDE_AUDIO_SOUND_OBJECT_H
#define SS_AMPLITUDE_AUDIO_SOUND_OBJECT_H

#include <SparkyStudios/Audio/Amplitude/Core/Bus.h>
#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/RefCounter.h>

#include <SparkyStudios/Audio/Amplitude/IO/FileLoader.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Attenuation.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Effect.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Rtpc.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct EngineInternalState;

    /**
     * @brief The SoundObject class is the base class for all sound objects.
     */
    class SoundObject
    {
    public:
        SoundObject();
        virtual ~SoundObject() = default;
        
        /**
         * @brief Loads the sound object from the given source.
         *
         * @param source The sound object file content to load.
         * @param state The engine state to use while loading the sound object.
         *
         * @return true if the sound object was loaded successfully, false otherwise.
         */
        virtual bool LoadDefinition(const std::string& source, EngineInternalState* state) = 0;

        /**
         * @brief Loads the sound object from the given file path.
         *
         * @param filename The path to the sound object file to load.
         * @param state The engine state to use while loading the sound object.
         *
         * @return true if the sound object was loaded successfully, false otherwise.
         */
        virtual bool LoadDefinitionFromFile(const AmOsString& filename, EngineInternalState* state) = 0;

        /**
         * @brief Acquires referenced objects in this sound object.
         *
         * @param state The engine state used while loading the sound object.
         */
        virtual void AcquireReferences(EngineInternalState* state) = 0;

        /**
         * @brief Releases the references acquired when loading the sound object.
         *
         * @param state The engine state used while loading the sound object.
         */
        virtual void ReleaseReferences(EngineInternalState* state) = 0;

        /**
         * @brief Gets the actual gain of the sound object.
         *
         * @return The sound object gain.
         */
        [[nodiscard]] virtual const RtpcValue& GetGain() const;

        /**
         * @brief Gets the actual priority of the sound object.
         *
         * @return The sound object priority.
         */
        [[nodiscard]] virtual const RtpcValue& GetPriority() const;

        /**
         * @brief Get the unique ID of this sound object.
         *
         * @return The unique sound object ID.
         */
        [[nodiscard]] virtual AmSoundID GetId() const;

        /**
         * @brief Get the name of this sound object.
         *
         * @return The sound object's name.
         */
        [[nodiscard]] virtual const std::string& GetName() const;

        /**
         * @brief Get the Effect object associated with this sound object.
         *
         * @return The Effect object.
         */
        [[nodiscard]] virtual const Effect* GetEffect() const;

        /**
         * @brief Get the Attenuation object associated with this sound object.
         *
         * @return The Attenuation object.
         */
        [[nodiscard]] virtual const Attenuation* GetAttenuation() const;

        /**
         * @brief Return the bus this sound object will play on.
         *
         * @return The bus this sound object will play on.
         */
        [[nodiscard]] virtual Bus GetBus() const;

        /**
         * @brief Get the references counter of this instance.
         *
         * @return The references counter.
         */
        virtual RefCounter* GetRefCounter();

    protected:
        // The bus this sound object will play on.
        BusInternalState* m_bus;

        AmCollectionID m_id;
        std::string m_name;

        RtpcValue m_gain;
        RtpcValue m_priority;

        Effect* m_effect;
        Attenuation* m_attenuation;

        RefCounter m_refCounter;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_SOUND_OBJECT_H
