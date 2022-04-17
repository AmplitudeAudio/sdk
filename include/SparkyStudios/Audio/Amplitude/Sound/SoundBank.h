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

#ifndef SPARK_AUDIO_SOUND_BANK_H
#define SPARK_AUDIO_SOUND_BANK_H

#include <SparkyStudios/Audio/Amplitude/Core/RefCounter.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct SoundBankDefinition;

    class Engine;

    /**
     * @brief Amplitude Sound Bank
     *
     * A Sound Bank is a group of sound collections and event, packed in a single binary. This way allows
     * you to pack the needed data for your game as you want (ie. sound banks per levels). A sound bank
     * need to be loaded by the Engine using <code>Engine::LoadSoundBank()</code> before to play sounds and
     * trigger events inside it. When the sound bank data should be released (ie. changing the level, closing
     * the game, etc.), you need to unload the sound bank using <code>Engine::UnloadSoundBank()</code>.
     */
    class SoundBank
    {
    public:
        SoundBank();
        explicit SoundBank(const std::string& source);

        /**
         * @brief Initializes the sound bank by loading all the packed data.
         *
         * @param filename The path to the sound bank file.
         * @param engine The engine in which load the sound bank.
         *
         * @return true when the operation succeeds, false otherwise.
         */
        bool Initialize(const AmOsString& filename, Engine* engine);

        /**
         * @brief Initializes the sound bank by loading all the packed data.
         *
         * @param fileData The sound bank file content.
         * @param engine The engine in which load the sound bank.
         *
         * @return true when the operation succeeds, false otherwise.
         */
        bool InitializeFromMemory(const char* fileData, Engine* engine);

        /**
         * @brief Unloads the sound bank from the Engine.
         *
         * @param engine The engine from which unload the sound bank.
         */
        void Deinitialize(Engine* engine);

        /**
         * @brief Returns the unique ID of this SoundBank.
         *
         * @return The SoundBank unique ID.
         */
        [[nodiscard]] AmBankID GetId() const;

        /**
         * @brief Returns the name of this SoundBank.
         *
         * @return The SoundBank name.
         */
        [[nodiscard]] const std::string& GetName() const;

        /**
         * @brief Returns the definition data used to initialize this SoundBank.
         *
         * @return The sound bank definition data.
         */
        [[nodiscard]] const SoundBankDefinition* GetSoundBankDefinition() const;

        /**
         * @brief Get the references counter of this instance.
         *
         * @return The references counter.
         */
        RefCounter* GetRefCounter();

    private:
        bool InitializeInternal(Engine* engine);

        RefCounter _refCounter;
        std::string _soundBankDefSource;

        std::string _name;
        AmBankID _id;
    };

} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_SOUND_BANK_H
