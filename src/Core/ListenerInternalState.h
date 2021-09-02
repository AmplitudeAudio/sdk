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

#ifndef SPARK_AUDIO_LISTENER_INTERNAL_STATE_H
#define SPARK_AUDIO_LISTENER_INTERNAL_STATE_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <Utils/intrusive_list.h>

namespace SparkyStudios::Audio::Amplitude
{
    class ListenerInternalState
    {
    public:
        ListenerInternalState();

        /**
         * @brief Gets the ID of this Listener in game.
         *
         * @return The game Listener ID.
         */
        [[nodiscard]] AmListenerID GetId() const;

        /**
         * @brief Sets the ID of this Listener in game.
         *
         * @param id The game Listener ID.
         */
        void SetId(AmListenerID id);

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

        hmm_mat4& GetInverseMatrix();

        [[nodiscard]] const hmm_mat4& GetInverseMatrix() const;

        /**
         * @brief Updates the inverse matrix of this Entity.
         *
         * This method is called automatically by the Engine on
         * each frame update.
         */
        void Update();

        fplutil::intrusive_list_node node;

    private:
        AmListenerID _id;

        hmm_vec3 _location;
        hmm_vec3 _direction;
        hmm_vec3 _up;

        hmm_mat4 _inverseMatrix;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_LISTENER_INTERNAL_STATE_H
