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

#ifndef _AM_IMPLEMENTATION_CORE_ENTITY_INTERNAL_STATE_H
#define _AM_IMPLEMENTATION_CORE_ENTITY_INTERNAL_STATE_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <Core/Playback/ChannelInternalState.h>
#include <Utils/intrusive_list.h>

namespace SparkyStudios::Audio::Amplitude
{
    class EntityInternalState
    {
    public:
        EntityInternalState();

        /**
         * @brief Gets the ID of this Entity in game.
         *
         * @return The game Entity ID.
         */
        [[nodiscard]] AM_INLINE AmEntityID GetId() const
        {
            return _id;
        }

        /**
         * @brief Sets the ID of this Entity in game.
         *
         * @param id The game Entity ID.
         */
        AM_INLINE void SetId(AmEntityID id)
        {
            _id = id;
        }

        /**
         * @brief Gets the velocity of the Entity.
         *
         * @return The Entity's velocity.
         */
        [[nodiscard]] AM_INLINE const AmVec3& GetVelocity() const
        {
            return _velocity;
        }

        /**
         * @brief Sets the location of this Entity.
         *
         * @param location The new location.
         */
        void SetLocation(const AmVec3& location);

        /**
         * @brief Gets the current location of this Entity.
         *
         * @return The current location of this Entity.
         */
        [[nodiscard]] AM_INLINE const AmVec3& GetLocation() const
        {
            return _location;
        }

        /**
         * @brief Sets the orientation of this Entity.
         *
         * @param orientation The new orientation.
         */
        AM_INLINE void SetOrientation(const Orientation& orientation)
        {
            _orientation = orientation;
        }

        /**
         * @brief Get the direction vector of the Entity.
         *
         * @return The direction vector.
         */
        [[nodiscard]] AM_INLINE AmVec3 GetDirection() const
        {
            return _orientation.GetForward();
        }

        /**
         * @brief Get the up vector of the Entity.
         *
         * @return The up vector.
         */
        [[nodiscard]] AM_INLINE AmVec3 GetUp() const
        {
            return _orientation.GetUp();
        }

        /**
         * @brief Get the right vector of the Entity.
         *
         * @return The orientation of this Entity.
         */
        [[nodiscard]] AM_INLINE const Orientation& GetOrientation() const
        {
            return _orientation;
        }

        /**
         * @brief Returns the inverse transformation matrix of this Entity.
         *
         * @return The inverse transformation matrix.
         */
        [[nodiscard]] AM_INLINE const AmMat4& GetInverseMatrix() const
        {
            return _inverseMatrix;
        }

        /**
         * @brief Sets the directivity of sounds played by this Entity.
         *
         * @param directivity The directivity of the sound.
         * @param directivitySharpness The sharpness of the directivity.
         */
        void SetDirectivity(AmReal32 directivity, AmReal32 directivitySharpness);

        /**
         * @brief Get the directivity of sounds played by this Entity.
         *
         * @return The directivity.
         */
        [[nodiscard]] AM_INLINE AmReal32 GetDirectivity() const
        {
            return _directivity;
        }

        /**
         * @brief Get the sharpness of the directivity of sounds played by this Entity.
         *
         * @return The directivity sharpness.
         */
        [[nodiscard]] AM_INLINE AmReal32 GetDirectivitySharpness() const
        {
            return _directivitySharpness;
        }

        /**
         * @brief Set the obstruction level of sounds played by this Entity.
         *
         * @param obstruction The obstruction amount. This is provided by the
         * game engine.
         */
        void SetObstruction(AmReal32 obstruction);

        /**
         * @brief Set the occlusion level of sounds played by this Entity.
         *
         * @param occlusion The occlusion amount. This is provided by the
         * game engine.
         */
        void SetOcclusion(AmReal32 occlusion);

        /**
         * @brief Get the obstruction level of sounds played by this Entity.
         *
         * @return The obstruction amount.
         */
        [[nodiscard]] AM_INLINE AmReal32 GetObstruction() const
        {
            return _obstruction;
        }

        /**
         * @brief Get the occlusion level of sounds played by this Entity.
         *
         * @return The occlusion amount.
         */
        [[nodiscard]] AM_INLINE AmReal32 GetOcclusion() const
        {
            return _occlusion;
        }

        /**
         * @brief Sets the environment factor for this Entity in the given environment.
         *
         * @param environment The environment ID.
         * @param factor The environment factor.
         */
        void SetEnvironmentFactor(AmEnvironmentID environment, AmReal32 factor);

        /**
         * @brief Gets the environment factor of this Entity for the given environment.
         *
         * @param environment The environment ID.
         *
         * @return The environment factor.
         */
        [[nodiscard]] AmReal32 GetEnvironmentFactor(AmEnvironmentID environment);

        /**
         * @brief Get the list of environments where this Entity belongs or has visited.
         *
         * @return The list of environments where this Entity belongs or has visited.
         */
        [[nodiscard]] AM_INLINE const std::map<AmEnvironmentID, AmReal32>& GetEnvironments() const
        {
            return _environmentFactors;
        }

        /**
         * @brief Updates the inverse matrix of this Entity.
         *
         * This method is called automatically by the Engine on
         * each frame update.
         */
        void Update();

        ChannelList& GetPlayingSoundList()
        {
            return _playingSoundList;
        }

        [[nodiscard]] const ChannelList& GetPlayingSoundList() const
        {
            return _playingSoundList;
        }

        fplutil::intrusive_list_node node;

    private:
        AmEntityID _id;

        AmVec3 _lastLocation;
        AmVec3 _velocity;

        AmVec3 _location;
        Orientation _orientation;

        AmMat4 _inverseMatrix;

        AmReal32 _obstruction;
        AmReal32 _occlusion;

        AmReal32 _directivity;
        AmReal32 _directivitySharpness;

        std::map<AmEnvironmentID, AmReal32> _environmentFactors;

        // Keeps track of how many sounds are being played on this entity.
        ChannelList _playingSoundList;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_CORE_ENTITY_INTERNAL_STATE_H
