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
#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#define AM_LCG_M 2147483647
#define AM_LCG_A 48271
#define AM_LCG_C 0

namespace SparkyStudios::Audio::Amplitude
{
    struct BeizerCurveControlPoints
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

    AM_API_PRIVATE struct
    {
        AmInt32 state;
    } gLCG = { 4321 };

    AM_API_PRIVATE AM_INLINE float AmDitherReal32(const AmReal32 ditherMin, const AmReal32 ditherMax)
    {
        gLCG.state = (AM_LCG_A * gLCG.state + AM_LCG_C) % AM_LCG_M;
        const AmReal32 x = gLCG.state / (double)0x7FFFFFFF;
        return ditherMin + x * (ditherMax - ditherMin);
    }

    AM_API_PRIVATE AM_INLINE AmInt32 AmFloatToFixedPoint(const AmReal32 x)
    {
        return static_cast<AmInt32>(x * kAmFixedPointUnit);
    }

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

    AM_API_PRIVATE AM_INLINE AmReal32 ComputeDopplerFactor(
        const AmVec3& locationDelta,
        const AmVec3& sourceVelocity,
        const AmVec3& listenerVelocity,
        const AmReal32 soundSpeed,
        const AmReal32 dopplerFactor)
    {
        const AmReal32 deltaLength = AM_Len(locationDelta);

        if (deltaLength == 0.0f)
            return 1.0f;

        AmReal32 vss = AM_Dot(sourceVelocity, locationDelta) / deltaLength;
        AmReal32 vls = AM_Dot(listenerVelocity, locationDelta) / deltaLength;

        const AmReal32 maxSpeed = soundSpeed / dopplerFactor;
        vss = AM_MIN(vss, maxSpeed);
        vls = AM_MIN(vls, maxSpeed);

        return (soundSpeed - vls * dopplerFactor) / (soundSpeed - vss * dopplerFactor);
    }

    /**
     * @brief Returns the next power of 2 of a given number.
     *
     * @param val The number.
     *
     * @return The next power of 2.
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
     * @tparam T An integer type, a floating-point type, or a any other type where operatror *= is defined.
     * @param base Input of the power function.
     * @param exp The exponent of the power function. Must be non-negative.
     *
     * @return The result of raising the base to the power of the exponent.
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
     * @brief Returns a direction vector relative to a given position and rotation.
     *
     * @param originPosition Origin position of the direction.
     * @param originRotation Origin rotation of the direction.
     * @param position Target position of the direction.
     *
     * @return A relative direction vector (not normalized).
     */
    AM_API_PRIVATE AM_INLINE AmVec3 GetRelativeDirection(const AmVec3& originPosition, const AmQuat& originRotation, const AmVec3& position)
    {
        return AM_RotateV3Q(position - originPosition, AM_InvQ(originRotation));
    }

    /**
     * @brief Finds the greatest common divisor (GCD) of two integers.
     *
     * @param a First integer.
     * @param b Second integer.
     *
     * @return The greatest common divisor of a and b.
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
