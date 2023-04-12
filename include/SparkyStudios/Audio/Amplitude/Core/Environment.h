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

#ifndef SS_AMPLITUDE_AUDIO_ENVIRONMENT_H
#define SS_AMPLITUDE_AUDIO_ENVIRONMENT_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>
#include <SparkyStudios/Audio/Amplitude/Core/RefCounter.h>
#include <SparkyStudios/Audio/Amplitude/Math/Shape.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Effect.h>

namespace SparkyStudios::Audio::Amplitude
{
    class EnvironmentInternalState;

    /**
     * @brief An Environment is a zone where every spatialized audio playing inside him got
     * applied some effects, like reverb, echo, or gain.
     *
     * The Environment class is a lightweight reference to a EnvironmentInternalState object
     * which is managed by the Engine.
     */
    class Environment
    {
    public:
        /**
         * @brief Creates an uninitialized Environment.
         *
         * An uninitialized Environment cannot provide location and orientation
         * information, and therefore cannot play sounds.
         */
        Environment();

        explicit Environment(EnvironmentInternalState* state);

        /**
         * @brief Uninitializes this Environment.
         *
         * Note that this does not destroy the internal state it references,
         * it just removes this reference to it.
         */
        void Clear();

        /**
         * @brief Checks whether this Environment has been initialized.
         *
         * @return boolean true if this Environment has been initialized.
         */
        [[nodiscard]] bool Valid() const;

        /**
         * @brief Returns the unique ID of this Environment.
         *
         * @return The Environment unique ID.
         */
        [[nodiscard]] AmEnvironmentID GetId() const;

        /**
         * @brief Sets the location of this Environment.
         *
         * @param location The new location.
         */
        void SetLocation(const AmVec3& location) const;

        /**
         * @brief Gets the current location of this Environment.
         *
         * @return The current location of this Environment.
         */
        [[nodiscard]] const AmVec3& GetLocation() const;

        /**
         * @brief Sets the orientation of this Environment.
         *
         * @param direction The direction towards the Environment.
         * @param up The up vector.
         */
        void SetOrientation(const AmVec3& direction, const AmVec3& up) const;

        /**
         * @brief Get the direction vector of the Environment.
         *
         * @return The direction vector.
         */
        [[nodiscard]] const AmVec3& GetDirection() const;

        /**
         * @brief Get the up vector of the Environment.
         *
         * @return The up vector.
         */
        [[nodiscard]] const AmVec3& GetUp() const;

        /**
         * @brief Get the Environment factor for the given location.
         *
         * @param location The location for which compute the environment factor.
         *
         * @return The environment factor.
         */
        [[nodiscard]] AmReal32 GetFactor(const AmVec3& location) const;

        /**
         * @brief Get the Environment factor for the given entity.
         *
         * @param entity The entity for which compute the environment factor.
         *
         * @return The environment factor.
         */
        [[nodiscard]] AmReal32 GetFactor(const Entity& entity) const;

        /**
         * @brief Set the Effect applied in the Environment.
         *
         * @param effect The effect to apply in the Environment.
         */
        void SetEffect(AmEffectID effect) const;

        /**
         * @brief Set the Effect applied in the Environment.
         *
         * @param effect The effect to apply in the Environment.
         */
        void SetEffect(const std::string& effect) const;

        /**
         * @brief Set the Effect applied in the Environment.
         *
         * @param effect The effect to apply in the Environment.
         */
        void SetEffect(const Effect* effect) const;

        /**
         * @brief Get the Effect linked to this environment.
         *
         * @return An Effect instance.
         */
        [[nodiscard]] const Effect* GetEffect() const;

        /**
         * @brief Set the Zone for this environment.
         *
         * @param zone The environment's zone.
         */
        void SetZone(Zone* zone) const;

        /**
         * @brief Get the Zone linked to this environment.
         *
         * @return An Zone instance.
         */
        [[nodiscard]] Zone* GetZone() const;

        /**
         * @brief Returns the internal state of this Environment.
         *
         * @return The Environment internal state.
         */
        [[nodiscard]] EnvironmentInternalState* GetState() const;

        /**
         * @brief Update the state of this Environment.
         *
         * This method is called automatically by the Engine
         * on each frames.
         */
        void Update() const;

    private:
        EnvironmentInternalState* _state;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_ENVIRONMENT_H
