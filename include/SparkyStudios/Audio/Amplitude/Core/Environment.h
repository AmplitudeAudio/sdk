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
     * The `Environment` class is a lightweight reference to an `EnvironmentInternalState` object
     * which is managed by the `Engine`.
     *
     * @ingroup engine
     */
    class AM_API_PUBLIC Environment
    {
    public:
        /**
         * @brief Creates an uninitialized `Environment`.
         *
         * An uninitialized Environment cannot provide location and orientation
         * information, and therefore cannot play sounds.
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
         * @brief Uninitializes this Environment.
         *
         * Note that this does not destroy the internal state it references,
         * it just removes this reference to it.
         */
        void Clear();

        /**
         * @brief Checks whether this `Environment` has been initialized.
         *
         * @return `true` if this `Environment` is initialized, `false` otherwise.
         */
        [[nodiscard]] bool Valid() const;

        /**
         * @brief Returns the unique ID of this `Environment`.
         *
         * @return The `Environment` unique ID.
         */
        [[nodiscard]] AmEnvironmentID GetId() const;

        /**
         * @brief Sets the location of this `Environment`.
         *
         * @param[in] location The new location.
         */
        void SetLocation(const AmVec3& location) const;

        /**
         * @brief Gets the current location of this `Environment`.
         *
         * @return The current location of this `Environment`.
         */
        [[nodiscard]] const AmVec3& GetLocation() const;

        /**
         * @brief Sets the orientation of this `Environment`.
         *
         * @param[in] orientation The new orientation.
         */
        void SetOrientation(const Orientation& orientation) const;

        /**
         * @brief Gets the current orientation of this `Environment`.
         *
         * @return The current orientation of this `Environment`.
         */
        [[nodiscard]] const Orientation& GetOrientation() const;

        /**
         * @brief Gets the direction vector of the `Environment`.
         *
         * @return The direction vector.
         */
        [[nodiscard]] AmVec3 GetDirection() const;

        /**
         * @brief Gets the up vector of the `Environment`.
         *
         * @return The up vector.
         */
        [[nodiscard]] AmVec3 GetUp() const;

        /**
         * @brief Gets the `Environment` factor for the given location.
         *
         * @param[in] location The location for which compute the environment factor.
         *
         * @return The environment factor.
         */
        [[nodiscard]] AmReal32 GetFactor(const AmVec3& location) const;

        /**
         * @brief Gets the `Environment` factor for the given entity.
         *
         * @param[in] entity The entity for which compute the environment factor.
         *
         * @return The environment factor.
         */
        [[nodiscard]] AmReal32 GetFactor(const Entity& entity) const;

        /**
         * @brief Sets the `Effect` applied in the `Environment`.
         *
         * @param[in] effect The ID of the effect to apply in the `Environment`.
         */
        void SetEffect(AmEffectID effect) const;

        /**
         * @brief Sets the `Effect` applied in the `Environment`.
         *
         * @param[in] effect The name of the effect to apply in the `Environment`.
         */
        void SetEffect(const AmString& effect) const;

        /**
         * @brief Sets the `Effect` applied in the `Environment`.
         *
         * @param[in] effect The effect to apply in the `Environment`.
         */
        void SetEffect(const Effect* effect) const;

        /**
         * @brief Gets the `Effect` linked to this environment.
         *
         * @return An `Effect` instance.
         */
        [[nodiscard]] const Effect* GetEffect() const;

        /**
         * @brief Sets the `Zone` for this environment.
         *
         * @param[in] zone The environment's zone.
         */
        void SetZone(Zone* zone) const;

        /**
         * @brief Gets the `Zone` linked to this environment.
         *
         * @return An `Zone` instance.
         */
        [[nodiscard]] Zone* GetZone() const;

        /**
         * @brief Returns the internal state of this `Environment`.
         *
         * @return The `Environment` internal state.
         *
         * @warning This method is for internal usage only.
         */
        [[nodiscard]] EnvironmentInternalState* GetState() const;

        /**
         * @brief Updates the state of this `Environment`.
         *
         * This method is called automatically by the `Engine`
         * on each frames to update the internal state of the `Environment`.
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
