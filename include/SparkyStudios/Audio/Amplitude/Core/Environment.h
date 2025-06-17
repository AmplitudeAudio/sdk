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

#ifndef _AM_CORE_ENVIRONMENT_H
#define _AM_CORE_ENVIRONMENT_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>
#include <SparkyStudios/Audio/Amplitude/Math/Shape.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Effect.h>

namespace SparkyStudios::Audio::Amplitude
{
    class EnvironmentInternalState;

    /**
     * @brief An Environment is a zone where every spatialized audio playing inside him got
     * applied a specific effect.
     *
     * The @c Environment class is a lightweight reference to an @c EnvironmentInternalState object
     * which is managed internally by the @c Engine.
     *
     * @ingroup engine
     */
    class AM_API_PUBLIC Environment
    {
    public:
        /**
         * @brief Creates an uninitialized environment.
         *
         * An uninitialized environment cannot provide location and orientation
         * information, and therefore cannot play sounds.
         *
         * To create an initialized environment, use the @ref Engine::AddEnvironment "`AddEnvironment()`" method of the
         * @c Engine instance.
         *
         * @code{cpp}
         * amEngine->AddEnvironment(1234); // You should provide a unique ID
         * @endcode
         */
        Environment();

        /**
         * @brief Creates a wrapper instance over the provided state.
         *
         * @param[in] state The internal state to wrap.
         *
         * @warning This constructor is for internal usage only.
         */
        explicit Environment(EnvironmentInternalState* state);

        /**
         * @brief Uninitializes this environment.
         *
         * @note This does not destroy the internal state it references, it just removes this reference to it.
         *
         * To completely destroy the environment, use the @ref Engine::RemoveEnvironment "`RemoveEnvironment()`" method
         * of the @c Engine instance.
         *
         * @code{cpp}
         * amEngine->RemoveEnvironment(1234); // You should provide the environment ID
         * @endcode
         */
        void Clear();

        /**
         * @brief Checks whether this environment has been initialized.
         *
         * @return @c true if this environment is initialized, @c false otherwise.
         */
        [[nodiscard]] bool Valid() const;

        /**
         * @brief Returns the unique ID of this environment.
         *
         * @return The environment's unique ID.
         */
        [[nodiscard]] AmEnvironmentID GetId() const;

        /**
         * @brief Sets the location of this environment.
         *
         * @param[in] location The new location.
         */
        void SetLocation(const AmVec3& location) const;

        /**
         * @brief Gets the current location of this environment.
         *
         * @return The current location of this environment.
         */
        [[nodiscard]] const AmVec3& GetLocation() const;

        /**
         * @brief Sets the orientation of this environment.
         *
         * @param[in] orientation The new orientation.
         */
        void SetOrientation(const Orientation& orientation) const;

        /**
         * @brief Gets the current orientation of this environment.
         *
         * @return The current orientation of this environment.
         */
        [[nodiscard]] const Orientation& GetOrientation() const;

        /**
         * @brief Gets the direction vector of the environment.
         *
         * @return The direction vector.
         */
        [[nodiscard]] AmVec3 GetDirection() const;

        /**
         * @brief Gets the up vector of the environment.
         *
         * @return The up vector.
         */
        [[nodiscard]] AmVec3 GetUp() const;

        /**
         * @brief Gets the environment factor for the given location.
         *
         * @param[in] location The location for which compute the environment factor.
         *
         * @return The environment factor.
         */
        [[nodiscard]] AmReal32 GetFactor(const AmVec3& location) const;

        /**
         * @brief Gets the environment factor for the given entity.
         *
         * @param[in] entity The entity for which compute the environment factor.
         *
         * @return The environment factor.
         */
        [[nodiscard]] AmReal32 GetFactor(const Entity& entity) const;

        /**
         * @brief Sets the effect applied in the environment.
         *
         * @param[in] effect The ID of the effect to apply in the environment.
         *
         * @see Effect
         */
        void SetEffect(AmEffectID effect) const;

        /**
         * @brief Sets the effect applied in the environment.
         *
         * @param[in] effect The name of the effect to apply in the environment.
         *
         * @see Effect
         */
        void SetEffect(const AmString& effect) const;

        /**
         * @brief Sets the effect applied in the environment.
         *
         * @param[in] effect The effect to apply in the environment.
         *
         * @see Effect
         */
        void SetEffect(const Effect* effect) const;

        /**
         * @brief Gets the effect linked to this environment.
         *
         * @return An Effect instance.
         *
         * @see Effect
         */
        [[nodiscard]] const Effect* GetEffect() const;

        /**
         * @brief Sets the zone for this environment.
         *
         * @param[in] zone The environment's zone.
         *
         * @see Zone
         */
        void SetZone(std::shared_ptr<Zone> zone) const;

        /**
         * @brief Gets the zone linked to this environment.
         *
         * @return An zone instance.
         *
         * @see Zone
         */
        [[nodiscard]] std::shared_ptr<Zone> GetZone() const;

        /**
         * @brief Returns the internal state of this environment.
         *
         * @return The environment internal state.
         *
         * @warning This method is for internal usage only.
         */
        [[nodiscard]] EnvironmentInternalState* GetState() const;

        /**
         * @brief Updates the state of this environment.
         *
         * The @c Engine calls this method automatically on each frame to update the internal state of the environment.
         *
         * @warning This method is for internal usage only.
         */
        void Update() const;

    private:
        /**
         * @brief The internal state of the entity.
         *
         * @internal
         */
        EnvironmentInternalState* _state;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_ENVIRONMENT_H
