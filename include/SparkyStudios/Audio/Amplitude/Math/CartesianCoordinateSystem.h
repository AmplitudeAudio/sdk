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

#ifndef _AM_MATH_CARTESIAN_COORDINATE_SYSTEM_H
#define _AM_MATH_CARTESIAN_COORDINATE_SYSTEM_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief A class representing a cartesian coordinate system.
     *
     * It's used to know in which direction is positive along each axis, and also allow
     * Amplitude to convert incoming data to the internal coordinate sytem.
     */
    class AM_API_PUBLIC CartesianCoordinateSystem
    {
    public:
        /**
         * @brief Enumerates the axes of the cartesian coordinate system.
         */
        enum class Axis
        {
            /**
             * @brief The positive X axis (1, 0, 0).
             */
            PositiveX,

            /**
             * @brief The positive Y axis (0, 1, 0).
             */
            PositiveY,

            /**
             * @brief The positive Z axis (0, 0, 1).
             */
            PositiveZ,

            /**
             * @brief The negative X axis (-1, 0, 0).
             */
            NegativeX,

            /**
             * @brief The negative Y axis (0, -1, 0).
             */
            NegativeY,

            /**
             * @brief The negative Z axis (0, 0, -1).
             */
            NegativeZ
        };

        /**
         * @brief Converts values from one cartesian coordinate system to another.
         */
        class Converter
        {
        public:
            /**
             * @brief Constructs a converter from two cartesian coordinate systems.
             *
             * @param from The source cartesian coordinate system.
             * @param to The target cartesian coordinate system.
             */
            Converter(const CartesianCoordinateSystem& from, const CartesianCoordinateSystem& to);

            /**
             * @brief Converts a vector from the source coordinate system to the target coordinate system.
             *
             * @param vector The vector to convert.
             *
             * @return A vector in the target coordinate system.
             */
            [[nodiscard]] AmVec3 Forward(const AmVec3& vector) const;

            /**
             * @brief Converts a quaternion from the source coordinate system to the target coordinate system.
             *
             * @param quaternion The quaternion to convert.
             *
             * @return A quaternion in the target coordinate system.
             */
            [[nodiscard]] AmQuat Forward(const AmQuat& quaternion) const;

            /**
             * @brief Converts a scalar from the source coordinate system to the target coordinate system.
             *
             * @param scalar The scalar to convert.
             *
             * @return A scalar in the target coordinate system.
             */
            [[nodiscard]] AmReal32 Forward(const AmReal32& scalar) const;

            /**
             * @brief Converts a vector from the target coordinate system to the source coordinate system.
             *
             * @param vector The vector to convert.
             *
             * @return A vector in the source coordinate system.
             */
            [[nodiscard]] AmVec3 Backward(const AmVec3& vector) const;

            /**
             * @brief Converts a quaternion from the target coordinate system to the source coordinate system.
             *
             * @param quaternion The quaternion to convert.
             *
             * @return A quaternion in the source coordinate system.
             */
            [[nodiscard]] AmQuat Backward(const AmQuat& quaternion) const;

            /**
             * @brief Converts a scalar from the target coordinate system to the source coordinate system.
             *
             * @param scalar The scalar to convert.
             *
             * @return A scalar in the source coordinate system.
             */
            [[nodiscard]] AmReal32 Backward(const AmReal32& scalar) const;

        private:
            AmMat3 _fromToMatrix;
            AmMat3 _toFromMatrix;

            AmReal32 _fromToScalar;
            AmReal32 _toFromScalar;

            AmReal32 _windingSwap;
        };

        /**
         * @brief Creates a cartesian coordinate system which match the one used in Amplitude.
         *
         * @return Amplitude's internal coordinate system for right-handed, Z-up cartesian coordinate system.
         */
        static CartesianCoordinateSystem Default();

        /**
         * @brief Creates a cartesian coordinate system suitable for the AmbiX ambisonics format.
         *
         * @return AmbiX format's cartesian coordinate system.
         */
        static CartesianCoordinateSystem AmbiX();

        /**
         * @brief Creates a right-handed, Y-up cartesian coordinate system.
         *
         * @return A right-handed, Y-up cartesian coordinate system.
         */
        static CartesianCoordinateSystem RightHandedYUp();

        /**
         * @brief Creates a left-handed, Y-up cartesian coordinate system.
         *
         * @return A left-handed, Y-up cartesian coordinate system.
         */
        static CartesianCoordinateSystem LeftHandedYUp();

        /**
         * @brief Creates a right-handed, Z-up cartesian coordinate system.
         *
         * @return A right-handed, Z-up cartesian coordinate system.
         */
        static CartesianCoordinateSystem RightHandedZUp();

        /**
         * @brief Creates a left-handed, Z-up cartesian coordinate system.
         *
         * @return A left-handed, Z-up cartesian coordinate system.
         */
        static CartesianCoordinateSystem LeftHandedZUp();

        /**
         * @brief Converts a vector from one coordinate system to another.
         *
         * @param vector The vector to convert.
         * @param from The source coordinate system.
         * @param to The destination coordinate system.
         *
         * @return The converted vector.
         */
        static AmVec3 Convert(const AmVec3& vector, const CartesianCoordinateSystem& from, const CartesianCoordinateSystem& to);

        /**
         * @brief Converts a vector from one coordinate system to the default coordinate system.
         *
         * @param vector The vector to convert.
         * @param from The source coordinate system.
         *
         * @return The converted vector.
         */
        static AmVec3 ConvertToDefault(const AmVec3& vector, const CartesianCoordinateSystem& from);

        /**
         * @brief Gets a vector corresponding to the given axis.
         *
         * @param axis The axis to get the vector for.
         *
         * @return A vector corresponding to the given axis.
         */
        [[nodiscard]] static AmVec3 GetVector(Axis axis);

        /**
         * @brief Creates a new cartesian coordinate system with the given axes.
         *
         * @param rigth The right axis of the new coordinate system.
         * @param forward The forward axis of the new coordinate system.
         * @param up The up axis of the new coordinate system.
         */
        CartesianCoordinateSystem(Axis rigth, Axis forward, Axis up);

        /**
         * @brief Gets the vector corresponding to the coordinate system's right axis.
         * @return The coordinate system's right vector.
         */
        [[nodiscard]] AM_INLINE(AmVec3) GetRightVector() const
        {
            return GetVector(_rightAxis);
        }

        /**
         * @brief Gets the vector corresponding to the coordinate system's forward axis.
         * @return The coordinate system's forward vector.
         */
        [[nodiscard]] AM_INLINE(AmVec3) GetForwardVector() const
        {
            return GetVector(_forwardAxis);
        }

        /**
         * @brief Gets the vector corresponding to the coordinate system's up axis.
         * @return The coordinate system's up vector.
         */
        [[nodiscard]] AM_INLINE(AmVec3) GetUpVector() const
        {
            return GetVector(_upAxis);
        }

        /**
         * @brief Converts a vector from one coordinate system to the current one.
         *
         * @param vector The vector to convert.
         * @param from The original coordinate system of the vector.
         *
         * @return The converted vector in the current coordinate system.
         */
        [[nodiscard]] AmVec3 Convert(const AmVec3& vector, const CartesianCoordinateSystem& from) const;

        /**
         * @biref Converts a quaternion from one coordinate system to the current one.
         *
         * @param quaternion The quaternion to convert.
         * @param from The original coordinate system of the quaternion.
         *
         * @return The converted quaternion in the current coordinate system.
         */
        [[nodiscard]] AmQuat Convert(const AmQuat& quaternion, const CartesianCoordinateSystem& from) const;

        /**
         * @brief Converts a scalar from one coordinate system to the current one.
         *
         * @param scalar The scalar to convert.
         * @param from The original coordinate system of the scalar.
         *
         * @return The converted scalar in the current coordinate system.
         */
        [[nodiscard]] AmReal32 Convert(const AmReal32& scalar, const CartesianCoordinateSystem& from) const;

    private:
        Axis _rightAxis;
        Axis _forwardAxis;
        Axis _upAxis;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif //_AM_MATH_CARTESIAN_COORDINATE_SYSTEM_H
