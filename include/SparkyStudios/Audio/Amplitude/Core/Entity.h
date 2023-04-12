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

#ifndef SS_AMPLITUDE_AUDIO_ENTITY_H
#define SS_AMPLITUDE_AUDIO_ENTITY_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    class EntityInternalState;

    /**
     * @brief An Entity represent an object in the game.
     *
     * Amplitude use entities to link sound to an object in the game. Each sounds
     * played from an entity get the location and orientation data fom that entity.
     *
     * The Entity class is a lightweight reference to a EntityInternalState object
     * which is managed by the Engine.
     */
    class Entity
    {
    public:
        /**
         * @brief Creates an uninitialized Entity.
         *
         * An uninitialized Entity cannot provide location and orientation
         * information, and therefore cannot play sounds.
         */
        Entity();

        explicit Entity(EntityInternalState* state);

        /**
         * @brief Uninitializes this Entity.
         *
         * Note that this does not destroy the internal state it references,
         * it just removes this reference to it.
         */
        void Clear();

        /**
         * @brief Checks whether this Entity has been initialized.
         *
         * @return boolean true if this Entity has been initialized.
         */
        [[nodiscard]] bool Valid() const;

        /**
         * @brief Gets the ID of this Entity in game.
         *
         * @return The game Entity ID.
         */
        [[nodiscard]] AmEntityID GetId() const;

        /**
         * @brief Gets the velocity of the Entity.
         *
         * @return The Entity's velocity.
         */
        [[nodiscard]] const AmVec3& GetVelocity() const;

        /**
         * @brief Sets the location of this Entity.
         *
         * @param location The new location.
         */
        void SetLocation(const AmVec3& location) const;

        /**
         * @brief Gets the current location of this Entity.
         *
         * @return The current location of this Entity.
         */
        [[nodiscard]] const AmVec3& GetLocation() const;

        /**
         * @brief Sets the orientation of this Entity.
         *
         * @param direction The direction towards the Entity.
         * @param up The up vector.
         */
        void SetOrientation(const AmVec3& direction, const AmVec3& up) const;

        /**
         * @brief Get the direction vector of the Entity.
         *
         * @return The direction vector.
         */
        [[nodiscard]] const AmVec3& GetDirection() const;

        /**
         * @brief Get the up vector of the Entity.
         *
         * @return The up vector.
         */
        [[nodiscard]] const AmVec3& GetUp() const;

        /**
         * @brief Update the state of this Entity.
         *
         * This method is called automatically by the Engine
         * on each frames.
         */
        void Update() const;

        /**
         * @brief Set the obstruction level of sounds played by this Entity.
         *
         * @param obstruction The obstruction amount. This is provided by the
         * game engine.
         */
        void SetObstruction(AmReal32 obstruction) const;

        /**
         * @brief Set the occlusion level of sounds played by this Entity.
         *
         * @param occlusion The occlusion amount. This is provided by the
         * game engine.
         */
        void SetOcclusion(AmReal32 occlusion) const;

        /**
         * @brief Get the obstruction level of sounds played by this Entity.
         *
         * @return The obstruction amount.
         */
        [[nodiscard]] AmReal32 GetObstruction() const;

        /**
         * @brief Get the occlusion level of sounds played by this Entity.
         *
         * @return The occlusion amount.
         */
        [[nodiscard]] AmReal32 GetOcclusion() const;

        /**
         * @brief Sets the environment factor for this Entity in the given environment.
         *
         * @param environment The environment ID.
         * @param factor The environment factor.
         */
        void SetEnvironmentFactor(AmEnvironmentID environment, AmReal32 factor) const;

        /**
         * @brief Gets the environment factor of this Entity for the given environment.
         *
         * @param environment The environment ID.
         *
         * @return The environment factor.
         */
        [[nodiscard]] AmReal32 GetEnvironmentFactor(AmEnvironmentID environment) const;

        /**
         * @brief Get the list of environments where this Entity belongs or has visited.
         *
         * @return The list of environments where this Entity belongs or has visited.
         */
        [[nodiscard]] const std::map<AmEnvironmentID, AmReal32>& GetEnvironments() const;

        /**
         * @brief Returns the internal state of this Entity.
         *
         * @return The Entity internal state.
         */
        [[nodiscard]] EntityInternalState* GetState() const;

    private:
        EntityInternalState* _state;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_ENTITY_H
