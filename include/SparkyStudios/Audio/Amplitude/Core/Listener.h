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

#ifndef SPARK_AUDIO_LISTENER_H
#define SPARK_AUDIO_LISTENER_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    class ListenerInternalState;

    /**
     * @brief An object whose distance from sounds determines their gain.
     *
     * The Listener class is a lightweight reference to a ListenerInternalState
     * which is managed by the Engine. Multiple Listener objects may point to
     * the same underlying data.
     */
    class Listener
    {
    public:
        /**
         * @brief Construct an uninitialized Listener.
         *
         * An uninitialized Listener cannot have its location set or queried.
         * To initialize the Listener, use <code>Engine::AddListener();</code>.
         */
        Listener();

        explicit Listener(ListenerInternalState* state);

        /**
         * @brief Uninitialize this Listener.
         *
         * Note that this does not destroy the internal state it references,
         * it just removes this reference to it. To destroy the Listener,
         * use <code>Engine::RemoveListener();</code>.
         */
        void Clear();

        /**
         * @brief Checks whether this Listener has been initialized.
         *
         * @return bool true if this Listener has been initialized.
         */
        [[nodiscard]] bool Valid() const;

        /**
         * @brief Gets the ID of this Listener in game.
         *
         * @return The game Listener ID.
         */
        [[nodiscard]] AmListenerID GetId() const;

        /**
         * @brief Gets the velocity of the Listener.
         *
         * @return The Listener's velocity.
         */
        [[nodiscard]] const AmVec3& GetVelocity() const;

        /**
         * @brief Returns the location of this Listener.
         *
         * @return AmVec3 The location of this Listener.
         */
        [[nodiscard]] const AmVec3& GetLocation() const;

        /**
         * @brief Set the location of this Listener.
         *
         * @param location The new location of this Listener.
         */
        void SetLocation(const AmVec3& location) const;

        /**
         * @brief Set the location, direction and up vector of this Listener.
         *
         * @param direction The direction of this Listener.
         * @param up THe up vector of this Listener.
         */
        void SetOrientation(const AmVec3& direction, const AmVec3& up) const;

        /**
         * @brief Update the state of this Listener.
         *
         * This method is called automatically by the Engine
         * on each frames.
         */
        void Update() const;

        /**
         * @brief Returns the internal state of this Listener.
         *
         * @return ListenerInternalState*
         */
        [[nodiscard]] ListenerInternalState* GetState() const
        {
            return _state;
        }

    private:
        ListenerInternalState* _state;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_LISTENER_H
