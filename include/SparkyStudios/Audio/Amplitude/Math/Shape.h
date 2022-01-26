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
        [[nodiscard]] virtual bool Contains(const hmm_vec3& location) = 0;

        /**
         * @brief Set the location of this shape in the 3D environment.
         *
         * @param location The shape location.
         */
        void SetPosition(const hmm_vec3& location);

        /**
         * @brief Set the orientation of this shape.
         *
         * @param direction The shape direction.
         * @param up The shape up vector.
         */
        void SetOrientation(const hmm_vec3& direction, const hmm_vec3& up);

        /**
         * @brief Get the LookAt transformation matrix for this shape.
         *
         * @return The lookAt transformation matrix.
         */
        [[nodiscard]] const hmm_mat4& GetLookAt() const;

    protected:
        hmm_vec3 m_location;
        hmm_vec3 m_direction;
        hmm_vec3 m_up;

        hmm_mat4 m_lookAtMatrix;

        bool m_needUpdate;
    };

    /**
     * @brief A box shape, defined by a width, an height, and a depth.
     */
    class BoxShape : public Shape
    {
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
         * @param halfWidth The new box shape's half height.
         */
        void SetHalfHeight(AmReal32 halfHeight);

        /**
         * @brief Set the half depth of the box shape.
         *
         * @param halfWidth The new box shape's half depth.
         */
        void SetHalfDepth(AmReal32 halfDepth);

        /**
         * @brief Checks if the given position is contained in this shape.
         *
         * @param location The 3D position to check.
         *
         * @return true if the shape contains the given position, false otherwise.
         */
        [[nodiscard]] bool Contains(const hmm_vec3& location) final;

    private:
        AmReal32 _halfWidth;
        AmReal32 _halfHeight;
        AmReal32 _halfDepth;

        hmm_vec3 _u;
        hmm_vec3 _v;
        hmm_vec3 _w;

        AmReal32 _uP1, _vP1, _wP1, _uP2, _vP3, _wP4;
    };

    /**
     * @brief A capsule shape, defined by a radius and an height.
     */
    class CapsuleShape : public Shape
    {
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
         * @brief Checks if the given position is contained in this shape.
         *
         * @param location The 3D position to check.
         *
         * @return true if the shape contains the given position, false otherwise.
         */
        [[nodiscard]] bool Contains(const hmm_vec3& location) final;

    private:
        AmReal32 _radius;
        AmReal32 _halfHeight;
    };

    /**
     * @brief A cone shape, defined by a radius and an height.
     */
    class ConeShape : public Shape
    {
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
         * @brief Checks if the given position is contained in this shape.
         *
         * @param location The 3D position to check.
         *
         * @return true if the shape contains the given position, false otherwise.
         */
        [[nodiscard]] bool Contains(const hmm_vec3& location) final;

    private:
        AmReal32 _radius;
        AmReal32 _height;
    };

    /**
     * @brief A sphere shape, defined by a radius.
     */
    class SphereShape : public Shape
    {
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
         * @brief Checks if the given position is contained in this shape.
         *
         * @param location The 3D position to check.
         *
         * @return true if the shape contains the given position, false otherwise.
         */
        [[nodiscard]] bool Contains(const hmm_vec3& location) final;

    private:
        AmReal32 _radius;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_SHAPE_H
