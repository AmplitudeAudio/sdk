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
     * @brief Defines the material type of a `Room` wall.
     *
     * Use this enum when you want use predefined absorption coefficients for a wall.
     * Note that the predefined coefficients are only for reference, and may not be
     * accurate for your specific use case.
     *
     * You can use the `RoomMaterialType::Custom` to define a custom material. This
     * will need you to provide the absorption coefficients yourself.
     *
     * @ingroup engine
     */
    enum class RoomMaterialType : AmUInt8
    {
        Transparent = 0,
        AcousticTile,
        CarpetOnConcrete,
        HeavyDrapes,
        GypsumBoard,
        ConcreteUnpainted,
        Wood,
        BrickPainted,
        FoamPanel,
        Glass,
        PlasterSmooth,
        Metal,
        Marble,
        WaterSurface,
        IceSurface,
        Custom
    };

    /**
     * @brief Enumerates the walls of a `Room`.
     *
     * @ingroup engine
     */
    enum class RoomWall : AmUInt8
    {
        /**
         * @brief The left wall.
         */
        Left = 0,

        /**
         * @brief The right wall.
         */
        Right,

        /**
         * @brief The bottom wall.
         */
        Floor,
        Bottom = Floor,

        /**
         * @brief The top wall.
         */
        Ceiling,
        Top = Ceiling,

        /**
         * @brief The front wall.
         */
        Front,

        /**
         * @brief The back wall.
         */
        Back,

        /**
         * @brief An Invalid wall. Also used to mark the maximum value of the enum.
         */
        Invalid
    };

    /**
     * @brief Represents the material of a `Room` wall.
     *
     * @ingroup engine
     */
    struct AM_API_PUBLIC RoomMaterial
    {
        /**
         * @brief Constructs a new `RoomMaterial`.
         *
         * @note This constructor initializes a `RoomMaterialType`::Custom material.
         */
        RoomMaterial();

        /**
         * @brief Constructs a new predefined `RoomMaterial`.
         *
         * @param[in] type The type of the material.
         */
        explicit RoomMaterial(RoomMaterialType type);

        /**
         * @brief The type of the material.
         */
        RoomMaterialType m_type;

        /**
         * @brief The absorption coefficients of the material.
         */
        AmReal32 m_absorptionCoefficients[9];
    };

    /**
     * @brief Represents a physical space where sound waves can propagate.
     *
     * Any sound source within the room will be affected by the room's properties,
     * and got applied early reflections and reverberation effects.
     *
     * This class is a lightweight wrapper around the internal `RoomInternalState` class.
     *
     * @ingroup engine
     */
    class AM_API_PUBLIC Room
    {
    public:
        /**
         * @brief Creates an uninitialized `Room`.
         *
         * An uninitialized `Room` doesn't affect sound sources.
         *
         * To create an initialized `Room`, use the `AddRoom()` method of the
         * `Engine` instance.
         * ```cpp
         * amEngine->AddRoom(1234); // You should provide an unique ID
         * ```
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
         * @brief Uninitializes this `Room`.
         *
         * This doesn't destroy the internal state it references,
         * it just removes this reference to it.
         *
         * To completely destroy the `Room`, use `RemoveRoom()` method
         * of the `Engine` instance.
         * ```cpp
         * amEngine->RemoveRoom(1234); // You should provide the room ID
         * ```
         */
        void Clear();

        /**
         * @brief Checks whether this `Room` has been initialized.
         *
         * @return `true` if this `Room` has been initialized with a
         * valid state.
         */
        [[nodiscard]] bool Valid() const;

        /**
         * @brief Returns the unique ID of this `Room`.
         *
         * @return The `Room` unique ID.
         */
        [[nodiscard]] AmRoomID GetId() const;

        /**
         * @brief Sets the location `Room`.
         *
         * @param[in] location The new location.
         */
        void SetLocation(const AmVec3& location) const;

        /**
         * @brief Gets the current location of this `Room`.
         *
         * @return The current location of this `Room`.
         */
        [[nodiscard]] const AmVec3& GetLocation() const;

        /**
         * @brief Sets the orientation of this `Room`.
         *
         * @param[in] orientation The new orientation.
         */
        void SetOrientation(const Orientation& orientation) const;

        /**
         * @brief Gets the current orientation of this `Room`.
         *
         * @return The current orientation of this `Room`.
         */
        [[nodiscard]] const Orientation& GetOrientation() const;

        /**
         * @brief Gets the direction vector of this `Room`.
         *
         * @return The direction (forward) vector.
         */
        [[nodiscard]] AmVec3 GetDirection() const;

        /**
         * @brief Gets the up vector of this `Room`.
         *
         * @return The up vector.
         */
        [[nodiscard]] AmVec3 GetUp() const;

        /**
         * @brief Sets the shape's dimensions of this `Room`.
         *
         * @param[in] dimensions The new dimensions.
         */
        void SetDimensions(AmVec3 dimensions) const;

        /**
         * @brief Sets the shape representing this `Room`.
         *
         * @param[in] shape The new shape.
         */
        void SetShape(const BoxShape& shape) const;

        /**
         * @brief Gets the shape representing this `Room`.
         *
         * @return The `Room` shape.
         */
        [[nodiscard]] const BoxShape& GetShape() const;

        /**
         * @brief Sets the material of a specific wall of this `Room`.
         *
         * @param[in] wall The wall to set the material for.
         * @param[in] material The new material.
         */
        void SetWallMaterial(RoomWall wall, const RoomMaterial& material) const;

        /**
         * @brief Sets the material of all walls of this `Room`.
         *
         * @param[in] material The new material.
         */
        void SetAllWallMaterials(const RoomMaterial& material) const;

        /**
         * @brief Sets the material of each wall of this `Room`.
         *
         * @param[in] leftWallMaterial The material for the left wall.
         * @param[in] rightWallMaterial The material for the right wall.
         * @param[in] floorMaterial The material for the floor.
         * @param[in] ceilingMaterial The material for the ceiling.
         * @param[in] frontWallMaterial The material for the front wall.
         * @param[in] backWallMaterial The material for the back wall.
         */
        void SetWallMaterials(
            const RoomMaterial& leftWallMaterial,
            const RoomMaterial& rightWallMaterial,
            const RoomMaterial& floorMaterial,
            const RoomMaterial& ceilingMaterial,
            const RoomMaterial& frontWallMaterial,
            const RoomMaterial& backWallMaterial) const;

        /**
         * @brief Gets the material of a specific wall of this `Room`.
         *
         * @param[in] wall The wall to get the material for.
         *
         * @return The material of the specified wall.
         */
        [[nodiscard]] const RoomMaterial& GetWallMaterial(RoomWall wall) const;

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
         * @brief Gets the volume of the `Room` in m3.
         *
         * @return The volume of the room's shape.
         */
        [[nodiscard]] AmReal32 GetVolume() const;

        /**
         * @brief Gets the surface area of a specific wall of this `Room`.
         *
         * @param[in] wall The wall to get the surface area for.
         *
         * @return The surface area of the specified wall.
         */
        [[nodiscard]] AmReal32 GetSurfaceArea(RoomWall wall) const;

        /**
         * @brief Updates the state of this `Room`.
         *
         * This method is called automatically by the Engine
         * on each frames to update the internal state of the `Room`
         *
         * @warning This method is for internal usage only.
         */
        void Update() const;

        /**
         * @brief Gets the internal state of the `Room`.
         *
         * @return The `Room` internal state.
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
