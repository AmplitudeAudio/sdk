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
         * @brief Sets the location of this Listener.
         *
         * @param location The new location.
         */
        void SetLocation(const AmVec3& location);

        /**
         * @brief Gets the current location of this Listener.
         *
         * @return The current location of this Listener.
         */
        [[nodiscard]] const AmVec3& GetLocation() const;

        /**
         * @brief Sets the orientation of this Listener.
         *
         * @param orientation
         */
        void SetOrientation(const Orientation& orientation);

        /**
         * @brief Gets the orientation of this Listener.
         *
         * @return The current orientation of this Listener.
         */
        [[nodiscard]] const Orientation& GetOrientation() const;

        /**
         * @brief Get the direction vector of the Listener.
         *
         * @return The direction vector.
         */
        [[nodiscard]] AmVec3 GetDirection() const;

        /**
         * @brief Get the up vector of the Listener.
         *
         * @return The up vector.
         */
        [[nodiscard]] AmVec3 GetUp() const;

        AmMat4& GetInverseMatrix();

        [[nodiscard]] const AmMat4& GetInverseMatrix() const;

        /**
         * @brief Gets the velocity of the Listener.
         *
         * @return The Listener's velocity.
         */
        [[nodiscard]] const AmVec3& GetVelocity() const;

        /**
         * @brief Updates the inverse matrix of this Listener.
         *
         * This method is called automatically by the Engine on
         * each frame update.
         */
        void Update();

        fplutil::intrusive_list_node node;

    private:
        AmListenerID _id;

        AmVec3 _location;
        Orientation _orientation;

        AmVec3 _lastLocation;
        AmVec3 _velocity;

        AmMat4 _inverseMatrix;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_LISTENER_INTERNAL_STATE_H
