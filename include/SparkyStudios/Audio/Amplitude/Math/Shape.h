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

#ifndef SS_AMPLITUDE_AUDIO_SHAPE_H
#define SS_AMPLITUDE_AUDIO_SHAPE_H

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
     * @brief A Shape.
     *
     * A Shape define a zone in the world where listeners and sound sources can be localized. This is used by the
     * engine to detect the position on these objects and apply a specific effect (environmental effect or
     * attenuation effect) to them.
     */
    class Shape
    {
    public:
        /**
         * @brief Creates a new Shape from a definition.
         *
         * @param definition The definition of the shape generated
         *                   from a flatbuffer binary.
         */
        static Shape* Create(const ShapeDefinition* definition);

        /**
         * @brief Construct a new Shape.
         */
        Shape();

        /**
         * @brief Get the shortest distance to the edge of this shape.
         *
         * @param entity The entity from which calculate the distance.
         *
         * @return The shortest distance from the location to the edge
         * of this shape. If negative, the given location in outside the shape.
         */
        [[nodiscard]] virtual AmReal32 GetShortestDistanceToEdge(const Entity& entity);

        /**
         * @brief Get the shortest distance to the edge of this shape.
         *
         * @param listener The listener from which calculate the distance.
         *
         * @return The shortest distance from the location to the edge
         * of this shape. If negative, the given location in outside the shape.
         */
        [[nodiscard]] virtual AmReal32 GetShortestDistanceToEdge(const Listener& listener);

        /**
         * @brief Get the shortest distance to the edge of this shape.
         *
         * @param location The location from which calculate the distance.
         *
         * @return The shortest distance from the location to the edge
         * of this shape. If negative, the given location in outside the shape.
         */
        [[nodiscard]] virtual AmReal32 GetShortestDistanceToEdge(const AmVec3& location) = 0;

        /**
         * @brief Checks if the given entity is contained in this shape.
         *
         * @param entity The entity to check.
         *
         * @return true if the shape contains the entity, false otherwise.
         */
        [[nodiscard]] virtual bool Contains(const Entity& entity);

        /**
         * @brief Checks if the given listener is contained in this shape.
         *
         * @param listener The listener to check.
         *
         * @return true if the shape contains the listener, false otherwise.
         */
        [[nodiscard]] virtual bool Contains(const Listener& listener);

        /**
         * @brief Checks if the given position is contained in this shape.
         *
         * @param location The 3D position to check.
         *
         * @return true if the shape contains the given position, false otherwise.
         */
        [[nodiscard]] virtual bool Contains(const AmVec3& location) = 0;

        /**
         * @brief Set the location of this shape in the 3D environment.
         *
         * @param location The shape location.
         */
        void SetLocation(const AmVec3& location);

        /**
         * @brief Set the orientation of this shape.
         *
         * @param direction The shape direction.
         * @param up The shape up vector.
         */
        void SetOrientation(const AmVec3& direction, const AmVec3& up);

        /**
         * @brief Get the LookAt transformation matrix for this shape.
         *
         * @return The lookAt transformation matrix.
         */
        [[nodiscard]] const AmMat4& GetLookAt() const;

        /**
         * @brief Get the position of this shape in the 3D environment.
         *
         * @return The shape's position.
         */
        [[nodiscard]] const AmVec3& GetLocation() const;

        /**
         * @brief Get the position of this shape in the 3D environment.
         *
         * @return The shape's position.
         */
        [[nodiscard]] const AmVec3& GetDirection() const;

        /**
         * @brief Get the up vector of the zone.
         *
         * @return The up vector.
         */
        [[nodiscard]] const AmVec3& GetUp() const;

    protected:
        AmVec3 m_location;
        AmVec3 m_direction;
        AmVec3 m_up;

        AmMat4 m_lookAtMatrix;

        bool m_needUpdate;
    };

    /**
     * @brief A shape that represents a zone in the world.
     *
     * This shape is mainly used by attenuations and environments. It's composed of an inner shape and an outer shape.
     * The inner shape is the zone where the factor is equal to one all the time. The outer shape is the zone where the
     * factor increase or decrease according to the shortest distance of the game object from the outer edge.
     *
     * If the game object is outside the outer shape (thus, outside the zone), the factor is zero.
     */
    class Zone
    {
    public:
        explicit Zone(Shape* inner, Shape* outer);
        virtual ~Zone() = default;

        /**
         * @brief Get the factor (a value in the range [0, 1]) according to the position
         * of the given entity in the zone.
         *
         * @param entity The entity to get the factor for.
         *
         * @return The factor.
         */
        [[nodiscard]] virtual AmReal32 GetFactor(const Entity& entity)
        {
            return GetFactor(entity.GetLocation());
        }

        /**
         * @brief Get the factor (a value in the range [0, 1]) according to the position
         * of the given listener in the zone.
         *
         * @param listener The listener to get the factor for.
         *
         * @return The factor.
         */
        [[nodiscard]] virtual AmReal32 GetFactor(const Listener& listener)
        {
            return GetFactor(listener.GetLocation());
        }

        /**
         * @brief Get the factor (a value in the range [0, 1]) according to the given
         * position in the zone.
         *
         * @param position The position in the zone to get the factor for.
         *
         * @return The factor.
         */
        [[nodiscard]] virtual AmReal32 GetFactor(const AmVec3& position) = 0;

        /**
         * @brief Set the location of this zone in the 3D environment.
         *
         * @param location The zone location.
         */
        void SetLocation(const AmVec3& location);

        /**
         * @brief Gets the current location of this zone.
         *
         * @return The current location of this zone.
         */
        [[nodiscard]] const AmVec3& GetLocation() const;

        /**
         * @brief Set the orientation of this shape.
         *
         * @param direction The shape direction.
         * @param up The shape up vector.
         */
        void SetOrientation(const AmVec3& direction, const AmVec3& up);

        /**
         * @brief Get the direction vector of the zone.
         *
         * @return The direction vector.
         */
        [[nodiscard]] const AmVec3& GetDirection() const;

        /**
         * @brief Get the up vector of the zone.
         *
         * @return The up vector.
         */
        [[nodiscard]] const AmVec3& GetUp() const;

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
     */
    class BoxShape : public Shape
    {
        friend class BoxZone;

    public:
        /**
         * @brief Creates a new BoxShape from a definition.
         *
         * @param definition The definition of the box shape generated
         *                   from a flatbuffer binary.
         */
        static BoxShape* Create(const BoxShapeDefinition* definition);

        /**
         * @brief Construct a new Box Shape.
         *
         * @param halfWidth The half width of the box shape.
         * @param halfHeight The half height of the box shape.
         * @param halfDepth The half depth of the box shape.
         */
        explicit BoxShape(AmReal32 halfWidth, AmReal32 halfHeight, AmReal32 halfDepth);

        /**
         * @brief Get the half width of the box shape.
         *
         * @return The box shape's half width.
         */
        [[nodiscard]] AmReal32 GetHalfWidth() const;

        /**
         * @brief Get the half height of the box shape.
         *
         * @return The box shape's half height.
         */
        [[nodiscard]] AmReal32 GetHalfHeight() const;

        /**
         * @brief Get the half depth of the box shape.
         *
         * @return The box shape's half depth.
         */
        [[nodiscard]] AmReal32 GetHalfDepth() const;

        /**
         * @brief Get the width of the box shape.
         *
         * @return The box shape's width.
         */
        [[nodiscard]] AmReal32 GetWidth() const;

        /**
         * @brief Get the height of the box shape.
         *
         * @return The box shape's height.
         */
        [[nodiscard]] AmReal32 GetHeight() const;

        /**
         * @brief Get the depth of the box shape.
         *
         * @return The box shape's depth.
         */
        [[nodiscard]] AmReal32 GetDepth() const;

        /**
         * @brief Set the half width of the box shape.
         *
         * @param halfWidth The new box shape's half width.
         */
        void SetHalfWidth(AmReal32 halfWidth);

        /**
         * @brief Set the half height of the box shape.
         *
         * @param halfHeight The new box shape's half height.
         */
        void SetHalfHeight(AmReal32 halfHeight);

        /**
         * @brief Set the half depth of the box shape.
         *
         * @param halfDepth The new box shape's half depth.
         */
        void SetHalfDepth(AmReal32 halfDepth);

        /**
         * @brief Get the shortest distance to the edge of this shape.
         *
         * @param location The location from which calculate the distance.
         *
         * @return The shortest distance from the location to the edge
         * of this shape. If negative, the given location in outside the shape.
         */
        [[nodiscard]] AmReal32 GetShortestDistanceToEdge(const AmVec3& location) final;

        /**
         * @brief Checks if the given position is contained in this shape.
         *
         * @param location The 3D position to check.
         *
         * @return true if the shape contains the given position, false otherwise.
         */
        [[nodiscard]] bool Contains(const AmVec3& location) final;

    private:
        void _update();

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
     */
    class CapsuleShape : public Shape
    {
        friend class CapsuleZone;

    public:
        /**
         * @brief Creates a new CapsuleShape from a definition.
         *
         * @param definition The definition of the capsule shape generated
         *                   from a flatbuffer binary.
         */
        static CapsuleShape* Create(const CapsuleShapeDefinition* definition);

        /**
         * @brief Construct a new Capsule Shape.
         *
         * @param radius The capsule radius.
         * @param halfHeight The capsule half height.
         */
        explicit CapsuleShape(AmReal32 radius, AmReal32 halfHeight);

        /**
         * @brief Get the radius of the capsule shape.
         *
         * @return The capsule's radius.
         */
        [[nodiscard]] AmReal32 GetRadius() const;

        /**
         * @brief Get the half height of the capsule shape.
         *
         * @return The capsule's half height.
         */
        [[nodiscard]] AmReal32 GetHalfHeight() const;

        /**
         * @brief Get the diameter of the capsule shape.
         *
         * @return The capsule's diameter.
         */
        [[nodiscard]] AmReal32 GetDiameter() const;

        /**
         * @brief Get the height of the capsule shape.
         *
         * @return The capsule's height.
         */
        [[nodiscard]] AmReal32 GetHeight() const;

        /**
         * @brief Set the radius of the capsule shape.
         *
         * @param radius The capsule's radius.
         */
        void SetRadius(AmReal32 radius);

        /**
         * @brief Set the half height of the capsule shape.
         *
         * @param halfHeight The capsule's half height.
         */
        void SetHalfHeight(AmReal32 halfHeight);

        /**
         * @brief Get the shortest distance to the edge of this shape.
         *
         * @param location The location from which calculate the distance.
         *
         * @return The shortest distance from the location to the edge
         * of this shape. If negative, the given location in outside the shape.
         */
        [[nodiscard]] AmReal32 GetShortestDistanceToEdge(const AmVec3& location) final;

        /**
         * @brief Checks if the given position is contained in this shape.
         *
         * @param location The 3D position to check.
         *
         * @return true if the shape contains the given position, false otherwise.
         */
        [[nodiscard]] bool Contains(const AmVec3& location) final;

    private:
        void Update();

        AmReal32 _radius;
        AmReal32 _halfHeight;

        AmVec3 _a, _b;
    };

    /**
     * @brief A cone shape, defined by a radius and an height.
     */
    class ConeShape : public Shape
    {
        friend class ConeZone;

    public:
        /**
         * @brief Creates a new ConeShape from a definition.
         *
         * @param definition The definition of the cone shape generated
         *                   from a flatbuffer binary.
         */
        static ConeShape* Create(const ConeShapeDefinition* definition);

        /**
         * @brief Construct a new Cone Shape.
         *
         * @param radius The radius of the cone's base.
         * @param height The height of the cone.
         */
        explicit ConeShape(AmReal32 radius, AmReal32 height);

        /**
         * @brief Get the radius of the cone shape.
         *
         * @return The cone base's radius.
         */
        [[nodiscard]] AmReal32 GetRadius() const;

        /**
         * @brief Get the diameter of the cone shape.
         *
         * @return The cone base's diameter.
         */
        [[nodiscard]] AmReal32 GetDiameter() const;

        /**
         * @brief Get the height of the cone shape.
         *
         * @return The cone's height.
         */
        [[nodiscard]] AmReal32 GetHeight() const;

        /**
         * @brief Set the radius of the cone shape.
         *
         * @param radius The cone base's radius.
         */
        void SetRadius(AmReal32 radius);

        /**
         * @brief Set the height of the cone shape.
         *
         * @param height The cone's height.
         */
        void SetHeight(AmReal32 height);

        /**
         * @brief Get the shortest distance to the edge of this shape.
         *
         * @param location The location from which calculate the distance.
         *
         * @return The shortest distance from the location to the edge
         * of this shape. If negative, the given location in outside the shape.
         */
        [[nodiscard]] AmReal32 GetShortestDistanceToEdge(const AmVec3& location) final;

        /**
         * @brief Checks if the given position is contained in this shape.
         *
         * @param location The 3D position to check.
         *
         * @return true if the shape contains the given position, false otherwise.
         */
        [[nodiscard]] bool Contains(const AmVec3& location) final;

    private:
        void Update();

        AmReal32 _radius;
        AmReal32 _height;
    };

    /**
     * @brief A sphere shape, defined by a radius.
     */
    class SphereShape : public Shape
    {
        friend class SphereZone;

    public:
        /**
         * @brief Creates a new SphereShape from a definition.
         *
         * @param definition The definition of the sphere shape generated
         *                   from a flatbuffer binary.
         */
        static SphereShape* Create(const SphereShapeDefinition* definition);

        /**
         * @brief Construct a new Sphere Shape.
         *
         * @param radius The sphere's radius.
         */
        explicit SphereShape(AmReal32 radius);

        /**
         * @brief Get the radius of the sphere shape.
         *
         * @return The sphere's radius.
         */
        [[nodiscard]] AmReal32 GetRadius() const;

        /**
         * @brief Get the diameter of the sphere shape.
         *
         * @return The sphere's diameter.
         */
        [[nodiscard]] AmReal32 GetDiameter() const;

        /**
         * @brief Set the radius of the sphere shape.
         *
         * @param radius The sphere's radius.
         */
        void SetRadius(AmReal32 radius);

        /**
         * @brief Get the shortest distance to the edge of this shape.
         *
         * @param location The location from which calculate the distance.
         *
         * @return The shortest distance from the location to the edge
         * of this shape. If negative, the given location in outside the shape.
         */
        [[nodiscard]] AmReal32 GetShortestDistanceToEdge(const AmVec3& location) final;

        /**
         * @brief Checks if the given position is contained in this shape.
         *
         * @param location The 3D position to check.
         *
         * @return true if the shape contains the given position, false otherwise.
         */
        [[nodiscard]] bool Contains(const AmVec3& location) final;

    private:
        void Update();

        AmReal32 _radius;
    };

    class BoxZone : public Zone
    {
    public:
        BoxZone(BoxShape* inner, BoxShape* outer);

        [[nodiscard]] AmReal32 GetFactor(const AmVec3& position) final;
    };

    class CapsuleZone : public Zone
    {
    public:
        CapsuleZone(CapsuleShape* inner, CapsuleShape* outer);

        [[nodiscard]] AmReal32 GetFactor(const AmVec3& position) final;
    };

    class ConeZone : public Zone
    {
    public:
        ConeZone(ConeShape* inner, ConeShape* outer);

        [[nodiscard]] AmReal32 GetFactor(const AmVec3& position) final;
    };

    class SphereZone : public Zone
    {
    public:
        SphereZone(SphereShape* inner, SphereShape* outer);

        [[nodiscard]] AmReal32 GetFactor(const AmVec3& position) final;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_SHAPE_H
