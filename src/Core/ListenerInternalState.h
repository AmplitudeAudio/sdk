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

#ifndef _AM_IMPLEMENTATION_CORE_LISTENER_INTERNAL_STATE_H
#define _AM_IMPLEMENTATION_CORE_LISTENER_INTERNAL_STATE_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <Core/Playback/ChannelInternalState.h>
#include <Math/LinearAlgebra.h>
#include <Utils/intrusive_list.h>

namespace SparkyStudios::Audio::Amplitude
{
    class AM_API_PUBLIC ListenerInternalState
    {
    public:
        ListenerInternalState();

        /**
         * @brief Gets the ID of this Listener in the game.
         *
         * @return The game Listener ID.
         */
        [[nodiscard]] AM_INLINE AmListenerID GetId() const
        {
            return _id;
        }

        /**
         * @brief Sets the ID of this Listener in the game.
         *
         * @param id The game Listener ID.
         */
        AM_INLINE void SetId(AmListenerID id)
        {
            _id = id;
        }

        /**
         * @brief Sets the location of this Listener.
         *
         * @param location The new location.
         */
        void SetLocation(const AmVector3& location);

        /**
         * @brief Gets the current location of this Listener.
         *
         * @return The current location of this Listener.
         */
        [[nodiscard]] AM_INLINE const AmVector3& GetLocation() const
        {
            return _location;
        }

        /**
         * @brief Sets the orientation of this Listener.
         *
         * @param orientation The new orientation.
         */
        AM_INLINE void SetOrientation(const Orientation& orientation)
        {
            _orientation = orientation;
        }

        /**
         * @brief Gets the orientation of this Listener.
         *
         * @return The current orientation of this Listener.
         */
        [[nodiscard]] AM_INLINE const Orientation& GetOrientation() const
        {
            return _orientation;
        }

        /**
         * @brief Get the direction vector of the Listener.
         *
         * @return The direction vector.
         */
        [[nodiscard]] AM_INLINE AmVector3 GetDirection() const
        {
            return _orientation.GetForward();
        }

        /**
         * @brief Get the up vector of the Listener.
         *
         * @return The up vector.
         */
        [[nodiscard]] AM_INLINE AmVector3 GetUp() const
        {
            return _orientation.GetUp();
        }

        AM_INLINE AmMatrix4& GetInverseMatrix()
        {
            return _inverseMatrix;
        }

        [[nodiscard]] AM_INLINE const AmMatrix4& GetInverseMatrix() const
        {
            return _inverseMatrix;
        }

        /**
         * @brief Gets the velocity of the Listener.
         *
         * @return The Listener's velocity.
         */
        [[nodiscard]] AM_INLINE const AmVector3& GetVelocity() const
        {
            return _velocity;
        }

        /**
         * @brief Sets the directivity of the Listener.
         *
         * @param directivity The new directivity.
         * @param sharpness The sharpness of the directivity.
         */
        void SetDirectivity(AmReal32 directivity, AmReal32 sharpness);

        /**
         * @brief Gets the directivity of the Listener.
         *
         * @return The Listener's directivity.
         */
        [[nodiscard]] AM_INLINE AmReal32 GetDirectivity() const
        {
            return _directivity;
        }

        /**
         * @brief Gets the sharpness for the directivity of the Listener.
         *
         * @return The Listener's directivity sharpness.
         */
        [[nodiscard]] AM_INLINE AmReal32 GetDirectivitySharpness() const
        {
            return _directivitySharpness;
        }

        /**
         * @brief Updates the inverse matrix of this Listener.
         *
         * The Engine calls this method automatically on
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
        AmListenerID _id;

        AmVector3 _location;
        Orientation _orientation;

        AmVector3 _lastLocation;
        AmVector3 _velocity;

        AmReal32 _directivity;
        AmReal32 _directivitySharpness;

        AmMatrix4 _inverseMatrix;

        // Keeps track of this entity is rendering how many sounds.
        ChannelList _playingSoundList;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_CORE_LISTENER_INTERNAL_STATE_H
