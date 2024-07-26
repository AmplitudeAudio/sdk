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

#include <Core/Playback/ChannelInternalState.h>
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
        [[nodiscard]] AM_INLINE AmListenerID GetId() const
        {
            return _id;
        }

        /**
         * @brief Sets the ID of this Listener in game.
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
        void SetLocation(const AmVec3& location);

        /**
         * @brief Gets the current location of this Listener.
         *
         * @return The current location of this Listener.
         */
        [[nodiscard]] AM_INLINE const AmVec3& GetLocation() const
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
        [[nodiscard]] AM_INLINE AmVec3 GetDirection() const
        {
            return _orientation.GetForward();
        }

        /**
         * @brief Get the up vector of the Listener.
         *
         * @return The up vector.
         */
        [[nodiscard]] AM_INLINE AmVec3 GetUp() const
        {
            return _orientation.GetUp();
        }

        AM_INLINE AmMat4& GetInverseMatrix()
        {
            return _inverseMatrix;
        }

        [[nodiscard]] AM_INLINE const AmMat4& GetInverseMatrix() const
        {
            return _inverseMatrix;
        }

        /**
         * @brief Gets the velocity of the Listener.
         *
         * @return The Listener's velocity.
         */
        [[nodiscard]] AM_INLINE const AmVec3& GetVelocity() const
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
         * @brief Gets the sharpness of the directivity of the Listener.
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
        AmListenerID _id;

        AmVec3 _location;
        Orientation _orientation;

        AmVec3 _lastLocation;
        AmVec3 _velocity;

        AmReal32 _directivity;
        AmReal32 _directivitySharpness;

        AmMat4 _inverseMatrix;

        // Keeps track of how many sounds are being rendered by this entity.
        ChannelList _playingSoundList;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_LISTENER_INTERNAL_STATE_H
