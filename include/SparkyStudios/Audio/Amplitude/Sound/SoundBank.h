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

#ifndef _AM_SOUND_SOUND_BANK_H
#define _AM_SOUND_SOUND_BANK_H

#include <queue>

#include <SparkyStudios/Audio/Amplitude/Core/RefCounter.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct SoundBankDefinition;

    class Engine;

    /**
     * @brief Amplitude sound bank asset.
     *
     * A Sound Bank is a group of Amplitude assets, registered in a single binary. This way allows
     * you to pack the necessary data for your game as you want (e.g., sound banks per levels). A sound bank
     * needs to be loaded by the Engine using Engine::LoadSoundBank() before to play sounds and
     * trigger events inside it. When the sound bank data should be released (e.g., changing the level, closing
     * the game, etc.), you need to unload the sound bank using Engine::UnloadSoundBank().
     *
     * @ingroup assets
     */
    class AM_API_PUBLIC SoundBank
    {
    public:
        /**
         * @brief Creates an uninitialized sound bank.
         */
        SoundBank();

        /**
         * @brief Creates a sound bank from the given source file.
         *
         * @warning This constructor is for internal usage only.
         */
        explicit SoundBank(const AmString& source);

        /**
         * @brief Initializes the sound bank by loading all the packed data.
         *
         * @param[in] filename The path to the sound bank file.
         * @param[in] engine The engine instance in which load the sound bank.
         *
         * @return @c true when the operation succeeds, @c false otherwise.
         */
        bool Initialize(const AmOsString& filename, Engine* engine);

        /**
         * @brief Initializes the sound bank by loading all the packed data.
         *
         * @param[in] fileData The sound bank file content.
         * @param[in] engine The engine instance in which load the sound bank.
         *
         * @return @c true when the operation succeeds, @c false otherwise.
         */
        bool InitializeFromMemory(const AmUInt8* fileData, Engine* engine);

        /**
         * @brief Unloads the sound bank from the Engine.
         *
         * @param[in] engine The engine instance from which unload the sound bank.
         */
        void Deinitialize(Engine* engine);

        /**
         * @brief Returns the unique ID of this sound bank.
         *
         * @return The sound bank's unique ID.
         */
        [[nodiscard]] AmBankID GetId() const;

        /**
         * @brief Returns the name of this sound bank.
         *
         * @return The sound bank's name.
         */
        [[nodiscard]] const AmString& GetName() const;

        /**
         * @brief Returns the definition data used to initialize this sound bank.
         *
         * @warning This method is for internal usage only.
         *
         * @return The sound bank's definition data.
         */
        [[nodiscard]] const SoundBankDefinition* GetSoundBankDefinition() const;

        /**
         * @brief Gets the reference counter of this instance.
         *
         * @return The references counter.
         */
        RefCounter* GetRefCounter();

        /**
         * @brief Load the sound files referenced in the sound bank.
         *
         * @param[in] engine The engine instance from which the sound files will be loaded.
         *
         * @warning This method should not be called directly. It is called automatically by the @c Engine with
         * the Engine::StartLoadSoundFiles() method.
         */
        void LoadSoundFiles(const Engine* engine);

    private:
        bool InitializeInternal(Engine* engine);

        RefCounter _refCounter;
        AmString _soundBankDefSource;

        AmString _name;
        AmBankID _id;

        std::queue<AmSoundID> _pendingSoundsToLoad;
    };

} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_SOUND_SOUND_BANK_H
