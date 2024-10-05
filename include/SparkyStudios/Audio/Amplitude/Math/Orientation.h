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

#ifndef _AM_MATH_ORIENTATION_H
#define _AM_MATH_ORIENTATION_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Represents an orientation in 3D space.
     *
     * This class provides methods for converting between different coordinate systems and
     * manipulating orientations. The orientation can be built from yaw, pitch, and roll angles,
     * or from a forward and up vector. Once built, the orientation is stored in the ZYX representation
     * (yaw, pitch, roll), in the ZYZ representation, in the quaternion representation, and in forward-up
     * vectors representation.
     *
     * @ingroup math
     */
    struct AM_API_PUBLIC Orientation
    {
        /**
         * @brief Gets a zero Orientation instance.
         *
         * @return A zero Orientation instance. 0 for all angles.
         */
        static Orientation Zero();

        /**
         * @brief Constructs an Orientation instance with the given yaw, pitch, and roll angles.
         *
         * @param[in] yaw The angle of rotation around the X-axis in radians.
         * @param[in] pitch The angle of rotation around the Y-axis in radians.
         * @param[in] roll The angle of rotation around the Z-axis in radians.
         */
        Orientation(AmReal32 yaw, AmReal32 pitch, AmReal32 roll);

        /**
         * @brief Constructs an Orientation instance from a forward and up vector.
         *
         * @param[in] forward The forward vector of the orientation.
         * @param[in] up The up vector of the orientation.
         */
        Orientation(AmVec3 forward, AmVec3 up);

        /**
         * @brief Constructs an Orientation instance from a quaternion.
         *
         * @param[in] quaternion The quaternion representing the orientation.
         */
        Orientation(AmQuat quaternion);

        /**
         * @brief The angle of rotation around the Z-axis in radians following the ZYX convention.
         */
        [[nodiscard]] AM_INLINE AmReal32 GetYaw() const
        {
            return _yaw;
        }

        /**
         * @brief The angle of rotation around the Y-axis in radians following the ZYX convention.
         */
        [[nodiscard]] AM_INLINE AmReal32 GetPitch() const
        {
            return _pitch;
        }

        /**
         * @brief The angle of rotation around the X-axis in radians following the ZYX convention.
         */
        [[nodiscard]] AM_INLINE AmReal32 GetRoll() const
        {
            return _roll;
        }

        /**
         * @brief The forward vector of the orientation.
         */
        [[nodiscard]] AM_INLINE AmVec3 GetForward() const
        {
            return _forward;
        }

        /**
         * @brief The up vector of the orientation.
         */
        [[nodiscard]] AM_INLINE AmVec3 GetUp() const
        {
            return _up;
        }

        /**
         * @brief The angle of rotation around the Z-axis in radians following the ZYZ convention.
         */
        [[nodiscard]] AM_INLINE AmReal32 GetAlpha() const
        {
            return _alpha;
        }

        /**
         * @brief The angle of rotation around the Y-axis in radians following the ZYZ convention.
         */
        [[nodiscard]] AM_INLINE AmReal32 GetBeta() const
        {
            return _beta;
        }

        /**
         * @brief The angle of rotation around the new Z-axis in radians following the ZYZ convention.
         */
        [[nodiscard]] AM_INLINE AmReal32 GetGamma() const
        {
            return _gamma;
        }

        /**
         * @brief The quaternion representation of the orientation.
         */
        [[nodiscard]] AM_INLINE AmQuat GetQuaternion() const
        {
            return _quaternion;
        }

        /**
         * @brief Converts the orientation to a rotation matrix.
         *
         * @return A rotation matrix representing the current orientation.
         */
        [[nodiscard]] AmMat4 GetRotationMatrix() const;

        /**
         * @brief Converts the orientation to a look-at matrix.
         *
         * @param[in] eye The eye's location.
         *
         * @return A look-at matrix representing the current orientation, with the eye at the given location.
         */
        [[nodiscard]] AmMat4 GetLookAtMatrix(AmVec3 eye) const;

    private:
        void ComputeForwardAndUpVectors();
        void ComputeZYXAngles();
        void ComputeZYZAngles();
        void ComputeQuaternion();

        AmVec3 _forward;
        AmReal32 _yaw;
        AmVec3 _up;
        AmReal32 _pitch;
        AmReal32 _roll;
        AmReal32 _alpha;
        AmReal32 _beta;
        AmReal32 _gamma;
        AmQuat _quaternion;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_MATH_ORIENTATION_H
