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

#ifndef _AM_MATH_SHAPE_H
#define _AM_MATH_SHAPE_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>
#include <SparkyStudios/Audio/Amplitude/Core/Listener.h>

namespace SparkyStudios::Audio::Amplitude
{
    class ShapeDefinition;
    class BoxShapeDefinition;
    class CapsuleShapeDefinition;
    class ConeShapeDefinition;
    class SphereShapeDefinition;

    /**
     * @brief A geometrical closed 3D shape.
     *
     * A `Shape` defines a zone in the world where game objects (listener, sound sources, entities, etc.) can
     * be localized. Shapes are used in many places of the engine, like to define a sound attenuation shape, or to build a room.
     *
     * @ingroup math
     */
    class AM_API_PUBLIC Shape
    {
    public:
        /**
         * @brief Creates a new `Shape` from a definition.
         *
         * @param[in] definition The definition of the shape generated
         * from a flatbuffer binary.
         *
         * @warning This method is intended for internal usage only.
         */
        static Shape* Create(const ShapeDefinition* definition);

        /**
         * @brief Constructs a new `Shape`.
         */
        Shape();

        /**
         * @brief Default destructor.
         */
        virtual ~Shape() = default;

        /**
         * @brief Gets the shortest distance to the edge of this shape.
         *
         * @param[in] entity The entity from which calculate the distance.
         *
         * @return The shortest distance from the entity location to the edge
         * of this shape. If negative, the given entity in outside the shape.
         */
        [[nodiscard]] virtual AmReal32 GetShortestDistanceToEdge(const Entity& entity);

        /**
         * @brief Gets the shortest distance to the edge of this shape.
         *
         * @param[in] listener The listener from which calculate the distance.
         *
         * @return The shortest distance from the listener location to the edge
         * of this shape. If negative, the given listener in outside the shape.
         */
        [[nodiscard]] virtual AmReal32 GetShortestDistanceToEdge(const Listener& listener);

        /**
         * @brief Gets the shortest distance to the edge of this shape.
         *
         * @param[in] location The location from which calculate the distance.
         *
         * @return The shortest distance from the location to the edge
         * of this shape. If negative, the given location in outside the shape.
         */
        [[nodiscard]] virtual AmReal32 GetShortestDistanceToEdge(const AmVec3& location) = 0;

        /**
         * @brief Checks if the given entity is contained in this shape.
         *
         * @param[in] entity The entity to check.
         *
         * @return `true` if the shape contains the entity, `false` otherwise.
         */
        [[nodiscard]] virtual bool Contains(const Entity& entity);

        /**
         * @brief Checks if the given listener is contained in this shape.
         *
         * @param[in] listener The listener to check.
         *
         * @return `true` if the shape contains the listener, `false` otherwise.
         */
        [[nodiscard]] virtual bool Contains(const Listener& listener);

        /**
         * @brief Checks if the given position is contained in this shape.
         *
         * @param[in] location The 3D position to check.
         *
         * @return `true` if the shape contains the given position, `false` otherwise.
         */
        [[nodiscard]] virtual bool Contains(const AmVec3& location) = 0;

        /**
         * @brief Sets the location of this shape in the 3D environment.
         *
         * @param[in] location The shape location.
         */
        void SetLocation(const AmVec3& location);

        /**
         * @brief Sets the orientation of this shape.
         *
         * @param[in] orientation The new orientation.
         */
        void SetOrientation(const Orientation& orientation);

        /**
         * @brief Gets the orientation of this shape.
         *
         * @return The orientation of this shape.
         */
        [[nodiscard]] const Orientation& GetOrientation() const;

        /**
         * @brief Gets the LookAt transformation matrix for this shape.
         *
         * @return The look-at transformation matrix.
         */
        [[nodiscard]] const AmMat4& GetLookAt() const;

        /**
         * @brief Gets the position of this shape in the 3D environment.
         *
         * @return The shape's position.
         */
        [[nodiscard]] const AmVec3& GetLocation() const;

        /**
         * @brief Gets the position of this shape in the 3D environment.
         *
         * @return The shape's position.
         */
        [[nodiscard]] AmVec3 GetDirection() const;

        /**
         * @brief Gets the up vector of the zone.
         *
         * @return The up vector.
         */
        [[nodiscard]] AmVec3 GetUp() const;

    protected:
        virtual void Update() = 0;

        AmVec3 m_location;
        Orientation m_orientation;

        AmMat4 m_lookAtMatrix;

        bool m_needUpdate;
    };

    /**
     * @brief A tuple of shapes that represents a zone in the world.
     *
     * This shape is mainly used by attenuations and environments. It's composed of an inner `Shape` and an outer `Shape`.
     * The inner shape is the place where the @ref GetFactor factor is equal to one all the time. The outer shape is the place where the
     * @ref GetFactor factor increase or decrease according to the shortest distance of the game object from the outer edge.
     *
     * If the game object is outside the outer shape (thus, outside the zone), the @ref GetFactor factor is zero.
     *
     * @ingroup math
     */
    class AM_API_PUBLIC Zone
    {
    public:
        /**
         * @brief Creates a new `Zone` from the given inner and outer shapes.
         *
         * @param[in] inner The inner shape.
         * @param[in] outer The outer shape.
         */
        explicit Zone(Shape* inner, Shape* outer);

        /**
         * @brief Default destructor.
         */
        virtual ~Zone() = default;

        /**
         * @brief Gets the factor according to the position of the given entity in the zone.
         *
         * @param[in] entity The entity to get the factor for.
         *
         * @return The factor.
         *
         * @note The factor is a value in the range [0, 1].
         */
        [[nodiscard]] virtual AM_INLINE AmReal32 GetFactor(const Entity& entity)
        {
            return GetFactor(entity.GetLocation());
        }

        /**
         * @brief Gets the factor according to the position of the given listener in the zone.
         *
         * @param[in] listener The listener to get the factor for.
         *
         * @return The factor.
         *
         * @note The factor is a value in the range [0, 1].
         */
        [[nodiscard]] virtual AM_INLINE AmReal32 GetFactor(const Listener& listener)
        {
            return GetFactor(listener.GetLocation());
        }

        /**
         * @brief Gets the factor according to the given position in the zone.
         *
         * @param[in] position The position in the zone to get the factor for.
         *
         * @return The factor.
         *
         * @note The factor is a value in the range [0, 1].
         */
        [[nodiscard]] virtual AmReal32 GetFactor(const AmVec3& position) = 0;

        /**
         * @brief Sets the location of this zone in the 3D environment.
         *
         * @param[in] location The zone location.
         */
        void SetLocation(const AmVec3& location);

        /**
         * @brief Gets the current location of this zone.
         *
         * @return The current location of this zone.
         */
        [[nodiscard]] const AmVec3& GetLocation() const;

        /**
         * @brief Sets the orientation of this zone.
         *
         * @param[in] orientation The new orientation.
         */
        void SetOrientation(const Orientation& orientation);

        /**
         * @brief Gets the orientation of this zone.
         *
         * @return The orientation of this zone.
         */
        [[nodiscard]] const Orientation& GetOrientation() const;

        /**
         * @brief Gets the direction vector of the zone.
         *
         * @return The direction vector.
         */
        [[nodiscard]] AmVec3 GetDirection() const;

        /**
         * @brief Gets the up vector of the zone.
         *
         * @return The up vector.
         */
        [[nodiscard]] AmVec3 GetUp() const;

    protected:
        /**
         * @brief The inner shape of the zone.
         */
        Shape* m_innerShape;

        /**
         * @brief The outer shape of the zone.
         */
        Shape* m_outerShape;
    };

    /**
     * @brief A box shape, defined by a width, an height, and a depth.
     *
     * @ingroup math
     */
    class AM_API_PUBLIC BoxShape : public Shape
    {
        friend class BoxZone;

    public:
        /**
         * @brief Creates a new `BoxShape` from a definition.
         *
         * @param[in] definition The definition of the shape generated
         * from a flatbuffer binary.
         *
         * @warning This method is intended for internal usage only.
         */
        static BoxShape* Create(const BoxShapeDefinition* definition);

        /**
         * @brief Constructs a new `BoxShape`.
         *
         * @param[in] halfWidth The half width of the box shape.
         * @param[in] halfHeight The half height of the box shape.
         * @param[in] halfDepth The half depth of the box shape.
         */
        explicit BoxShape(AmReal32 halfWidth, AmReal32 halfHeight, AmReal32 halfDepth);

        /**
         * @brief Constructs a new `BoxShape`.
         *
         * @param[in] position The position of the box shape.
         * @param[in] dimensions The dimensions of the box shape.
         */
        explicit BoxShape(const AmVec3& position, const AmVec3& dimensions);

        /**
         * @brief Gets the half width of the `BoxShape`.
         *
         * @return The box shape's half width.
         */
        [[nodiscard]] AmReal32 GetHalfWidth() const;

        /**
         * @brief Gets the half height of the `BoxShape`.
         *
         * @return The box shape's half height.
         */
        [[nodiscard]] AmReal32 GetHalfHeight() const;

        /**
         * @brief Gets the half depth of the `BoxShape`.
         *
         * @return The box shape's half depth.
         */
        [[nodiscard]] AmReal32 GetHalfDepth() const;

        /**
         * @brief Gets the width of the `BoxShape`.
         *
         * @return The box shape's width.
         */
        [[nodiscard]] AmReal32 GetWidth() const;

        /**
         * @brief Gets the height of the `BoxShape`.
         *
         * @return The box shape's height.
         */
        [[nodiscard]] AmReal32 GetHeight() const;

        /**
         * @brief Gets the depth of the `BoxShape`.
         *
         * @return The box shape's depth.
         */
        [[nodiscard]] AmReal32 GetDepth() const;

        /**
         * @brief Sets the half width of the `BoxShape`.
         *
         * @param[in] halfWidth The new box shape's half width.
         */
        void SetHalfWidth(AmReal32 halfWidth);

        /**
         * @brief Sets the half height of the `BoxShape`.
         *
         * @param[in] halfHeight The new box shape's half height.
         */
        void SetHalfHeight(AmReal32 halfHeight);

        /**
         * @brief Sets the half depth of the `BoxShape`.
         *
         * @param[in] halfDepth The new box shape's half depth.
         */
        void SetHalfDepth(AmReal32 halfDepth);

        /**
         * @brief Gets the shortest distance to the edge of this shape.
         *
         * @param[in] location The location from which calculate the distance.
         *
         * @return The shortest distance from the location to the edge
         * of this shape. If negative, the given location in outside the shape.
         */
        [[nodiscard]] AmReal32 GetShortestDistanceToEdge(const AmVec3& location) final;

        /**
         * @brief Checks if the given position is contained in this shape.
         *
         * @param[in] location The 3D position to check.
         *
         * @return `true` if the shape contains the given position, `false` otherwise.
         */
        [[nodiscard]] bool Contains(const AmVec3& location) final;

        /**
         * @brief Gets the closest point to the given location.
         *
         * @param[in] location The location to get the closest point for.
         *
         * @return The closest point to the given location.
         */
        [[nodiscard]] AmVec3 GetClosestPoint(const AmVec3& location) const;

        /**
         * @brief Gets the corners of the box shape.
         *
         * The returned corners are arranged in the following order:
         *
         *     6 - - - - 4
         *     | \       | \
         *     |   3 - - - - 5
         *     |   |     |   |
         *      1  | - - 7   |
         *       \ |       \ |
         *         0 - - - - 2
         *
         * @return The corners of the box shape.
         */
        [[nodiscard]] std::array<AmVec3, 8> GetCorners() const;

        /**
         * @brief Compares this shape with another shape for equality.
         *
         * @note Shapes are equal if they have the same dimensions, the position, and the same orientation.
         *
         * @param[in] other The other shape to compare with.
         *
         * @return `true` if the shapes are equal, `false` otherwise.
         */
        bool operator==(const BoxShape& other) const;

        /**
         * @brief Compares this shape with another shape for inequality.
         *
         * @note Shapes are equal if they have the same dimensions, the position, and the same orientation.
         *
         * @param[in] other The other shape to compare with.
         *
         * @return `false` if the shapes are equal, `true` otherwise.
         */
        bool operator!=(const BoxShape& other) const;

    private:
        void Update() override;

        AmReal32 _halfWidth;
        AmReal32 _halfHeight;
        AmReal32 _halfDepth;

        AmVec3 _u;
        AmVec3 _v;
        AmVec3 _w;

        AmVec3 _p1, _p2, _p3, _p4;
        AmReal32 _uP1, _vP1, _wP1, _uP2, _vP3, _wP4;
    };

    /**
     * @brief A capsule shape, defined by a radius and an height.
     *
     * @ingroup math
     */
    class AM_API_PUBLIC CapsuleShape : public Shape
    {
        friend class CapsuleZone;

    public:
        /**
         * @brief Creates a new `CapsuleShape` from a definition.
         *
         * @param[in] definition The definition of the shape generated
         * from a flatbuffer binary.
         *
         * @warning This method is intended for internal usage only.
         */
        static CapsuleShape* Create(const CapsuleShapeDefinition* definition);

        /**
         * @brief Constructs a new `CapsuleShape`.
         *
         * @param[in] radius The capsule radius.
         * @param[in] halfHeight The capsule half height.
         */
        explicit CapsuleShape(AmReal32 radius, AmReal32 halfHeight);

        /**
         * @brief Gets the radius of the capsule shape.
         *
         * @return The capsule's radius.
         */
        [[nodiscard]] AmReal32 GetRadius() const;

        /**
         * @brief Gets the half height of the capsule shape.
         *
         * @return The capsule's half height.
         */
        [[nodiscard]] AmReal32 GetHalfHeight() const;

        /**
         * @brief Gets the diameter of the capsule shape.
         *
         * @return The capsule's diameter.
         */
        [[nodiscard]] AmReal32 GetDiameter() const;

        /**
         * @brief Gets the height of the capsule shape.
         *
         * @return The capsule's height.
         */
        [[nodiscard]] AmReal32 GetHeight() const;

        /**
         * @brief Sets the radius of the capsule shape.
         *
         * @param[in] radius The capsule's radius.
         */
        void SetRadius(AmReal32 radius);

        /**
         * @brief Sets the half height of the capsule shape.
         *
         * @param[in] halfHeight The capsule's half height.
         */
        void SetHalfHeight(AmReal32 halfHeight);

        /**
         * @brief Gets the shortest distance to the edge of this shape.
         *
         * @param[in] location The location from which calculate the distance.
         *
         * @return The shortest distance from the location to the edge
         * of this shape. If negative, the given location in outside the shape.
         */
        [[nodiscard]] AmReal32 GetShortestDistanceToEdge(const AmVec3& location) final;

        /**
         * @brief Checks if the given position is contained in this shape.
         *
         * @param[in] location The 3D position to check.
         *
         * @return `true` if the shape contains the given position, `false` otherwise.
         */
        [[nodiscard]] bool Contains(const AmVec3& location) final;

        /**
         * @brief Compares this shape with another shape for equality.
         *
         * @note Shapes are equal if they have the same dimensions, the position, and the same orientation.
         *
         * @param[in] other The other shape to compare with.
         *
         * @return `true` if the shapes are equal, `false` otherwise.
         */
        bool operator==(const CapsuleShape& other) const;

        /**
         * @brief Compares this shape with another shape for inequality.
         *
         * @note Shapes are equal if they have the same dimensions, the position, and the same orientation.
         *
         * @param[in] other The other shape to compare with.
         *
         * @return `false` if the shapes are equal, `true` otherwise.
         */
        bool operator!=(const CapsuleShape& other) const;

    private:
        void Update() override;

        AmReal32 _radius;
        AmReal32 _halfHeight;

        AmVec3 _a, _b;
    };

    /**
     * @brief A cone shape, defined by a radius and an height.
     *
     * @ingroup math
     */
    class AM_API_PUBLIC ConeShape : public Shape
    {
        friend class ConeZone;

    public:
        /**
         * @brief Creates a new ConeShape from a definition.
         *
         * @param[in] definition The definition of the shape generated
         * from a flatbuffer binary.
         *
         * @warning This method is intended for internal usage only.
         */
        static ConeShape* Create(const ConeShapeDefinition* definition);

        /**
         * @brief Constructs a new `ConeShape`.
         *
         * @param[in] radius The radius of the cone's base.
         * @param[in] height The height of the cone.
         */
        explicit ConeShape(AmReal32 radius, AmReal32 height);

        /**
         * @brief Gets the radius of the cone shape.
         *
         * @return The cone base's radius.
         */
        [[nodiscard]] AmReal32 GetRadius() const;

        /**
         * @brief Gets the diameter of the cone shape.
         *
         * @return The cone base's diameter.
         */
        [[nodiscard]] AmReal32 GetDiameter() const;

        /**
         * @brief Gets the height of the cone shape.
         *
         * @return The cone's height.
         */
        [[nodiscard]] AmReal32 GetHeight() const;

        /**
         * @brief Sets the radius of the cone shape.
         *
         * @param[in] radius The cone base's radius.
         */
        void SetRadius(AmReal32 radius);

        /**
         * @brief Sets the height of the cone shape.
         *
         * @param[in] height The cone's height.
         */
        void SetHeight(AmReal32 height);

        /**
         * @brief Gets the shortest distance to the edge of this shape.
         *
         * @param[in] location The location from which calculate the distance.
         *
         * @return The shortest distance from the location to the edge
         * of this shape. If negative, the given location in outside the shape.
         */
        [[nodiscard]] AmReal32 GetShortestDistanceToEdge(const AmVec3& location) final;

        /**
         * @brief Checks if the given position is contained in this shape.
         *
         * @param[in] location The 3D position to check.
         *
         * @return true if the shape contains the given position, false otherwise.
         */
        [[nodiscard]] bool Contains(const AmVec3& location) final;

        /**
         * @brief Compares this shape with another shape for equality.
         *
         * @note Shapes are equal if they have the same dimensions, the position, and the same orientation.
         *
         * @param[in] other The other shape to compare with.
         *
         * @return `true` if the shapes are equal, `false` otherwise.
         */
        bool operator==(const ConeShape& other) const;

        /**
         * @brief Compares this shape with another shape for inequality.
         *
         * @note Shapes are equal if they have the same dimensions, the position, and the same orientation.
         *
         * @param[in] other The other shape to compare with.
         *
         * @return `false` if the shapes are equal, `true` otherwise.
         */
        bool operator!=(const ConeShape& other) const;

    private:
        void Update() override;

        AmReal32 _radius;
        AmReal32 _height;
    };

    /**
     * @brief A sphere shape, defined by a radius.
     *
     * @ingroup math
     */
    class AM_API_PUBLIC SphereShape : public Shape
    {
        friend class SphereZone;

    public:
        /**
         * @brief Creates a new SphereShape from a definition.
         *
         * @param[in] definition The definition of the shape generated
         * from a flatbuffer binary.
         *
         * @warning This method is intended for internal usage only.
         */
        static SphereShape* Create(const SphereShapeDefinition* definition);

        /**
         * @brief Constructs a new `SphereShape`.
         *
         * @param[in] radius The sphere's radius.
         */
        explicit SphereShape(AmReal32 radius);

        /**
         * @brief Gets the radius of the sphere shape.
         *
         * @return The sphere's radius.
         */
        [[nodiscard]] AmReal32 GetRadius() const;

        /**
         * @brief Gets the diameter of the sphere shape.
         *
         * @return The sphere's diameter.
         */
        [[nodiscard]] AmReal32 GetDiameter() const;

        /**
         * @brief Sets the radius of the sphere shape.
         *
         * @param[in] radius The sphere's radius.
         */
        void SetRadius(AmReal32 radius);

        /**
         * @brief Gets the shortest distance to the edge of this shape.
         *
         * @param[in] location The location from which calculate the distance.
         *
         * @return The shortest distance from the location to the edge
         * of this shape. If negative, the given location in outside the shape.
         */
        [[nodiscard]] AmReal32 GetShortestDistanceToEdge(const AmVec3& location) final;

        /**
         * @brief Checks if the given position is contained in this shape.
         *
         * @param[in] location The 3D position to check.
         *
         * @return true if the shape contains the given position, false otherwise.
         */
        [[nodiscard]] bool Contains(const AmVec3& location) final;

        /**
         * @brief Compares this shape with another shape for equality.
         *
         * @note Shapes are equal if they have the same dimensions, the position, and the same orientation.
         *
         * @param[in] other The other shape to compare with.
         *
         * @return `true` if the shapes are equal, `false` otherwise.
         */
        bool operator==(const SphereShape& other) const;

        /**
         * @brief Compares this shape with another shape for inequality.
         *
         * @note Shapes are equal if they have the same dimensions, the position, and the same orientation.
         *
         * @param[in] other The other shape to compare with.
         *
         * @return `false` if the shapes are equal, `true` otherwise.
         */
        bool operator!=(const SphereShape& other) const;

    private:
        void Update() override;

        AmReal32 _radius;
    };

    /**
     * @brief A `Zone` built with an inner `BoxShape` and an outer `BoxShape`.
     *
     * @ingroup math
     */
    class AM_API_PUBLIC BoxZone : public Zone
    {
    public:
        /**
         * @brief Constructs a new `BoxZone`.
         *
         * @param[in] inner The inner `BoxShape`.
         * @param[in] outer The outer `BoxShape`.
         */
        BoxZone(BoxShape* inner, BoxShape* outer);

        /**
         * @inherit
         */
        [[nodiscard]] AmReal32 GetFactor(const AmVec3& position) final;
    };

    /**
     * @brief A `Zone` built with an inner `CapsuleShape` and an outer `CapsuleShape`.
     *
     * @ingroup math
     */
    class AM_API_PUBLIC CapsuleZone : public Zone
    {
    public:
        /**
         * @brief Constructs a new `CapsuleZone`.
         *
         * @param[in] inner The inner `CapsuleShape`.
         * @param[in] outer The outer `CapsuleShape`.
         */
        CapsuleZone(CapsuleShape* inner, CapsuleShape* outer);

        /**
         * @inherit
         */
        [[nodiscard]] AmReal32 GetFactor(const AmVec3& position) final;
    };

    /**
     * @brief A `Zone` built with an inner `ConeShape` and an outer `ConeShape`.
     *
     * @ingroup math
     */
    class AM_API_PUBLIC ConeZone : public Zone
    {
    public:
        /**
         * @brief Constructs a new `ConeZone`.
         *
         * @param[in] inner The inner `ConeShape`.
         * @param[in] outer The outer `ConeShape`.
         */
        ConeZone(ConeShape* inner, ConeShape* outer);

        /**
         * @inherit
         */
        [[nodiscard]] AmReal32 GetFactor(const AmVec3& position) final;
    };

    /**
     * @brief A `Zone` built with an inner `SphereShape` and an outer `SphereShape`.
     *
     * @ingroup math
     */
    class AM_API_PUBLIC SphereZone : public Zone
    {
    public:
        /**
         * @brief Constructs a new `SphereZone`.
         *
         * @param[in] inner The inner `SphereShape`.
         * @param[in] outer The outer `SphereShape`.
         */
        SphereZone(SphereShape* inner, SphereShape* outer);

        /**
         * @inherit
         */
        [[nodiscard]] AmReal32 GetFactor(const AmVec3& position) final;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_MATH_SHAPE_H
