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

#ifndef _AM_MATH_UTILS_H
#define _AM_MATH_UTILS_H

#include <SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h>
#include <SparkyStudios/Audio/Amplitude/Math/LinearAlgebra.h>

/**
 * @{
 * @internal
 */
#define AM_LCG_M 2147483647
#define AM_LCG_A 48271
#define AM_LCG_C 0
#define AM_PI 3.14159265358979323846
#define AM_PI32 3.14159265359f
#define AM_DEG180 180.0
#define AM_DEG18032 180.0f
#define AM_TURNHALF 0.5
#define AM_TURNHALF32 0.5f
#define AM_RadToDeg ((float)(AM_DEG180 / AM_PI))
#define AM_RadToTurn ((float)(AM_TURNHALF / AM_PI))
#define AM_DegToRad ((float)(AM_PI / AM_DEG180))
#define AM_DegToTurn ((float)(AM_TURNHALF / AM_DEG180))
#define AM_TurnToRad ((float)(AM_PI / AM_TURNHALF))
#define AM_TurnToDeg ((float)(AM_DEG180 / AM_TURNHALF))
/** @} */

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief A structure containing control points for a Bézier curve.
     *
     * @ingroup math
     */
    struct BezierCurveControlPoints
    {
        /**
         * @brief The x coordinate of the second control point.
         */
        AmReal32 x1;

        /**
         * @brief The y coordinate of the second control point.
         */
        AmReal32 y1;

        /**
         * @brief The x coordinate of the third control point.
         */
        AmReal32 x2;

        /**
         * @brief The y coordinate of the third control point.
         */
        AmReal32 y2;
    };

    /**
     * @internal
     */
    AM_API_PRIVATE struct
    {
        AmInt32 state;
    } gLCG = { 4321 };

    /**
     * @brief Generates a random number between @c ditherMin and @c ditherMax.
     *
     * @param[in] ditherMin The minimum value for the random number.
     * @param[in] ditherMax The maximum value for the random number.
     *
     * @return A random number between @c ditherMin and @c ditherMax.
     *
     * @ingroup math
     */
    AM_API_PRIVATE AM_INLINE AmReal32 AmDitherReal32(const AmReal32 ditherMin, const AmReal32 ditherMax)
    {
        gLCG.state = (AM_LCG_A * gLCG.state + AM_LCG_C) % AM_LCG_M;
        const AmReal32 x = gLCG.state / static_cast<double>(0x7FFFFFFF);
        return ditherMin + x * (ditherMax - ditherMin);
    }

    /**
     * @brief Converts a 32-bit floating-point audio sample to a fixed-point representation.
     *
     * @param[in] x The 32-bit floating-point audio sample to convert.
     *
     * @return The fixed-point representation of the input 32-bit floating-point audio sample.
     *
     * @ingroup math
     */
    AM_API_PRIVATE AM_INLINE AmInt32 AmFloatToFixedPoint(const AmReal32 x)
    {
        return static_cast<AmInt32>(x * kAmFixedPointUnit);
    }

    /**
     * @brief Converts a 16-bit signed integer audio sample to a 32-bit floating-point representation.
     *
     * @param[in] x The 16-bit signed integer audio sample to convert.
     *
     * @return The 32-bit floating-point representation of the input 16-bit signed integer audio sample.
     *
     * @note For more accurate conversion, the SDK should be compiled with the @c AM_ACCURATE_CONVERSION
     * macro defined.
     *
     * @ingroup math
     */
    AM_API_PRIVATE AM_INLINE AmReal32 AmInt16ToReal32(const AmInt16 x)
    {
        auto y = static_cast<AmReal32>(x);

#if defined(AM_ACCURATE_CONVERSION)
        // The accurate way.
        y = y + 32768.0f; // -32768..32767 to 0..65535
        y = y * 0.00003051804379339284f; // 0..65535 to 0..2
        y = y - 1; // 0..2 to -1..1
#else
        // The fast way.
        y = y * 0.000030517578125f; // -32768..32767 to -1..0.999969482421875
#endif

        return y;
    }

    /**
     * @brief Converts a 32-bit signed integer audio sample to a 32-bit floating-point representation.
     *
     * @param[in] x The 32-bit signed integer audio sample to convert.
     *
     * @return The 32-bit floating-point representation of the input 32-bit signed integer audio sample.
     *
     * @note For more accurate conversion, the SDK should be compiled with the @c AM_ACCURATE_CONVERSION
     * macro defined.
     *
     * @ingroup math
     */
    AM_API_PRIVATE AM_INLINE AmReal32 AmInt32ToReal32(const AmInt32 x)
    {
        auto y = static_cast<AmReal32>(x);

#if defined(AM_ACCURATE_CONVERSION)
        // The accurate way.
        y = y + 32768.0f; // -32768..32767 to 0..65535
        y = y * 0.00003051804379339284f; // 0..65535 to 0..2
        y = y - 1; // 0..2 to -1..1
#else
        // The fast way.
        y = y * 0.000030517578125f; // -32768..32767 to -1..0.999969482421875
#endif

        return y;
    }

    /**
     * @brief Converts a 32-bit floating-point audio sample to a 16-bit signed integer representation.
     *
     * @param[in] x The 32-bit floating-point audio sample to convert.
     * @param[in] dithering If @c true, adds a dithering noise to the output.
     *
     * @return The 16-bit signed integer representation of the input 32-bit floating-point audio sample.
     *
     * @note For more accurate conversion, the SDK should be compiled with the @c AM_ACCURATE_CONVERSION
     * macro defined.
     *
     * @ingroup math
     */
    AM_API_PRIVATE AM_INLINE AmInt16 AmReal32ToInt16(const AmReal32 x, bool dithering = false)
    {
        AmReal32 y = x;

        if (dithering)
        {
            // Performs a rectangular dithering
            y += AmDitherReal32(1.0f / INT16_MIN, 1.0f / INT16_MAX);
        }

        y = AM_CLAMP(y, -1.0f, 1.0f);

#if defined(AM_ACCURATE_CONVERSION)
        // The accurate way.
        y = y + 1; // -1..1 to 0..2
        y = y * 32767.5f; // 0..2 to 0..65535
        y = y - 32768.0f; // 0...65535 to -32768..32767
#else
        // The fast way.
        y = y * 32767.0f; // -1..1 to -32767..32767
#endif

        return static_cast<AmInt16>(y);
    }

    /**
     * @brief Computes the Catmull-Rom interpolation value at a given time @c t between four points.
     *
     * @param[in] t The time value between 0 and 1.
     * @param[in] p0 The first point.
     * @param[in] p1 The second point.
     * @param[in] p2 The third point.
     * @param[in] p3 The fourth point.
     *
     * @return The Catmull-Rom interpolation value at the given time @c t.
     *
     * @ingroup math
     */
    AM_API_PRIVATE AM_INLINE AmReal32
    CatmullRom(const AmReal32 t, const AmReal32 p0, const AmReal32 p1, const AmReal32 p2, const AmReal32 p3)
    {
        // clang-format off
        return 0.5f * (
            (2 * p1) +
            (-p0 + p2) * t +
            (2 * p0 - 5 * p1 + 4 * p2 - p3) * t * t +
            (-p0 + 3 * p1 - 3 * p2 + p3) * t * t * t
        );
        // clang-format on
    }

    /**
     * @brief Returns the next power of 2 of a given number.
     *
     * @tparam T An integer type, a floating-point type, or any other type where operator *= is defined.
     *
     * @param[in] val The number.
     *
     * @return The next power of 2.
     *
     * @ingroup math
     */
    template<typename T>
    AM_API_PRIVATE AM_INLINE T NextPowerOf2(const T& val)
    {
        T nextPowerOf2 = 1;
        while (nextPowerOf2 < val)
            nextPowerOf2 *= 2;

        return nextPowerOf2;
    }

    /**
     * @brief Computes the value base^exp using the squared exponentiation method.
     *
     * @tparam T An integer type, a floating-point type, or any other type where operator *= is defined.

     * @param[in] base Input of the power function.
     * @param[in] exp The exponent of the power function. Must be non-negative.
     *
     * @return The result of raising the base to the power of the exponent.
     *
     * @ingroup math
     */
    template<typename T>
    AM_API_PRIVATE AM_INLINE T IntegerPow(T base, AmInt32 exp)
    {
        AMPLITUDE_ASSERT(exp >= 0);
        T result = static_cast<T>(1);

        while (true)
        {
            if (exp & 1)
                result *= base;

            exp >>= 1;

            if (!exp)
                break;

            base *= base;
        }

        return result;
    }


    /**
     * @brief Finds the greatest common divisor (GCD) of two integers.
     *
     * @param[in] a First integer.
     * @param[in] b Second integer.
     *
     * @return The greatest common divisor of a and b.
     *
     * @ingroup math
     */
    AM_API_PRIVATE AM_INLINE AmInt64 FindGCD(AmInt64 a, AmInt64 b)
    {
        a = std::abs(a);
        b = std::abs(b);

        AmInt64 c = 0;

        while (b != 0)
        {
            c = b;
            b = a % b;
            a = c;
        }

        return a;
    }
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_MATH_UTILS_H
