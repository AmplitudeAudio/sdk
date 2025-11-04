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

#ifndef _AM_CORE_ROOM_H
#define _AM_CORE_ROOM_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>
#include <SparkyStudios/Audio/Amplitude/Math/Shape.h>

namespace SparkyStudios::Audio::Amplitude
{
    class RoomInternalState;

    /**
     * @brief Defines the material type of @c Room walls.
     *
     * Use this enum when you want to use predefined absorption coefficients for a wall.
     * Note that the predefined coefficients are only for reference and may not be accurate
     * for your specific use case.
     *
     * You can use the `RoomWallMaterialType::Custom` to define a custom material. This
     * will need you to provide the absorption coefficients yourself.
     *
     * @ingroup engine
     */
    enum eRoomWallMaterialType : AmUInt8
    {
        eRoomWallMaterialType_Transparent = 0,
        eRoomWallMaterialType_AcousticTile,
        eRoomWallMaterialType_CarpetOnConcrete,
        eRoomWallMaterialType_HeavyDrapes,
        eRoomWallMaterialType_GypsumBoard,
        eRoomWallMaterialType_ConcreteUnpainted,
        eRoomWallMaterialType_Wood,
        eRoomWallMaterialType_BrickPainted,
        eRoomWallMaterialType_FoamPanel,
        eRoomWallMaterialType_Glass,
        eRoomWallMaterialType_PlasterSmooth,
        eRoomWallMaterialType_Metal,
        eRoomWallMaterialType_Marble,
        eRoomWallMaterialType_WaterSurface,
        eRoomWallMaterialType_IceSurface,
        eRoomWallMaterialType_Custom
    };

    /**
     * @brief Lists the walls in a @c Room.
     *
     * @ingroup engine
     */
    enum eRoomWall : AmUInt8
    {
        /**
         * @brief The left wall.
         */
        eRoomWall_Left = 0,

        /**
         * @brief The right wall.
         */
        eRoomWall_Right,

        /**
         * @brief The bottom wall.
         */
        eRoomWall_Floor,
        eRoomWall_Bottom = eRoomWall_Floor,

        /**
         * @brief The top wall.
         */
        eRoomWall_Ceiling,
        eRoomWall_Top = eRoomWall_Ceiling,

        /**
         * @brief The front wall.
         */
        eRoomWall_Front,

        /**
         * @brief The back wall.
         */
        eRoomWall_Back,

        /**
         * @brief An Invalid wall. Also used to mark the maximum value of the enum.
         */
        eRoomWall_Invalid
    };

    /**
     * @brief Represents the material of a @c Room wall.
     *
     * @ingroup engine
     */
    struct AM_API_PUBLIC RoomWallMaterial
    {
        /**
         * @brief Constructs a new @c RoomWallMaterial.
         *
         * @note This constructor initializes a `RoomWallMaterialType::Custom` material.
         */
        RoomWallMaterial();

        /**
         * @brief Constructs a new predefined @c RoomWallMaterial.
         *
         * This constructor will initialize a new @c RoomWallMaterial with the recommended settings
         * for the provided material type.
         *
         * @param[in] type The type of the material.
         */
        explicit RoomWallMaterial(eRoomWallMaterialType type);

        /**
         * @brief The type of the material.
         */
        eRoomWallMaterialType m_type;

        /**
         * @brief The absorption coefficients of the material.
         */
        AmReal32 m_absorptionCoefficients[9];

        /**
         * @brief Checks if two @c RoomWallMaterial objects are equal.
         *
         * @param[in] other The other @c RoomWallMaterial to compare with.
         *
         * @return @c true if the materials are equal, @c false otherwise.
         */
        bool operator==(const RoomWallMaterial& other) const;

        /**
         * @brief Checks if two @c RoomWallMaterial objects are not equal.
         *
         * @param[in] other The other @c RoomWallMaterial to compare with.
         *
         * @return @c true if the materials are not equal, @c false otherwise.
         */
        bool operator!=(const RoomWallMaterial& other) const;
    };

    /**
     * @brief Represents a physical space where sound waves can propagate.
     *
     * Any sound source within the room will be affected by the room's properties,
     * and got applied early reflections and reverberation effects.
     *
     * The @c Room class is a lightweight wrapper around the internal @c RoomInternalState class.
     *
     * @ingroup engine
     */
    class AM_API_PUBLIC Room
    {
    public:
        /**
         * @brief Creates an uninitialized room.
         *
         * @note An uninitialized room doesn't affect sound sources.
         *
         * @note To create an initialized room, use the @ref Engine::AddRoom "`AddRoom()`" method of the
         * @c Engine instance.
         *
         * @code{cpp}
         * amEngine->AddRoom(1234); // You should provide a unique ID
         * @endcode
         */
        Room();

        /**
         * @brief Creates a wrapper instance over the provided state.
         *
         * @param[in] state The internal state to wrap.
         *
         * @warning This constructor is for internal usage only.
         */
        explicit Room(RoomInternalState* state);

        /**
         * @brief Uninitializes this room.
         *
         * @note This doesn't destroy the internal state it references, it just removes this reference to it.
         *
         * @note To completely destroy the room, use the @ref Engine::RemoveRoom "`RemoveRoom()`" method
         * of the @c Engine instance.
         *
         * @code{cpp}
         * amEngine->RemoveRoom(1234); // You should provide the room ID
         * @endcode
         */
        void Clear();

        /**
         * @brief Checks whether this room has been initialized.
         *
         * @return @c true if this room has been initialized with a valid state, @c false otherwise.
         */
        [[nodiscard]] bool Valid() const;

        /**
         * @brief Returns the unique ID of this room.
         *
         * @return The room's unique ID.
         */
        [[nodiscard]] AmRoomID GetId() const;

        /**
         * @brief Sets the location room.
         *
         * @param[in] location The new location.
         */
        void SetLocation(const AmVector3& location) const;

        /**
         * @brief Gets the current location of this room.
         *
         * @return The current location of this room.
         */
        [[nodiscard]] const AmVector3& GetLocation() const;

        /**
         * @brief Sets the orientation of this room.
         *
         * @param[in] orientation The new orientation.
         */
        void SetOrientation(const Orientation& orientation) const;

        /**
         * @brief Gets the current orientation of this room.
         *
         * @return The current orientation of this room.
         */
        [[nodiscard]] const Orientation& GetOrientation() const;

        /**
         * @brief Gets the direction vector of this room.
         *
         * @return The direction (forward) vector.
         */
        [[nodiscard]] AmVector3 GetDirection() const;

        /**
         * @brief Gets the up vector of this room.
         *
         * @return The up vector.
         */
        [[nodiscard]] AmVector3 GetUp() const;

        /**
         * @brief Sets the shape's dimensions of this room.
         *
         * @param[in] dimensions The new dimensions.
         */
        void SetDimensions(AmVector3 dimensions) const;

        /**
         * @brief Sets the shape representing this room.
         *
         * @param[in] shape The new shape.
         */
        void SetShape(const BoxShape& shape) const;

        /**
         * @brief Gets the shape representing this room.
         *
         * @return The room shape.
         */
        [[nodiscard]] const BoxShape& GetShape() const;

        /**
         * @brief Sets the material for a specific wall of this room.
         *
         * @param[in] wall The wall to set the material for.
         * @param[in] material The new material.
         */
        void SetWallMaterial(eRoomWall wall, const RoomWallMaterial& material) const;

        /**
         * @brief Sets the material for all walls of this room.
         *
         * @param[in] material The new material.
         */
        void SetAllWallMaterials(const RoomWallMaterial& material) const;

        /**
         * @brief Sets the material for each wall of this room.
         *
         * @param[in] leftWallMaterial The material for the left wall.
         * @param[in] rightWallMaterial The material for the right wall.
         * @param[in] floorMaterial The material for the floor.
         * @param[in] ceilingMaterial The material for the ceiling.
         * @param[in] frontWallMaterial The material for the front wall.
         * @param[in] backWallMaterial The material for the back wall.
         */
        void SetWallMaterials(
            const RoomWallMaterial& leftWallMaterial,
            const RoomWallMaterial& rightWallMaterial,
            const RoomWallMaterial& floorMaterial,
            const RoomWallMaterial& ceilingMaterial,
            const RoomWallMaterial& frontWallMaterial,
            const RoomWallMaterial& backWallMaterial) const;

        /**
         * @brief Gets the material for a specific wall of this room.
         *
         * @param[in] wall The wall to get the material for.
         *
         * @return The material of the specified wall.
         */
        [[nodiscard]] const RoomWallMaterial& GetWallMaterial(eRoomWall wall) const;

        /**
         * @brief Sets the room effects gain.
         *
         * @param[in] gain The gain applied to early reflections and reverberations effects.
         */
        void SetGain(AmReal32 gain) const;

        /**
         * @brief Gets the room effects gain.
         *
         * @return The room effects gain.
         */
        [[nodiscard]] AmReal32 GetGain() const;

        /**
         * @brief Gets the volume of the room in m3.
         *
         * @return The volume of the room's shape.
         */
        [[nodiscard]] AmReal32 GetVolume() const;

        /**
         * @brief Gets the dimensions of the room in meters.
         *
         * @return The dimensions of the room's shape.
         */
        [[nodiscard]] AmVector3 GetDimensions() const;

        /**
         * @brief Gets the surface area for a specific wall of this room.
         *
         * @param[in] wall The wall to get the surface area for.
         *
         * @return The surface area of the specified wall.
         */
        [[nodiscard]] AmReal32 GetSurfaceArea(eRoomWall wall) const;

        /**
         * @brief Updates the state of this room.
         *
         * The @c Engine calls this method automatically on each frame to update the internal state of the room.
         *
         * @warning This method is for internal usage only.
         */
        void Update() const;

        /**
         * @brief Gets the internal state of the room.
         *
         * @return The room's internal state.
         *
         * @warning This method is for internal usage only.
         */
        [[nodiscard]] RoomInternalState* GetState() const;

    private:
        /**
         * @brief The internal state of the room.
         *
         * @internal
         */
        RoomInternalState* _state;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_ROOM_H
