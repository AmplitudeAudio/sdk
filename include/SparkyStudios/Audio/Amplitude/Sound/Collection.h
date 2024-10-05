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

#ifndef _AM_SOUND_COLLECTION_H
#define _AM_SOUND_COLLECTION_H

#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Amplitude Collection Asset.
     *
     * A `Collection` is a container sound object that group multiple sounds over the same name. Only
     * one sound can be playing at a time in the same collection, and the sound picked for playback
     * is chosen by the collection's `Scheduler`.
     *
     * @see [SoundObject](../../engine/SoundObject/index.md)
     *
     * @ingroup assets
     */
    class AM_API_PUBLIC Collection
        : public SoundObject
        , public Asset<AmCollectionID>
    {
    public:
        /**
         * @brief Returns a Sound from this collection from the World scope.
         *
         * @param[in] toSkip The list of Sound IDs to skip fom the selection.
         *
         * @return The selected Sound.
         */
        [[nodiscard]] virtual Sound* SelectFromWorld(const std::vector<AmSoundID>& toSkip) const = 0;

        /**
         * @brief Returns a Sound from this collection from an Entity scope.
         *
         * @param[in] entity The entity from which pick the sound.
         * @param[in] toSkip The list of Sound IDs to skip fom the selection.
         *
         * @return The selected Sound.
         */
        virtual Sound* SelectFromEntity(const Entity& entity, const std::vector<AmSoundID>& toSkip) = 0;

        /**
         * @brief Resets the internal state of the scheduler running for the given Entity.
         *
         * @param[in] entity The entity for which reset the scheduler state.
         */
        virtual void ResetEntityScopeScheduler(const Entity& entity) = 0;

        /**
         * @brief Resets the internal state of the scheduler running for the World.
         */
        virtual void ResetWorldScopeScheduler() = 0;

        /**
         * @brief Returns the list of Sound objects referenced in this collection.
         *
         * @return The list of Sound IDs.
         */
        [[nodiscard]] virtual const std::vector<AmSoundID>& GetSounds() const = 0;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_SOUND_COLLECTION_H
