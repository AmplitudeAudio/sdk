// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#ifndef _AM_IMPLEMENTATION_CORE_ROOM_INTERNAL_STATE_H
#define _AM_IMPLEMENTATION_CORE_ROOM_INTERNAL_STATE_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Math/Orientation.h>
#include <SparkyStudios/Audio/Amplitude/Math/Shape.h>

#include <Core/Playback/ChannelInternalState.h>
#include <Utils/intrusive_list.h>

namespace SparkyStudios::Audio::Amplitude
{
    class RoomReflectionsProperties
    {
    public:
        RoomReflectionsProperties()
            : _shape(0.0f, 0.0f, 0.0f)
            , _cutOffFrequency(0.0f)
            , _coefficients{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }
        {}

        [[nodiscard]] const AmVec3& GetRoomLocation() const
        {
            return _shape.GetLocation();
        }

        AM_INLINE void SetRoomLocation(const AmVec3& location)
        {
            _shape.SetLocation(location);
        }

        [[nodiscard]] const Orientation& GetRoomOrientation() const
        {
            return _shape.GetOrientation();
        }

        AM_INLINE void SetRoomOrientation(const Orientation& orientation)
        {
            _shape.SetOrientation(orientation);
        }

        [[nodiscard]] AM_INLINE AmVec3 GetRoomDirection() const
        {
            return _shape.GetOrientation().GetForward();
        }

        [[nodiscard]] AM_INLINE AmVec3 GetRoomUp() const
        {
            return _shape.GetOrientation().GetUp();
        }

        [[nodiscard]] AM_INLINE AmReal32 GetRoomWidth() const
        {
            return _shape.GetWidth();
        }

        [[nodiscard]] AM_INLINE AmReal32 GetRoomDepth() const
        {
            return _shape.GetDepth();
        }

        [[nodiscard]] AM_INLINE AmReal32 GetRoomHeight() const
        {
            return _shape.GetHeight();
        }

        [[nodiscard]] AM_INLINE const BoxShape& GetRoomShape() const
        {
            return _shape;
        }

        [[nodiscard]] AM_INLINE BoxShape& GetRoomShape()
        {
            return _shape;
        }

        AM_INLINE void SetRoomShape(const BoxShape& shape)
        {
            _shape = shape;
        }

        [[nodiscard]] AM_INLINE AmReal32 GetCutOffFrequency() const
        {
            return _cutOffFrequency;
        }

        AM_INLINE void SetCutOffFrequency(AmReal32 frequency)
        {
            _cutOffFrequency = frequency;
        }

        [[nodiscard]] AM_INLINE AmReal32* GetCoefficients()
        {
            return _coefficients;
        }

        AM_INLINE const AmReal32* GetCoefficients() const
        {
            return _coefficients;
        }

        [[nodiscard]] AM_INLINE AmReal32 GetGain() const
        {
            return _gain;
        }

        AM_INLINE void SetGain(AmReal32 gain)
        {
            _gain = gain;
        }

    private:
        BoxShape _shape;
        AmReal32 _cutOffFrequency;
        AmReal32 _coefficients[6];
        AmReal32 _gain;
    };

    class RoomInternalState
    {
        friend class Room;

    public:
        RoomInternalState()
            : _id(kAmInvalidObjectId)
            , _reflectionsProperties()
            , _needUpdate(false)
            , _playingSoundList(&ChannelInternalState::room_node)
        {}

        /**
         * @brief Gets the ID of this room.
         *
         * @return The room ID.
         */
        [[nodiscard]] AM_INLINE AmRoomID GetId() const
        {
            return _id;
        }

        /**
         * @brief Sets the ID of this room.
         *
         * @param id The room ID.
         */
        AM_INLINE void SetId(AmRoomID id)
        {
            _id = id;
        }

        /**
         * @brief Sets the location of this room.
         *
         * @param location The new location.
         */
        AM_INLINE void SetLocation(const AmVec3& location)
        {
            _reflectionsProperties.SetRoomLocation(location);
            _needUpdate = true;
        }

        /**
         * @brief Gets the current location of this Environment.
         *
         * @return The current location of this Environment.
         */
        [[nodiscard]] AM_INLINE const AmVec3& GetLocation() const
        {
            return _reflectionsProperties.GetRoomLocation();
        }

        /**
         * @brief Sets the orientation of this Environment.
         *
         * @param orientation The new orientation.
         */
        AM_INLINE void SetOrientation(const Orientation& orientation)
        {
            _reflectionsProperties.SetRoomOrientation(orientation);
            _needUpdate = true;
        }

        /**
         * @brief Gets the orientation of this Environment.
         *
         * @return The current orientation of this Environment.
         */
        [[nodiscard]] AM_INLINE const Orientation& GetOrientation() const
        {
            return _reflectionsProperties.GetRoomOrientation();
        }

        /**
         * @brief Get the direction vector of the Environment.
         *
         * @return The direction vector.
         */
        [[nodiscard]] AM_INLINE AmVec3 GetDirection() const
        {
            return _reflectionsProperties.GetRoomDirection();
        }

        /**
         * @brief Get the up vector of the Environment.
         *
         * @return The up vector.
         */
        [[nodiscard]] AM_INLINE AmVec3 GetUp() const
        {
            return _reflectionsProperties.GetRoomUp();
        }

        /**
         * @brief Sets the shape of this room.
         *
         * @param shape The new shape.
         */
        AM_INLINE void SetShape(const BoxShape& shape)
        {
            _reflectionsProperties.SetRoomShape(shape);
            _needUpdate = true;
        }

        /**
         * @brief Gets the shape of this room.
         *
         * @return The current shape.
         */
        [[nodiscard]] AM_INLINE const BoxShape& GetShape() const
        {
            return _reflectionsProperties.GetRoomShape();
        }

        /**
         * @brief Gets the shape of this room.
         *
         * @return The current shape.
         */
        [[nodiscard]] AM_INLINE BoxShape& GetShape()
        {
            return _reflectionsProperties.GetRoomShape();
        }

        AM_INLINE void SetWallMaterial(RoomWall wall, const RoomMaterial& material)
        {
            _materials[static_cast<AmSize>(wall)] = material;
        }

        [[nodiscard]] AM_INLINE const RoomMaterial& GetWallMaterial(RoomWall wall) const
        {
            return _materials[static_cast<AmSize>(wall)];
        }

        [[nodiscard]] AM_INLINE AmVec3 GetDimensions() const
        {
            return { _reflectionsProperties.GetRoomWidth(), _reflectionsProperties.GetRoomDepth(), _reflectionsProperties.GetRoomHeight() };
        }

        AM_INLINE void SetDimensions(const AmVec3& dimensions)
        {
            auto& shape = _reflectionsProperties.GetRoomShape();
            shape.SetHalfWidth(dimensions.X * 0.5f);
            shape.SetHalfHeight(dimensions.Z * 0.5f);
            shape.SetHalfDepth(dimensions.Y * 0.5f);
            _needUpdate = true;
        }

        /**
         * @brief Sets the cut-off frequency of this room.
         *
         * @param frequency The new cut-off frequency.
         */
        AM_INLINE void SetCutOffFrequency(AmReal32 frequency)
        {
            _reflectionsProperties.SetCutOffFrequency(frequency);
            _needUpdate = true;
        }

        /**
         * @brief Gets the cut-off frequency of this room.
         *
         * @return The current cut-off frequency.
         */
        [[nodiscard]] AM_INLINE AmReal32 GetCutOffFrequency() const
        {
            return _reflectionsProperties.GetCutOffFrequency();
        }

        [[nodiscard]] AM_INLINE AmReal32 GetVolume() const
        {
            const auto& dimensions = GetDimensions();
            return dimensions.X * dimensions.Y * dimensions.Z;
        }

        [[nodiscard]] AM_INLINE AmReal32 GetSurfaceArea(RoomWall wall) const
        {
            const auto& dimensions = GetDimensions();

            switch (wall)
            {
            case RoomWall::Front:
            case RoomWall::Back:
                return dimensions.X * dimensions.Z;
            case RoomWall::Left:
            case RoomWall::Right:
                return dimensions.Y * dimensions.Z;
            case RoomWall::Top:
            case RoomWall::Bottom:
                return dimensions.X * dimensions.Y;
            default:
                return 0.0f;
            }
        }

        [[nodiscard]] AM_INLINE AmReal32* GetCoefficients()
        {
            return _reflectionsProperties.GetCoefficients();
        }

        [[nodiscard]] const AmReal32* GetCoefficients() const
        {
            return _reflectionsProperties.GetCoefficients();
        }

        [[nodiscard]] AM_INLINE AmReal32 GetGain() const
        {
            return _reflectionsProperties.GetGain();
        }

        AM_INLINE void SetGain(AmReal32 gain)
        {
            _reflectionsProperties.SetGain(gain);
            _needUpdate = true;
        }

        /**
         * @brief Updates the reflection properties of this room.
         *
         * @note This method is called automatically by the Engine on
         * each frame update.
         */
        void Update();

        [[nodiscard]] AM_INLINE bool NeedsUpdate() const
        {
            return _needUpdate;
        }

        AM_INLINE void SetWasUpdated(bool updated)
        {
            _wasUpdated = updated;
        }

        [[nodiscard]] AM_INLINE bool WasUpdated() const
        {
            return _wasUpdated;
        }

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
        AmRoomID _id;

        RoomMaterial _materials[6];
        RoomReflectionsProperties _reflectionsProperties;

        bool _needUpdate;
        bool _wasUpdated;

        ChannelList _playingSoundList;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_CORE_ROOM_INTERNAL_STATE_H
