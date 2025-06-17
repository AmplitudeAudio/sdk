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

#ifndef _AM_CORE_LISTENER_H
#define _AM_CORE_LISTENER_H

#include <SparkyStudios/Audio/Amplitude/Math/Orientation.h>

namespace SparkyStudios::Audio::Amplitude
{
    class ListenerInternalState;

    /**
     * @brief An object used to render sound sources.
     *
     * The @c Listener class is a lightweight reference to a @c ListenerInternalState which is managed internally
     * by the @c Engine. Multiple @c Listener objects may point to the same underlying data, and share the same state.
     *
     * @ingroup engine
     */
    class AM_API_PUBLIC Listener
    {
    public:
        /**
         * @brief Construct an uninitialized listener.
         *
         * @note An uninitialized listener cannot have its location set or queried, nor render sound sources.
         *
         * @note To create an initialized listener, use the @ref Engine::AddListener "`AddListener()`" method
         * of the @c Engine instance.
         *
         * @code{cpp}
         * amEngine->AddListener(1234); // You should provide a unique ID
         * @endcode
         */
        Listener();

        /**
         * @brief Creates a wrapper instance over the provided state.
         *
         * @param[in] state The internal state to wrap.
         *
         * @warning This constructor is for internal usage only.
         */
        explicit Listener(ListenerInternalState* state);

        /**
         * @brief Uninitializes this listener.
         *
         * @note This does not destroy the internal state it references, it just removes this reference to it.
         *
         * @note To completely destroy the listener, use @ref Engine::RemoveListener "`RemoveListener()`" method
         * of the @c Engine instance.
         *
         * @code{cpp}
         * amEngine->RemoveListener(1234); // You should provide the listener ID
         * @endcode
         */
        void Clear();

        /**
         * @brief Checks whether this listener has been initialized.
         *
         * @return @c true if this listener is initialized, @c false otherwise.
         */
        [[nodiscard]] bool Valid() const;

        /**
         * @brief Gets the ID of this listener.
         *
         * @return The listener's ID.
         */
        [[nodiscard]] AmListenerID GetId() const;

        /**
         * @brief Gets the velocity of this listener.
         *
         * @return The listener's velocity.
         */
        [[nodiscard]] const AmVec3& GetVelocity() const;

        /**
         * @brief Gets the location of this listener.
         *
         * @return The listener's location.
         */
        [[nodiscard]] const AmVec3& GetLocation() const;

        /**
         * @brief Sets the location of this listener.
         *
         * @param[in] location The new location of this listener.
         */
        void SetLocation(const AmVec3& location) const;

        /**
         * @brief Gets the direction vector of the listener.
         *
         * @return The direction vector.
         */
        [[nodiscard]] AmVec3 GetDirection() const;

        /**
         * @brief Gets the up vector of the listener.
         *
         * @return The up vector.
         */
        [[nodiscard]] AmVec3 GetUp() const;

        /**
         * @brief Sets the location, direction and up vector of this listener, through an Orientation representation.
         *
         * @param[in] orientation The new orientation of this listener.
         */
        void SetOrientation(const Orientation& orientation) const;

        /**
         * @brief Gets the orientation of this listener.
         *
         * @return The listener's orientation.
         */
        [[nodiscard]] Orientation GetOrientation() const;

        /**
         * @brief Sets the directivity and sharpness of this listener.
         *
         * This affects how sounds are perceived by the listener.
         *
         * @param[in] directivity The directivity of the listener, in the range [0, 1].
         * @param[in] sharpness The directivity sharpness of the listener, in the range [1, +INF]. Increasing this
         * value increases the directivity towards the front of the listener.
         */
        void SetDirectivity(AmReal32 directivity, AmReal32 sharpness) const;

        /**
         * @brief Gets the directivity of sounds rendered by this listener.
         *
         * @return The directivity of sound sources.
         */
        [[nodiscard]] AmReal32 GetDirectivity() const;

        /**
         * @brief Gets the directivity sharpness of sounds rendered by this listener.
         *
         * @return The directivity sharpness of sounds rendered by this listener.
         */
        [[nodiscard]] AmReal32 GetDirectivitySharpness() const;

        /**
         * @brief Gets the inverse matrix of the listener.
         *
         * You can use this matrix to convert locations from global space to listener space.
         */
        [[nodiscard]] const AmMat4& GetInverseMatrix() const;

        /**
         * @brief Update the state of this listener.
         *
         * The @c Engine calls this method automatically on each frame to update the internal state of the listener.
         *
         * @warning This method is for internal usage only.
         */
        void Update() const;

        /**
         * @brief Returns the internal state of this listener.
         *
         * @return The listener's internal state.
         *
         * @warning This method is for internal usage only.
         */
        [[nodiscard]] ListenerInternalState* GetState() const;

    private:
        /**
         * @brief The internal state of the listener.
         *
         * @internal
         */
        ListenerInternalState* _state;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_LISTENER_H
