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

#ifndef SS_AMPLITUDE_AUDIO_ENVIRONMENT_INTERNAL_STATE_H
#define SS_AMPLITUDE_AUDIO_ENVIRONMENT_INTERNAL_STATE_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Math/Shape.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Effect.h>

#include <Utils/intrusive_list.h>

namespace SparkyStudios::Audio::Amplitude
{
    class EnvironmentInternalState
    {
    public:
        EnvironmentInternalState();

        /**
         * @brief Gets the ID of this Environment in game.
         *
         * @return The game Environment ID.
         */
        [[nodiscard]] AmEnvironmentID GetId() const;

        /**
         * @brief Sets the ID of this Environment in game.
         *
         * @param id The game Environment ID.
         */
        void SetId(AmEnvironmentID id);

        /**
         * @brief Sets the location of this Environment.
         *
         * @param location The new location.
         */
        void SetLocation(const AmVec3& location);

        /**
         * @brief Gets the current location of this Environment.
         *
         * @return The current location of this Environment.
         */
        [[nodiscard]] const AmVec3& GetLocation() const;

        /**
         * @brief Sets the orientation of this Environment.
         *
         * @param orientation The new orientation.
         */
        void SetOrientation(const Orientation& orientation);

        /**
         * @brief Gets the orientation of this Environment.
         *
         * @return The current orientation of this Environment.
         */
        [[nodiscard]] const Orientation& GetOrientation();

        /**
         * @brief Get the direction vector of the Environment.
         *
         * @return The direction vector.
         */
        [[nodiscard]] AmVec3 GetDirection() const;

        /**
         * @brief Get the up vector of the Environment.
         *
         * @return The up vector.
         */
        [[nodiscard]] AmVec3 GetUp() const;

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
        void SetEffect(AmEffectID effect);

        /**
         * @brief Set the Effect applied in the Environment.
         *
         * @param effect The effect to apply in the Environment.
         */
        void SetEffect(const std::string& effect);

        /**
         * @brief Set the Effect applied in the Environment.
         *
         * @param effect The effect to apply in the Environment.
         */
        void SetEffect(const Effect* effect);

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
        void SetZone(Zone* zone);

        /**
         * @brief Get the Zone linked to this environment.
         *
         * @return An Zone instance.
         */
        [[nodiscard]] const Zone* GetZone() const;

        /**
         * @brief Get the Zone linked to this environment.
         *
         * @return An Zone instance.
         */
        [[nodiscard]] Zone* GetZone();

        /**
         * @brief Updates the inverse matrix of this Environment.
         *
         * This method is called automatically by the Engine on
         * each frame update.
         */
        void Update();

        fplutil::intrusive_list_node node;

    private:
        AmEnvironmentID _id;

        Zone* _zone;
        const Effect* _effect;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_ENVIRONMENT_INTERNAL_STATE_H