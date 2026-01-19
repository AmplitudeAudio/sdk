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

#ifndef _AM_CORE_ENTITY_H
#define _AM_CORE_ENTITY_H

#include <SparkyStudios/Audio/Amplitude/Math/Orientation.h>

#include <unordered_map>

namespace SparkyStudios::Audio::Amplitude
{
    class EntityInternalState;

    /**
     * @brief An Entity represents a spatially positioned object in the game.
     *
     * Amplitude uses entities to link sound to an object in the game. Each sound
     * played from an entity gets the location and orientation data from that entity.
     *
     * The @c Entity class is a lightweight reference to an @c EntityInternalState object
     * which is managed internally by the Engine.
     *
     * @ingroup engine
     */
    class AM_API_PUBLIC Entity
    {
    public:
        /**
         * @brief Creates an uninitialized entity.
         *
         * An uninitialized entity cannot provide location and orientation
         * information, and therefore cannot play sounds.
         *
         * To create an initialized entity, use the @ref Engine::AddEntity "`AddEntity()`" method of the
         * @c Engine instance.
         *
         * @code{cpp}
         * amEngine->AddEntity(1234); // You should provide a unique ID
         * @endcode
         */
        Entity();

        /**
         * @brief Creates a wrapper instance over the provided state.
         *
         * @param[in] state The internal state to wrap.
         *
         * @warning This constructor is for internal usage only.
         */
        explicit Entity(EntityInternalState* state);

        /**
         * @brief Uninitializes this entity.
         *
         * @note This does not destroy the internal state it references, it just removes this reference to it.
         *
         * To completely destroy the entity, use the @ref Engine::RemoveEntity "`RemoveEntity()`" method
         * of the @c Engine instance.
         *
         * @code{cpp}
         * amEngine->RemoveEntity(1234); // You should provide the entity ID
         * @endcode
         */
        void Clear();

        /**
         * @brief Checks whether this entity has been initialized.
         *
         * @return @c true if this entity is initialized, @c false otherwise.
         */
        [[nodiscard]] bool Valid() const;

        /**
         * @brief Gets the ID of this entity.
         *
         * @return The entity ID.
         */
        [[nodiscard]] AmEntityID GetId() const;

        /**
         * @brief Gets the velocity of the entity.
         *
         * @return The entity's velocity.
         */
        [[nodiscard]] const AmVector3& GetVelocity() const;

        /**
         * @brief Sets the location of this entity.
         *
         * @param[in] location The new location.
         */
        void SetLocation(const AmVector3& location) const;

        /**
         * @brief Gets the current location of this entity.
         *
         * @return The current location of this entity.
         */
        [[nodiscard]] const AmVector3& GetLocation() const;

        /**
         * @brief Sets the orientation of this entity.
         *
         * @param[in] orientation The new orientation.
         */
        void SetOrientation(const Orientation& orientation) const;

        /**
         * @brief Gets the direction vector of the entity.
         *
         * @return The direction vector.
         */
        [[nodiscard]] AmVector3 GetDirection() const;

        /**
         * @brief Gets the up vector of the entity.
         *
         * @return The up vector.
         */
        [[nodiscard]] AmVector3 GetUp() const;

        /**
         * @brief Gets the orientation of the entity.
         *
         * @return The entity's orientation.
         */
        [[nodiscard]] const Orientation& GetOrientation() const;

        /**
         * @brief Updates the state of this entity.
         *
         * The @c Engine calls this method automatically on each frame to update the internal state of the entity.
         *
         * @warning This method is for internal usage only.
         */
        void Update() const;

        /**
         * @brief Sets the obstruction level of sounds played by this entity.
         *
         * @param[in] obstruction The obstruction amount.
         */
        void SetObstruction(AmReal32 obstruction) const;

        /**
         * @brief Sets the occlusion level of sounds played by this entity.
         *
         * @param[in] occlusion The occlusion amount.
         */
        void SetOcclusion(AmReal32 occlusion) const;

        /**
         * @brief Sets the directivity and sharpness of sounds played by this entity.
         *
         * @param[in] directivity The directivity of the sound source, in the range [0, 1].
         * @param[in] sharpness The directivity sharpness of the sound source, in the range [1, +INF].
         * Increasing this value increases the directivity towards the front of the source.
         */
        void SetDirectivity(AmReal32 directivity, AmReal32 sharpness) const;

        /**
         * @brief Gets the obstruction level of sounds played by this entity.
         *
         * @return The obstruction amount.
         */
        [[nodiscard]] AmReal32 GetObstruction() const;

        /**
         * @brief Gets the occlusion level of sounds played by this entity.
         *
         * @return The occlusion amount.
         */
        [[nodiscard]] AmReal32 GetOcclusion() const;

        /**
         * @brief Gets the directivity of sounds played by this entity.
         *
         * @return The directivity of sound sources.
         */
        [[nodiscard]] AmReal32 GetDirectivity() const;

        /**
         * @brief Gets the directivity sharpness of sounds played by this entity.
         *
         * @return The directivity sharpness of sounds played by this entity.
         */
        [[nodiscard]] AmReal32 GetDirectivitySharpness() const;

        /**
         * @brief Sets the environment factor for this entity in the given environment.
         *
         * @param[in] environment The environment ID.
         * @param[in] factor The environment factor.
         */
        void SetEnvironmentFactor(AmEnvironmentID environment, AmReal32 factor) const;

        /**
         * @brief Gets the environment factor of this entity for the given environment.
         *
         * @param[in] environment The environment ID.
         *
         * @return The environment factor.
         */
        [[nodiscard]] AmReal32 GetEnvironmentFactor(AmEnvironmentID environment) const;

        /**
         * @brief Gets the list of environments where this entity belongs or has visited.
         *
         * @return The list of environments where this entity belongs or has visited.
         */
        [[nodiscard]] const std::unordered_map<AmEnvironmentID, AmReal32>& GetEnvironments() const;

        /**
         * @brief Gets the number of active channels for this entity.
         *
         * @return The number of active channels.
         */
        [[nodiscard]] AmUInt64 GetActiveChannelCount() const;

        /**
         * @brief Returns the internal state of this entity.
         *
         * @return The entity internal state.
         *
         * @warning This method is for internal usage only.
         */
        [[nodiscard]] EntityInternalState* GetState() const;

    private:
        /**
         * @brief The internal state of the entity.
         *
         * @internal
         */
        EntityInternalState* _state;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_ENTITY_H
