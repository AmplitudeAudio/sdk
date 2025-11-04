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

#ifndef _AM_MATH_LINEAR_ALGEBRA_H
#define _AM_MATH_LINEAR_ALGEBRA_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief A column-vector of two float values.
     *
     * - Index 0 is the value of the X component.
     * - Index 1 is the value of the Y component.
     */
    typedef union AmVector2 {
        AmReal32 data[2];

        struct
        {
            AmReal32 x;
            AmReal32 y;
        };

#ifdef __cplusplus
        AM_INLINE AmReal32* ptr()
        {
            return data;
        }

        [[nodiscard]] AM_INLINE const AmReal32* ptr() const
        {
            return data;
        }

        AM_INLINE AmReal32& operator[](const AmUInt32 index)
        {
            AMPLITUDE_ASSERT(AM_BETWEEN(index, 0, 1));
            return data[index];
        }

        [[nodiscard]] AM_INLINE const AmReal32& operator[](const AmUInt32 index) const
        {
            AMPLITUDE_ASSERT(AM_BETWEEN(index, 0, 1));
            return data[index];
        }

        [[nodiscard]] AM_INLINE bool operator==(const AmVector2& other) const
        {
            return x == other.x && y == other.y;
        }

        [[nodiscard]] AM_INLINE bool operator!=(const AmVector2& other) const
        {
            return !(*this == other);
        }
#endif
    } AmVector2;

    /**
     * @brief A column-vector of three float values.
     *
     * - Index 0 is the value of the X component.
     * - Index 1 is the value of the Y component.
     * - Index 2 is the value of the Z component.
     */
    typedef union AmVector3 {
        AmReal32 data[3];

        struct
        {
            AmReal32 x;
            AmReal32 y;
            AmReal32 z;
        };

        struct
        {
            AmVector2 xy;
            AmReal32 _pad0;
        };

        struct
        {
            AmReal32 _pad1;
            AmVector2 yz;
        };

#ifdef __cplusplus
        AM_INLINE AmReal32* ptr()
        {
            return data;
        }

        [[nodiscard]] AM_INLINE const AmReal32* ptr() const
        {
            return data;
        }

        AM_INLINE AmReal32& operator[](const AmUInt32 index)
        {
            AMPLITUDE_ASSERT(AM_BETWEEN(index, 0, 2));
            return data[index];
        }

        [[nodiscard]] AM_INLINE const AmReal32& operator[](const AmUInt32 index) const
        {
            AMPLITUDE_ASSERT(AM_BETWEEN(index, 0, 2));
            return data[index];
        }

        [[nodiscard]] AM_INLINE bool operator==(const AmVector3& other) const
        {
            return x == other.x && y == other.y && z == other.z;
        }

        [[nodiscard]] AM_INLINE bool operator!=(const AmVector3& other) const
        {
            return !(*this == other);
        }
#endif
    } AmVector3;

    /**
     * @brief A column-vector of four float values.
     *
     * - Index 0 is the value of the X component.
     * - Index 1 is the value of the Y component.
     * - Index 2 is the value of the Z component.
     * - Index 3 is the value of the W component.
     */
    typedef union AmVector4 {
        AmReal32 data[4];

        struct
        {
            AmReal32 x;
            AmReal32 y;
            AmReal32 z;
            AmReal32 w;
        };

        struct
        {
            AmVector2 xy;
            AmVector2 zw;
        };

        struct
        {
            AmReal32 _pad0;
            AmVector2 yz;
            AmReal32 _pad1;
        };

        struct
        {
            AmVector3 xyz;
            AmReal32 _pad2;
        };

        struct
        {
            AmReal32 _pad3;
            AmVector3 yzw;
        };

#ifdef __cplusplus
        AM_INLINE AmReal32* ptr()
        {
            return data;
        }

        [[nodiscard]] AM_INLINE const AmReal32* ptr() const
        {
            return data;
        }

        AM_INLINE AmReal32& operator[](const AmUInt32 index)
        {
            AMPLITUDE_ASSERT(AM_BETWEEN(index, 0, 3));
            return data[index];
        }

        [[nodiscard]] AM_INLINE const AmReal32& operator[](const AmUInt32 index) const
        {
            AMPLITUDE_ASSERT(AM_BETWEEN(index, 0, 3));
            return data[index];
        }

        [[nodiscard]] AM_INLINE bool operator==(const AmVector4& other) const
        {
            return x == other.x && y == other.y && z == other.z && w == other.w;
        }

        [[nodiscard]] AM_INLINE bool operator!=(const AmVector4& other) const
        {
            return !(*this == other);
        }
#endif
    } AmVector4;

    /**
     * @brief A quaternion rotation.
     *
     * - Index 0 is the value of the W component.
     * - Index 1 is the value of the X component.
     * - Index 2 is the value of the Y component.
     * - Index 3 is the value of the Z component.
     */
    typedef union AmQuaternion {
        AmReal32 data[4];

        struct
        {
            AmReal32 w;
            AmReal32 x;
            AmReal32 y;
            AmReal32 z;
        };

        struct
        {
            AmReal32 _pad0;
            AmVector3 xyz;
        };

#ifdef __cplusplus
        AM_INLINE AmReal32* ptr()
        {
            return data;
        }

        [[nodiscard]] AM_INLINE const AmReal32* ptr() const
        {
            return data;
        }

        AM_INLINE AmReal32& operator[](const AmUInt32 index)
        {
            AMPLITUDE_ASSERT(AM_BETWEEN(index, 0, 3));
            return data[index];
        }

        [[nodiscard]] AM_INLINE const AmReal32& operator[](const AmUInt32 index) const
        {
            AMPLITUDE_ASSERT(AM_BETWEEN(index, 0, 3));
            return data[index];
        }

        [[nodiscard]] AM_INLINE bool operator==(const AmQuaternion& other) const
        {
            return w == other.w && x == other.x && y == other.y && z == other.z;
        }

        [[nodiscard]] AM_INLINE bool operator!=(const AmQuaternion& other) const
        {
            return !(*this == other);
        }
#endif
    } AmQuaternion;

    /**
     * @brief A 3x3 matrix represented as an array of 9 float values in column-major order.
     *
     * The matrix layout in memory is:
     * [m00 m01 m02]
     * [m10 m11 m12]
     * [m20 m21 m22]
     */
    typedef union AmMatrix3 {
        AmReal32 data[9];
        AmVector3 rows[3];

        struct
        {
            AmReal32 m00;
            AmReal32 m01;
            AmReal32 m02;
            AmReal32 m10;
            AmReal32 m11;
            AmReal32 m12;
            AmReal32 m20;
            AmReal32 m21;
            AmReal32 m22;
        };

#ifdef __cplusplus
        AM_INLINE AmReal32* ptr()
        {
            return data;
        }

        [[nodiscard]] AM_INLINE const AmReal32* ptr() const
        {
            return data;
        }

        AM_INLINE AmVector3& operator[](const AmUInt32 index)
        {
            AMPLITUDE_ASSERT(AM_BETWEEN(index, 0, 2));
            return rows[index];
        }

        [[nodiscard]] AM_INLINE const AmVector3& operator[](const AmUInt32 index) const
        {
            AMPLITUDE_ASSERT(AM_BETWEEN(index, 0, 2));
            return rows[index];
        }

        [[nodiscard]] AM_INLINE bool operator==(const AmMatrix3& other) const
        {
            return rows[0] == other.rows[0] && rows[1] == other.rows[1] && rows[2] == other.rows[2];
        }

        [[nodiscard]] AM_INLINE bool operator!=(const AmMatrix3& other) const
        {
            return !(*this == other);
        }
#endif
    } AmMatrix3;

    /**
     * @brief A 4x4 matrix represented as an array of 16 float values in column-major order.
     *
     * The matrix layout in memory is:
     * [m00 m01 m02 m03]
     * [m10 m11 m12 m13]
     * [m20 m21 m22 m23]
     * [m30 m31 m32 m33]
     */
    typedef union AmMatrix4 {
        AmReal32 data[16];
        AmVector4 rows[4];

        struct
        {
            AmReal32 m00;
            AmReal32 m01;
            AmReal32 m02;
            AmReal32 m03;
            AmReal32 m10;
            AmReal32 m11;
            AmReal32 m12;
            AmReal32 m13;
            AmReal32 m20;
            AmReal32 m21;
            AmReal32 m22;
            AmReal32 m23;
            AmReal32 m30;
            AmReal32 m31;
            AmReal32 m32;
            AmReal32 m33;
        };

#ifdef __cplusplus
        AM_INLINE AmReal32* ptr()
        {
            return data;
        }

        [[nodiscard]] AM_INLINE const AmReal32* ptr() const
        {
            return data;
        }

        AM_INLINE AmVector4& operator[](const AmUInt32 index)
        {
            AMPLITUDE_ASSERT(AM_BETWEEN(index, 0, 3));
            return rows[index];
        }

        [[nodiscard]] AM_INLINE const AmVector4& operator[](const AmUInt32 index) const
        {
            AMPLITUDE_ASSERT(AM_BETWEEN(index, 0, 3));
            return rows[index];
        }

        [[nodiscard]] AM_INLINE bool operator==(const AmMatrix4& other) const
        {
            return rows[0] == other.rows[0] && rows[1] == other.rows[1] && rows[2] == other.rows[2] && rows[3] == other.rows[3];
        }

        [[nodiscard]] AM_INLINE bool operator!=(const AmMatrix4& other) const
        {
            return !(*this == other);
        }
#endif
    } AmMatrix4;

    /**
     * @brief A zero vector in 2D space.
     *
     * This constant represents a vector with all components set to zero.
     *
     * @ingroup math
     */
    constexpr AmVector2 kVector2Zero = { 0.0f, 0.0f };

    /**
     * @brief A unit vector in 2D space.
     *
     * This constant represents a vector with all components set to one.
     *
     * @ingroup math
     */
    constexpr AmVector2 kVector2One = { 1.0f, 1.0f };

    /**
     * @brief A unit vector along the X-axis in 2D space.
     *
     * This constant represents a basis vector pointing in the positive X direction.
     *
     * @ingroup math
     */
    constexpr AmVector2 kVector2UnitX = { 1.0f, 0.0f };

    /**
     * @brief A unit vector along the Y-axis in 2D space.
     *
     * This constant represents a basis vector pointing in the positive Y direction.
     *
     * @ingroup math
     */
    constexpr AmVector2 kVector2UnitY = { 0.0f, 1.0f };

    /**
     * @brief A zero vector in 3D space.
     *
     * This constant represents a vector with all components set to zero.
     *
     * @ingroup math
     */
    constexpr AmVector3 kVector3Zero = { 0.0f, 0.0f, 0.0f };

    /**
     * @brief A unit vector in 3D space.
     *
     * This constant represents a vector with all components set to one.
     *
     * @ingroup math
     */
    constexpr AmVector3 kVector3One = { 1.0f, 1.0f, 1.0f };

    /**
     * @brief A unit vector along the X-axis in 3D space.
     *
     * This constant represents a basis vector pointing in the positive X direction.
     *
     * @ingroup math
     */
    constexpr AmVector3 kVector3UnitX = { 1.0f, 0.0f, 0.0f };

    /**
     * @brief A unit vector along the Y-axis in 3D space.
     *
     * This constant represents a basis vector pointing in the positive Y direction.
     *
     * @ingroup math
     */
    constexpr AmVector3 kVector3UnitY = { 0.0f, 1.0f, 0.0f };

    /**
     * @brief A unit vector along the Z-axis in 3D space.
     *
     * This constant represents a basis vector pointing in the positive Z direction.
     *
     * @ingroup math
     */
    constexpr AmVector3 kVector3UnitZ = { 0.0f, 0.0f, 1.0f };

    /**
     * @brief A zero vector in 4D space.
     *
     * This constant represents a vector with all components set to zero.
     *
     * @ingroup math
     */
    constexpr AmVector4 kVector4Zero = { 0.0f, 0.0f, 0.0f, 0.0f };

    /**
     * @brief A unit vector in 4D space.
     *
     * This constant represents a vector with all components set to one.
     *
     * @ingroup math
     */
    constexpr AmVector4 kVector4One = { 1.0f, 1.0f, 1.0f, 1.0f };

    /**
     * @brief A unit vector along the X-axis in 4D space.
     *
     * This constant represents a basis vector pointing in the positive X direction.
     *
     * @ingroup math
     */
    constexpr AmVector4 kVector4UnitX = { 1.0f, 0.0f, 0.0f, 0.0f };

    /**
     * @brief A unit vector along the Y-axis in 4D space.
     *
     * This constant represents a basis vector pointing in the positive Y direction.
     *
     * @ingroup math
     */
    constexpr AmVector4 kVector4UnitY = { 0.0f, 1.0f, 0.0f, 0.0f };

    /**
     * @brief A unit vector along the Z-axis in 4D space.
     *
     * This constant represents a basis vector pointing in the positive Z direction.
     *
     * @ingroup math
     */
    constexpr AmVector4 kVector4UnitZ = { 0.0f, 0.0f, 1.0f, 0.0f };

    /**
     * @brief A unit vector along the W-axis in 4D space.
     *
     * This constant represents a basis vector pointing in the positive W direction.
     *
     * @ingroup math
     */
    constexpr AmVector4 kVector4UnitW = { 0.0f, 0.0f, 0.0f, 1.0f };

    /**
     * @brief An identity matrix in 3D space.
     *
     * This constant represents a 3x3 identity matrix, which is used to represent no transformation.
     * @ingroup math
     */
    constexpr AmMatrix3 kMatrix3Identity = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };

    /**
     * @brief An identity matrix in 4D space.
     *
     * This constant represents a 4x4 identity matrix, which is used to represent no transformation.
     *
     * @ingroup math
     */
    constexpr AmMatrix4 kMatrix4Identity = {
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_MATH_LINEAR_ALGEBRA_H
