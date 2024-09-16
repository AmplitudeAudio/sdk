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
     * @brief Defines the material type of a @c Room wall.
     *
     * Use this enum when you want use predefined absorption coefficients for a wall.
     * Note that the predefined coefficients are only for reference, and may not be
     * accurate for your specific use case.
     *
     * You can use the @c RoomMaterialType::Custom to define a custom material. This
     * will need you to provide the absorption coefficients yourself.
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
     * @brief Enumerates the walls of a @c Room.
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
    };

    /**
     * @brief Represents the material of a @c Room wall.
     */
    struct AM_API_PUBLIC RoomMaterial
    {
        /**
         * @brief Constructs a new @c RoomMaterial.
         *
         * @note This constructor initializes a @c RoomMaterialType::Custom material.
         */
        RoomMaterial();

        /**
         * @brief Constructs a new predefined @c RoomMaterial.
         *
         * @param type The type of the material.
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
     * @brief Represents a physical space where sound waves can propagate. Any sound source within the room will be affected
     * by the room's properties, and got applied early reflections and reverberation effects.
     *
     * @note This class is a lightweight wrapper around the internal @c RoomInternalState class.
     */
    class AM_API_PUBLIC Room
    {
    public:
        /**
         * @brief Creates an uninitialized @c Room.
         *
         * @note An uninitialized @c Room doesn't affect sound sources.
         */
        Room();

        /**
         * @brief Creates a wrapper instance over the provided state.
         */
        explicit Room(RoomInternalState* state);

        /**
         * @brief Uninitializes this @c Room.
         *
         * @note This doesn't destroy the internal state it references,
         * it just removes this reference to it.
         */
        void Clear();

        /**
         * @brief Checks whether this @c Room has been initialized.
         *
         * @return @c true if this @c Room has been initialized with a
         * valid state.
         */
        [[nodiscard]] bool Valid() const;

        /**
         * @brief Returns the unique ID of this @c Room.
         *
         * @return The @c Room unique ID.
         */
        [[nodiscard]] AmRoomID GetId() const;

        /**
         * @brief Sets the location @c Room.
         *
         * @param location The new location.
         */
        void SetLocation(const AmVec3& location) const;

        /**
         * @brief Gets the current location of this @c Room.
         *
         * @return The current location of this @c Room.
         */
        [[nodiscard]] const AmVec3& GetLocation() const;

        /**
         * @brief Sets the orientation of this @c Room.
         *
         * @param orientation The new orientation.
         */
        void SetOrientation(const Orientation& orientation) const;

        /**
         * @brief Gets the current orientation of this @c Room.
         *
         * @return The current orientation of this @c Room.
         */
        [[nodiscard]] const Orientation& GetOrientation() const;

        /**
         * @brief Gets the direction vector of this @c Room.
         *
         * @return The direction (forward) vector.
         */
        [[nodiscard]] AmVec3 GetDirection() const;

        /**
         * @brief Gets the up vector of this @c Room.
         *
         * @return The up vector.
         */
        [[nodiscard]] AmVec3 GetUp() const;

        /**
         * @brief Sets the shape's dimensions of this @c Room.
         */
        void SetDimensions(AmVec3 dimensions) const;

        /**
         * @brief Sets the shape representing this @c Room.
         */
        void SetShape(const BoxShape& shape) const;

        /**
         * @brief Gets the shape representing this @c Room.
         *
         * @return The @c Room shape.
         */
        [[nodiscard]] const BoxShape& GetShape() const;

        /**
         * @brief Sets the material of a specific wall of this @c Room.
         *
         * @param wall The wall to set the material for.
         * @param material The new material.
         */
        void SetWallMaterial(RoomWall wall, const RoomMaterial& material) const;

        /**
         * @brief Sets the material of all walls of this @c Room.
         *
         * @param material The new material.
         */
        void SetAllWallMaterials(const RoomMaterial& material) const;

        /**
         * @brief Sets the material of each wall of this @c Room.
         *
         * @param leftWallMaterial The material for the left wall.
         * @param rightWallMaterial The material for the right wall.
         * @param floorMaterial The material for the floor.
         * @param ceilingMaterial The material for the ceiling.
         * @param frontWallMaterial The material for the front wall.
         * @param backWallMaterial The material for the back wall.
         */
        void SetWallMaterials(
            const RoomMaterial& leftWallMaterial,
            const RoomMaterial& rightWallMaterial,
            const RoomMaterial& floorMaterial,
            const RoomMaterial& ceilingMaterial,
            const RoomMaterial& frontWallMaterial,
            const RoomMaterial& backWallMaterial) const;

        /**
         * @brief Gets the material of a specific wall of this @c Room.
         *
         * @param wall The wall to get the material for.
         *
         * @return The material of the specified wall.
         */
        [[nodiscard]] const RoomMaterial& GetWallMaterial(RoomWall wall) const;

        /**
         * @brief Sets the gain of the early reflections of sound sources
         * of this room.
         *
         * @param gain The gain applied to early reflections of sound sources.
         */
        void SetGain(AmReal32 gain) const;

        /**
         * @brief Gets the early reflections gain.
         *
         * @return The early reflections gain.
         */
        [[nodiscard]] AmReal32 GetGain() const;

        /**
         * @brief Gets the volume of the @c Room in m3.
         *
         * @return The volume of the room.
         */
        [[nodiscard]] AmReal32 GetVolume() const;

        /**
         * @brief Update the state of this @c Room.
         *
         * This method is called automatically by the Engine
         * on each frames.
         */
        void Update() const;

        /**
         * @brief Gets the internal state of the @c Room.
         *
         * @note This method is intended for internal usage only.
         */
        [[nodiscard]] RoomInternalState* GetState() const;

    private:
        RoomInternalState* _state;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_ROOM_H
