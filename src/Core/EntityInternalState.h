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

#ifndef SS_AMPLITUDE_AUDIO_ENTITY_INTERNAL_STATE_H
#define SS_AMPLITUDE_AUDIO_ENTITY_INTERNAL_STATE_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <Core/ChannelInternalState.h>
#include <Utils/intrusive_list.h>

namespace SparkyStudios::Audio::Amplitude
{
    typedef fplutil::intrusive_list<ChannelInternalState> ChannelList;

    class EntityInternalState
    {
    public:
        EntityInternalState();

        /**
         * @brief Gets the ID of this Entity in game.
         *
         * @return The game Entity ID.
         */
        [[nodiscard]] AmEntityID GetId() const;

        /**
         * @brief Sets the ID of this Entity in game.
         *
         * @param id The game Entity ID.
         */
        void SetId(AmEntityID id);

        /**
         * @brief Sets the location of this Entity.
         *
         * @param location The new location.
         */
        void SetLocation(const hmm_vec3& location);

        /**
         * @brief Gets the current location of this Entity.
         *
         * @return The current location of this Entity.
         */
        [[nodiscard]] const hmm_vec3& GetLocation() const;

        /**
         * @brief Sets the orientation of this Entity.
         *
         * @param direction The direction towards the Entity.
         * @param up The up vector.
         */
        void SetOrientation(const hmm_vec3& direction, const hmm_vec3& up);

        /**
         * @brief Get the direction vector of the Entity.
         *
         * @return The direction vector.
         */
        [[nodiscard]] const hmm_vec3& GetDirection() const;

        /**
         * @brief Get the up vector of the Entity.
         *
         * @return The up vector.
         */
        [[nodiscard]] const hmm_vec3& GetUp() const;

        /**
         * @brief Returns the inverse transformation matrix of this Entity.
         *
         * @return The inverse transformation matrix.
         */
        [[nodiscard]] const hmm_mat4& GetInverseMatrix() const;

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

        hmm_vec3 _location;
        hmm_vec3 _direction;
        hmm_vec3 _up;

        hmm_mat4 _inverseMatrix;

        // Keeps track of how many sounds are being played on this entity.
        ChannelList _playingSoundList;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_ENTITY_INTERNAL_STATE_H
