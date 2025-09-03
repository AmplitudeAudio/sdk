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

#include <memory>

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
     * A @c Shape defines a zone in the world where game objects (listener, sound sources, entities, etc.) can
     * be localized. Shapes are used in many places of the engine, like to define a sound attenuation shape or to build a room.
     *
     * @ingroup math
     */
    class AM_API_PUBLIC Shape
    {
    public:
        /**
         * @brief Creates a new @c Shape from a definition.
         *
         * @param[in] definition The definition of the shape generated from a flatbuffer binary.
         *
         * @warning This method is intended for internal usage only.
         */
        static std::shared_ptr<Shape> Create(const ShapeDefinition* definition);

        /**
         * @brief Constructs a new @c Shape.
         */
        Shape();

        /**
         * @brief Default destructor.
         */
        virtual ~Shape() = default;

        /**
         * @brief Gets the shortest distance to the edge of this shape.
         *
         * @param[in] entity The entity used to calculate the distance.
         *
         * @return The shortest distance from the entity location to the edge
         * of this shape. If negative, the given entity in outside the shape.
         */
        [[nodiscard]] virtual AmReal32 GetShortestDistanceToEdge(const Entity& entity);

        /**
         * @brief Gets the shortest distance to the edge of this shape.
         *
         * @param[in] listener The listener used to calculate the distance.
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
        [[nodiscard]] virtual AmReal32 GetShortestDistanceToEdge(const AmVector3& location) = 0;

        /**
         * @brief Checks if the given entity is contained in this shape.
         *
         * @param[in] entity The entity to check.
         *
         * @return @c true if the shape contains the entity, @c false otherwise.
         */
        [[nodiscard]] virtual bool Contains(const Entity& entity);

        /**
         * @brief Checks if the given listener is contained in this shape.
         *
         * @param[in] listener The listener to check.
         *
         * @return @c true if the shape contains the listener, @c false otherwise.
         */
        [[nodiscard]] virtual bool Contains(const Listener& listener);

        /**
         * @brief Checks if the given position is contained in this shape.
         *
         * @param[in] location The 3D position to check.
         *
         * @return @c true if the shape contains the given position, @c false otherwise.
         */
        [[nodiscard]] virtual bool Contains(const AmVector3& location) = 0;

        /**
         * @brief Sets the location of this shape in the 3D environment.
         *
         * @param[in] location The shape location.
         */
        void SetLocation(const AmVector3& location);

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
        [[nodiscard]] const AmMatrix4& GetLookAt() const;

        /**
         * @brief Gets the position of this shape in the 3D environment.
         *
         * @return The shape's position.
         */
        [[nodiscard]] const AmVector3& GetLocation() const;

        /**
         * @brief Gets the position of this shape in the 3D environment.
         *
         * @return The shape's position.
         */
        [[nodiscard]] AmVector3 GetDirection() const;

        /**
         * @brief Gets the up vector of the zone.
         *
         * @return The up vector.
         */
        [[nodiscard]] AmVector3 GetUp() const;

    protected:
        /**
         * @brief Updates the internal state of the shape.
         *
         * This pure virtual function must be implemented by derived classes to
         * perform custom updates whenever modifications occur, such as
         * changes to the shape's position, orientation, or other parameters.
         * It is intended to synchronize the internal data or perform
         * computations necessary for the shape's functionality.
         *
         * @note The engine calls this method internally when parameters like location or orientation are modified.
         */
        virtual void Update() = 0;

        /**
         * @brief Represents the 3D position of an object in space.
         */
        AmVector3 m_location;

        /**
         * @brief Represents the orientation of an object in 3D space.
         */
        Orientation m_orientation;

        /**
         * @brief The look-at matrix representing a transformation in 3D space.
         */
        AmMatrix4 m_lookAtMatrix;

        /**
         * @brief Indicates whether an update is required.
         */
        bool m_needUpdate;
    };

    /**
     * @brief A tuple of shapes that represents a zone in the world.
     *
     * This shape is mainly used by attenuation and environments. It's composed of an inner @c Shape and an outer @c Shape.
     * The inner shape is the place where the @ref GetFactor "factor" is equal to one all the time. The outer shape is the place where the
     * @ref GetFactor "factor" increases or decreases according to the shortest distance of the game object from the outer edge.
     *
     * If the game object is outside the outer shape (thus, outside the zone), the @ref GetFactor "factor" is zero.
     *
     * @ingroup math
     */
    class AM_API_PUBLIC Zone
    {
    public:
        /**
         * @brief Creates a new zone from the given inner and outer shapes.
         *
         * @param[in] inner The inner shape.
         * @param[in] outer The outer shape.
         */
        explicit Zone(std::shared_ptr<Shape> inner, std::shared_ptr<Shape> outer);

        /**
         * @brief Default destructor.
         */
        virtual ~Zone();

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
        [[nodiscard]] virtual AmReal32 GetFactor(const AmVector3& position) = 0;

        /**
         * @brief Sets the location of this zone in the 3D environment.
         *
         * @param[in] location The zone location.
         */
        void SetLocation(const AmVector3& location);

        /**
         * @brief Gets the current location of this zone.
         *
         * @return The current location of this zone.
         */
        [[nodiscard]] const AmVector3& GetLocation() const;

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
        [[nodiscard]] AmVector3 GetDirection() const;

        /**
         * @brief Gets the up vector of the zone.
         *
         * @return The up vector.
         */
        [[nodiscard]] AmVector3 GetUp() const;

    protected:
        /**
         * @brief The inner shape of the zone.
         */
        std::shared_ptr<Shape> m_innerShape;

        /**
         * @brief The outer shape of the zone.
         */
        std::shared_ptr<Shape> m_outerShape;
    };

    /**
     * @brief A box shape, defined by a width, a height, and a depth.
     *
     * @ingroup math
     */
    class AM_API_PUBLIC BoxShape final : public Shape
    {
        friend class BoxZone;

    public:
        /**
         * @brief Creates a new box shape from a flatbuffer definition.
         *
         * @param[in] definition The definition of the shape generated
         * from a flatbuffer binary.
         *
         * @warning This method is intended for internal usage only.
         */
        static std::shared_ptr<BoxShape> Create(const BoxShapeDefinition* definition);

        /**
         * @brief Constructs a new box shape.
         *
         * @param[in] halfWidth The half-width of the box shape.
         * @param[in] halfHeight The half-height of the box shape.
         * @param[in] halfDepth The half-depth of the box shape.
         */
        explicit BoxShape(AmReal32 halfWidth, AmReal32 halfHeight, AmReal32 halfDepth);

        /**
         * @brief Constructs a new box shape.
         *
         * @param[in] position The position of the box shape.
         * @param[in] dimensions The dimensions of the box shape.
         */
        explicit BoxShape(const AmVector3& position, const AmVector3& dimensions);

        /**
         * @brief Gets the half-width of the box shape.
         *
         * @return The box shape's half-width.
         */
        [[nodiscard]] AmReal32 GetHalfWidth() const;

        /**
         * @brief Gets the half-height of the box shape.
         *
         * @return The box shape's half-height.
         */
        [[nodiscard]] AmReal32 GetHalfHeight() const;

        /**
         * @brief Gets the half-depth of the box shape.
         *
         * @return The box shape's half-depth.
         */
        [[nodiscard]] AmReal32 GetHalfDepth() const;

        /**
         * @brief Gets the width of the box shape.
         *
         * @return The box shape's width.
         */
        [[nodiscard]] AmReal32 GetWidth() const;

        /**
         * @brief Gets the height of the box shape.
         *
         * @return The box shape's height.
         */
        [[nodiscard]] AmReal32 GetHeight() const;

        /**
         * @brief Gets the depth of the box shape.
         *
         * @return The box shape's depth.
         */
        [[nodiscard]] AmReal32 GetDepth() const;

        /**
         * @brief Sets the half-width of the box shape.
         *
         * @param[in] halfWidth The new box shape's half-width.
         */
        void SetHalfWidth(AmReal32 halfWidth);

        /**
         * @brief Sets the half-height of the box shape.
         *
         * @param[in] halfHeight The new box shape's half-height.
         */
        void SetHalfHeight(AmReal32 halfHeight);

        /**
         * @brief Sets the half-depth of the box shape.
         *
         * @param[in] halfDepth The new box shape's half-depth.
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
        [[nodiscard]] AmReal32 GetShortestDistanceToEdge(const AmVector3& location) override;

        /**
         * @brief Checks if the given position is contained in this shape.
         *
         * @param[in] location The 3D position to check.
         *
         * @return @c true if the shape contains the given position, @c false otherwise.
         */
        [[nodiscard]] bool Contains(const AmVector3& location) override;

        /**
         * @brief Gets the closest point to the given location.
         *
         * @param[in] location The location to get the closest point for.
         *
         * @return The closest point to the given location.
         */
        [[nodiscard]] AmVector3 GetClosestPoint(const AmVector3& location) const;

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
        [[nodiscard]] std::array<AmVector3, 8> GetCorners() const;

        /**
         * @brief Compares this shape with another shape for equality.
         *
         * @note Shapes are equal if they have the same dimensions, the position, and the same orientation.
         *
         * @param[in] other The other shape to compare with.
         *
         * @return @c true if the shapes are equal, @c false otherwise.
         */
        bool operator==(const BoxShape& other) const;

        /**
         * @brief Compares this shape with another shape for inequality.
         *
         * @note Shapes are equal if they have the same dimensions, the position, and the same orientation.
         *
         * @param[in] other The other shape to compare with.
         *
         * @return @c false if the shapes are equal, @c true otherwise.
         */
        bool operator!=(const BoxShape& other) const;

    private:
        /**
         * @brief Updates the transformed parameters of the box shape.
         *
         * This method recalculates the corner points and direction vectors of the box
         * shape based on the current transformation matrix. It ensures that the box shape
         * is updated with the latest scale, position, orientation, or any other transformation
         * applied. It also internally updates the state to avoid redundant updates.
         *
         * This function is primarily used internally and is called when any transformation
         * influencing the box shape has changed.
         */
        void Update() override;

        AmReal32 _halfWidth;
        AmReal32 _halfHeight;
        AmReal32 _halfDepth;

        AmVector3 _u;
        AmVector3 _v;
        AmVector3 _w;

        AmVector3 _p1, _p2, _p3, _p4;
        AmReal32 _uP1, _vP1, _wP1, _uP2, _vP3, _wP4;
    };

    /**
     * @brief A capsule shape, defined by a radius and a height.
     *
     * @ingroup math
     */
    class AM_API_PUBLIC CapsuleShape final : public Shape
    {
        friend class CapsuleZone;

    public:
        /**
         * @brief Creates a new capsule shape from a flatbuffer definition.
         *
         * @param[in] definition The definition of the shape generated
         * from a flatbuffer binary.
         *
         * @warning This method is intended for internal usage only.
         */
        static std::shared_ptr<CapsuleShape> Create(const CapsuleShapeDefinition* definition);

        /**
         * @brief Constructs a new capsule shape.
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
         * @brief Gets the half-height of the capsule shape.
         *
         * @return The capsule's half-height.
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
         * @brief Sets the half-height of the capsule shape.
         *
         * @param[in] halfHeight The capsule's half-height.
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
        [[nodiscard]] AmReal32 GetShortestDistanceToEdge(const AmVector3& location) override;

        /**
         * @brief Checks if the given position is contained in this shape.
         *
         * @param[in] location The 3D position to check.
         *
         * @return @c true if the shape contains the given position, @c false otherwise.
         */
        [[nodiscard]] bool Contains(const AmVector3& location) override;

        /**
         * @brief Compares this shape with another shape for equality.
         *
         * @note Shapes are equal if they have the same dimensions, the position, and the same orientation.
         *
         * @param[in] other The other shape to compare with.
         *
         * @return @c true if the shapes are equal, @c false otherwise.
         */
        bool operator==(const CapsuleShape& other) const;

        /**
         * @brief Compares this shape with another shape for inequality.
         *
         * @note Shapes are equal if they have the same dimensions, the position, and the same orientation.
         *
         * @param[in] other The other shape to compare with.
         *
         * @return @c false if the shapes are equal, @c true otherwise.
         */
        bool operator!=(const CapsuleShape& other) const;

    private:
        /**
         * @brief Updates the capsule shape's key spatial points.
         *
         * Recalculates the top and bottom cap positions of the capsule based on its current orientation,
         * radius, and half-height. This ensures the capsule shape remains consistent with its transformations.
         */
        void Update() override;

        AmReal32 _radius;
        AmReal32 _halfHeight;

        AmVector3 _a, _b;
    };

    /**
     * @brief A cone shape, defined by a radius and a height.
     *
     * @ingroup math
     */
    class AM_API_PUBLIC ConeShape final : public Shape
    {
        friend class ConeZone;

    public:
        /**
         * @brief Creates a new cone shape from a definition.
         *
         * @param[in] definition The definition of the shape generated
         * from a flatbuffer binary.
         *
         * @warning This method is intended for internal usage only.
         */
        static std::shared_ptr<ConeShape> Create(const ConeShapeDefinition* definition);

        /**
         * @brief Constructs a new cone shape.
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
        [[nodiscard]] AmReal32 GetShortestDistanceToEdge(const AmVector3& location) override;

        /**
         * @brief Checks if the given position is contained in this shape.
         *
         * @param[in] location The 3D position to check.
         *
         * @return true if the shape contains the given position, false otherwise.
         */
        [[nodiscard]] bool Contains(const AmVector3& location) override;

        /**
         * @brief Compares this shape with another shape for equality.
         *
         * @note Shapes are equal if they have the same dimensions, the position, and the same orientation.
         *
         * @param[in] other The other shape to compare with.
         *
         * @return @c true if the shapes are equal, @c false otherwise.
         */
        bool operator==(const ConeShape& other) const;

        /**
         * @brief Compares this shape with another shape for inequality.
         *
         * @note Shapes are equal if they have the same dimensions, the position, and the same orientation.
         *
         * @param[in] other The other shape to compare with.
         *
         * @return @c false if the shapes are equal, @c true otherwise.
         */
        bool operator!=(const ConeShape& other) const;

    private:
        /**
         * @brief Updates the internal state of the cone shape.
         *
         * This method recalculates and synchronizes the internal state of the cone shape if required.
         * It is invoked when changes to the shape's properties or other dependent attributes
         * require an update to its internal representation.
         *
         * @note This function is called automatically when needed and should typically not require direct invocation.
         */
        void Update() override;

        AmReal32 _radius;
        AmReal32 _height;
    };

    /**
     * @brief A sphere shape, defined by a radius.
     *
     * @ingroup math
     */
    class AM_API_PUBLIC SphereShape final : public Shape
    {
        friend class SphereZone;

    public:
        /**
         * @brief Creates a new sphere shape from a definition.
         *
         * @param[in] definition The definition of the shape generated
         * from a flatbuffer binary.
         *
         * @warning This method is intended for internal usage only.
         */
        static std::shared_ptr<SphereShape> Create(const SphereShapeDefinition* definition);

        /**
         * @brief Constructs a new sphere shape.
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
        [[nodiscard]] AmReal32 GetShortestDistanceToEdge(const AmVector3& location) override;

        /**
         * @brief Checks if the given position is contained in this shape.
         *
         * @param[in] location The 3D position to check.
         *
         * @return true if the shape contains the given position, false otherwise.
         */
        [[nodiscard]] bool Contains(const AmVector3& location) override;

        /**
         * @brief Compares this shape with another shape for equality.
         *
         * @note Shapes are equal if they have the same dimensions, the position, and the same orientation.
         *
         * @param[in] other The other shape to compare with.
         *
         * @return @c true if the shapes are equal, @c false otherwise.
         */
        bool operator==(const SphereShape& other) const;

        /**
         * @brief Compares this shape with another shape for inequality.
         *
         * @note Shapes are equal if they have the same dimensions, the position, and the same orientation.
         *
         * @param[in] other The other shape to compare with.
         *
         * @return @c false if the shapes are equal, @c true otherwise.
         */
        bool operator!=(const SphereShape& other) const;

    private:
        /**
         * @brief Updates the internal state of the sphere shape.
         *
         * Marks the sphere as up to date by resetting any pending updates.
         * This method ensures that the shape is in a consistent state and
         * that all dependent calculations or operations use the latest data.
         */
        void Update() override;

        AmReal32 _radius;
    };

    /**
     * @brief A @c Zone built with an inner @c BoxShape and an outer @c BoxShape.
     *
     * @ingroup math
     */
    class AM_API_PUBLIC BoxZone : public Zone
    {
    public:
        /**
         * @brief Constructs a new box zone.
         *
         * @param[in] inner The inner @c BoxShape.
         * @param[in] outer The outer @c BoxShape.
         *
         * @see BoxShape
         */
        BoxZone(std::shared_ptr<BoxShape> inner, std::shared_ptr<BoxShape> outer);

        /**
         * @inherit
         */
        [[nodiscard]] AmReal32 GetFactor(const AmVector3& position) final;
    };

    /**
     * @brief A @c Zone built with an inner @c CapsuleShape and an outer @c CapsuleShape.
     *
     * @ingroup math
     */
    class AM_API_PUBLIC CapsuleZone : public Zone
    {
    public:
        /**
         * @brief Constructs a new capsule zone.
         *
         * @param[in] inner The inner @c CapsuleShape.
         * @param[in] outer The outer @c CapsuleShape.
         *
         * @see CapsuleShape
         */
        CapsuleZone(std::shared_ptr<CapsuleShape> inner, std::shared_ptr<CapsuleShape> outer);

        /**
         * @inherit
         */
        [[nodiscard]] AmReal32 GetFactor(const AmVector3& position) final;
    };

    /**
     * @brief A @c Zone built with an inner @c ConeShape and an outer @c ConeShape.
     *
     * @ingroup math
     */
    class AM_API_PUBLIC ConeZone : public Zone
    {
    public:
        /**
         * @brief Constructs a new cone zone.
         *
         * @param[in] inner The inner @c ConeShape.
         * @param[in] outer The outer @c ConeShape.
         *
         * @see ConeShape
         */
        ConeZone(std::shared_ptr<ConeShape> inner, std::shared_ptr<ConeShape> outer);

        /**
         * @inherit
         */
        [[nodiscard]] AmReal32 GetFactor(const AmVector3& position) final;
    };

    /**
     * @brief A @c Zone built with an inner @c SphereShape and an outer @c SphereShape.
     *
     * @ingroup math
     */
    class AM_API_PUBLIC SphereZone : public Zone
    {
    public:
        /**
         * @brief Constructs a new sphere zone.
         *
         * @param[in] inner The inner @c SphereShape.
         * @param[in] outer The outer @c SphereShape.
         *
         * @see SphereShape
         */
        SphereZone(std::shared_ptr<SphereShape> inner, std::shared_ptr<SphereShape> outer);

        /**
         * @inherit
         */
        [[nodiscard]] AmReal32 GetFactor(const AmVector3& position) final;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_MATH_SHAPE_H
