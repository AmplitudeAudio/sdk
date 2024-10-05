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
     * @brief A object which can render sound sources.
     *
     * The `Listener` class is a lightweight reference to a `ListenerInternalState`
     * which is managed by the `Engine`. Multiple `Listener` objects may point to
     * the same underlying data.
     *
     * @ingroup engine
     */
    class AM_API_PUBLIC Listener
    {
    public:
        /**
         * @brief Construct an uninitialized `Listener`.
         *
         * An uninitialized `Listener` cannot have its location set or queried, nor
         * render sound sources.
         *
         * To create an initialized `Listener`, use the `AddListener()` method of the
         * `Engine` instance.
         * ```cpp
         * amEngine->AddListener(1234); // You should provide an unique ID
         * ```
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
         * @brief Uninitializes this `Listener`.
         *
         * Note that this does not destroy the internal state it references,
         * it just removes this reference to it.
         *
         * To completely destroy the `Listener`, use `RemoveListener()` method
         * of the `Engine` instance.
         * ```cpp
         * amEngine->RemoveListener(1234); // You should provide the listener ID
         * ```
         */
        void Clear();

        /**
         * @brief Checks whether this `Listener` has been initialized.
         *
         * @return `true` if this `Listener` is initialized, `false` otherwise.
         */
        [[nodiscard]] bool Valid() const;

        /**
         * @brief Gets the ID of this `Listener`.
         *
         * @return The `Listener` ID.
         */
        [[nodiscard]] AmListenerID GetId() const;

        /**
         * @brief Gets the velocity of the `Listener`.
         *
         * @return The listener's velocity.
         */
        [[nodiscard]] const AmVec3& GetVelocity() const;

        /**
         * @brief Gets the location of this `Listener`.
         *
         * @return The location of this `Listener`.
         */
        [[nodiscard]] const AmVec3& GetLocation() const;

        /**
         * @brief Sets the location of this `Listener`.
         *
         * @param[in] location The new location of this `Listener`.
         */
        void SetLocation(const AmVec3& location) const;

        /**
         * @brief Gets the direction vector of the `Listener`.
         *
         * @return The direction vector.
         */
        [[nodiscard]] AmVec3 GetDirection() const;

        /**
         * @brief Gets the up vector of the `Listener`.
         *
         * @return The up vector.
         */
        [[nodiscard]] AmVec3 GetUp() const;

        /**
         * @brief Sets the location, direction and up vector of this `Listener`.
         *
         * @param[in] orientation The new orientation of this `Listener`.
         */
        void SetOrientation(const Orientation& orientation) const;

        /**
         * @brief Gets the orientation of the `Listener`.
         *
         * @return The orientation of this `Listener`.
         */
        [[nodiscard]] Orientation GetOrientation() const;

        /**
         * @brief Sets the directivity and sharpness of `Listener`. This affects how sounds are perceived
         * by the `Listener`.
         *
         * @param[in] directivity The directivity of the listener, in the range [0, 1].
         * @param[in] sharpness The directivity sharpness of the listener, in the range [1, +INF].
         * Increasing this value increases the directivity towards the front of the listener.
         */
        void SetDirectivity(AmReal32 directivity, AmReal32 sharpness) const;

        /**
         * @brief Gets the directivity of sounds played by this `Listener`.
         *
         * @return The directivity of sound sources.
         */
        [[nodiscard]] AmReal32 GetDirectivity() const;

        /**
         * @brief Gets the directivity sharpness of sounds played by this `Listener`.
         *
         * @return The directivity sharpness of sounds played by this `Listener`.
         */
        [[nodiscard]] AmReal32 GetDirectivitySharpness() const;

        /**
         * @brief Gets the inverse matrix of the `Listener`.
         *
         * You can use this matrix to convert locations from global space to `Listener` space.
         */
        [[nodiscard]] const AmMat4& GetInverseMatrix() const;

        /**
         * @brief Update the state of this `Listener`.
         *
         * This method is called automatically by the `Engine`
         * on each frames to update the internal state of the `Listener`
         *
         * @warning This method is for internal usage only.
         */
        void Update() const;

        /**
         * @brief Returns the internal state of this Listener.
         *
         * @return The `Listener` internal state.
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
