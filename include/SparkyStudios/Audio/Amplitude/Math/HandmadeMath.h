/*
  HandmadeMath.h v2.0.0

  This is a single header file with a bunch of useful types and functions for
  games and graphics. Consider it a lightweight alternative to GLM that works
  both C and C++.

  =============================================================================
  CONFIG
  =============================================================================

  By default, all angles in Handmade Math are specified in radians. However, it
  can be configured to use degrees or turns instead. Use one of the following
  defines to specify the default unit for angles:

    #define HANDMADE_MATH_USE_RADIANS
    #define HANDMADE_MATH_USE_DEGREES
    #define HANDMADE_MATH_USE_TURNS

  Regardless of the default angle, you can use the following functions to
  specify an angle in a particular unit:

    AM_AngleRad(radians)
    AM_AngleDeg(degrees)
    AM_AngleTurn(turns)

  The definitions of these functions change depending on the default unit.

  -----------------------------------------------------------------------------

  Handmade Math ships with SSE (SIMD) implementations of several common
  operations. To disable the use of SSE intrinsics, you must define
  HANDMADE_MATH_NO_SSE before including this file:

    #define HANDMADE_MATH_NO_SSE
    #include "HandmadeMath.h"

  -----------------------------------------------------------------------------

  To use Handmade Math without the C runtime library, you must provide your own
  implementations of basic math functions. Otherwise, HandmadeMath.h will use
  the runtime library implementation of these functions.

  Define HANDMADE_MATH_PROVIDE_MATH_FUNCTIONS and provide your own
  implementations of AM_SINF, AM_COSF, AM_TANF, AM_ACOSF, and AM_SQRTF
  before including HandmadeMath.h, like so:

    #define HANDMADE_MATH_PROVIDE_MATH_FUNCTIONS
    #define AM_SINF MySinF
    #define AM_COSF MyCosF
    #define AM_TANF MyTanF
    #define AM_ACOSF MyACosF
    #define AM_SQRTF MySqrtF
    #include "HandmadeMath.h"

  By default, it is assumed that your math functions take radians. To use
  different units, you must define AM_ANGLE_USER_TO_INTERNAL and
  AM_ANGLE_INTERNAL_TO_USER. For example, if you want to use degrees in your
  code but your math functions use turns:

    #define AM_ANGLE_USER_TO_INTERNAL(a) ((a)*AM_DegToTurn)
    #define AM_ANGLE_INTERNAL_TO_USER(a) ((a)*AM_TurnToDeg)

  =============================================================================

  LICENSE

  This software is in the public domain. Where that dedication is not
  recognized, you are granted a perpetual, irrevocable license to copy,
  distribute, and modify this file as you see fit.

  =============================================================================

  CREDITS

  Originally written by Zakary Strange.

  Functionality:
   Zakary Strange (strangezak@protonmail.com && @strangezak)
   Matt Mascarenhas (@miblo_)
   Aleph
   FieryDrake (@fierydrake)
   Gingerbill (@TheGingerBill)
   Ben Visness (@bvisness)
   Trinton Bullard (@Peliex_Dev)
   @AntonDan
   Logan Forman (@dev_dwarf)

  Fixes:
   Jeroen van Rijn (@J_vanRijn)
   Kiljacken (@Kiljacken)
   Insofaras (@insofaras)
   Daniel Gibson (@DanielGibson)
*/

#ifndef HANDMADE_MATH_H
#define HANDMADE_MATH_H

// Dummy macros for when test framework is not present.
#ifndef COVERAGE
#define COVERAGE(a, b)
#endif

#ifndef ASSERT_COVERED
#define ASSERT_COVERED(a)
#endif

/* let's figure out if SSE is really available (unless disabled anyway)
   (it isn't on non-x86/x86_64 platforms or even x86 without explicit SSE support)
   => only use "#ifdef HANDMADE_MATH__USE_SSE" to check for SSE support below this block! */
#ifndef HANDMADE_MATH_NO_SSE
#ifdef _MSC_VER /* MSVC supports SSE in amd64 mode or _M_IX86_FP >= 1 (2 means SSE2) */
#if defined(_M_AMD64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 1)
#define HANDMADE_MATH__USE_SSE 1
#endif
#else /* not MSVC, probably GCC, clang, icc or something that doesn't support SSE anyway */
#ifdef __SSE__ /* they #define __SSE__ if it's supported */
#define HANDMADE_MATH__USE_SSE 1
#endif /*  __SSE__ */
#endif /* not _MSC_VER */
#endif /* #ifndef HANDMADE_MATH_NO_SSE */

#if (!defined(__cplusplus) && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L)
#define HANDMADE_MATH__USE_C11_GENERICS 1
#endif

#ifdef HANDMADE_MATH__USE_SSE
#include <xmmintrin.h>
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4201)
#endif

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#if (defined(__GNUC__) && (__GNUC__ == 4 && __GNUC_MINOR__ < 8)) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wmissing-braces"
#endif
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#endif

#if defined(__GNUC__) || defined(__clang__)
#define AM_DEPRECATED(msg) __attribute__((deprecated(msg)))
#elif defined(_MSC_VER)
#define AM_DEPRECATED(msg) __declspec(deprecated(msg))
#else
#define AM_DEPRECATED(msg)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(HANDMADE_MATH_USE_DEGREES) && !defined(HANDMADE_MATH_USE_TURNS) && !defined(HANDMADE_MATH_USE_RADIANS)
#define HANDMADE_MATH_USE_RADIANS
#endif

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

#if defined(HANDMADE_MATH_USE_RADIANS)
#define AM_AngleRad(a) (a)
#define AM_AngleDeg(a) ((a)*AM_DegToRad)
#define AM_AngleTurn(a) ((a)*AM_TurnToRad)
#elif defined(HANDMADE_MATH_USE_DEGREES)
#define AM_AngleRad(a) ((a)*AM_RadToDeg)
#define AM_AngleDeg(a) (a)
#define AM_AngleTurn(a) ((a)*AM_TurnToDeg)
#elif defined(HANDMADE_MATH_USE_TURNS)
#define AM_AngleRad(a) ((a)*AM_RadToTurn)
#define AM_AngleDeg(a) ((a)*AM_DegToTurn)
#define AM_AngleTurn(a) (a)
#endif

#if !defined(HANDMADE_MATH_PROVIDE_MATH_FUNCTIONS)
#include <math.h>
#define AM_SINF sinf
#define AM_COSF cosf
#define AM_TANF tanf
#define AM_SQRTF sqrtf
#define AM_ACOSF acosf
#endif

#if !defined(AM_ANGLE_USER_TO_INTERNAL)
#define AM_ANGLE_USER_TO_INTERNAL(a) (AM_ToRad(a))
#endif

#if !defined(AM_ANGLE_INTERNAL_TO_USER)
#if defined(HANDMADE_MATH_USE_RADIANS)
#define AM_ANGLE_INTERNAL_TO_USER(a) (a)
#elif defined(HANDMADE_MATH_USE_DEGREES)
#define AM_ANGLE_INTERNAL_TO_USER(a) ((a)*AM_RadToDeg)
#elif defined(HANDMADE_MATH_USE_TURNS)
#define AM_ANGLE_INTERNAL_TO_USER(a) ((a)*AM_RadToTurn)
#endif
#endif

#define AM_MIN(a, b) ((a) > (b) ? (b) : (a))
#define AM_MAX(a, b) ((a) < (b) ? (b) : (a))
#define AM_ABS(a) ((a) > 0 ? (a) : -(a))
#define AM_MOD(a, m) (((a) % (m)) >= 0 ? ((a) % (m)) : (((a) % (m)) + (m)))
#define AM_SQUARE(x) ((x) * (x))

typedef union AmVec2 {
    struct
    {
        float X, Y;
    };

    struct
    {
        float U, V;
    };

    struct
    {
        float Left, Right;
    };

    struct
    {
        float Width, Height;
    };

    float Elements[2];

#ifdef __cplusplus
    inline float& operator[](const int& Index)
    {
        return Elements[Index];
    }
#endif
} AmVec2;

typedef union AmVec3 {
    struct
    {
        float X, Y, Z;
    };

    struct
    {
        float U, V, W;
    };

    struct
    {
        float R, G, B;
    };

    struct
    {
        AmVec2 XY;
        float _Ignored0;
    };

    struct
    {
        float _Ignored1;
        AmVec2 YZ;
    };

    struct
    {
        AmVec2 UV;
        float _Ignored2;
    };

    struct
    {
        float _Ignored3;
        AmVec2 VW;
    };

    float Elements[3];

#ifdef __cplusplus
    inline float& operator[](const int& Index)
    {
        return Elements[Index];
    }
#endif
} AmVec3;

typedef union AmVec4 {
    struct
    {
        union {
            AmVec3 XYZ;
            struct
            {
                float X, Y, Z;
            };
        };

        float W;
    };
    struct
    {
        union {
            AmVec3 RGB;
            struct
            {
                float R, G, B;
            };
        };

        float A;
    };

    struct
    {
        AmVec2 XY;
        float _Ignored0;
        float _Ignored1;
    };

    struct
    {
        float _Ignored2;
        AmVec2 YZ;
        float _Ignored3;
    };

    struct
    {
        float _Ignored4;
        float _Ignored5;
        AmVec2 ZW;
    };

    float Elements[4];

#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSE;
#endif

#ifdef __cplusplus
    inline float& operator[](const int& Index)
    {
        return Elements[Index];
    }
#endif
} AmVec4;

typedef union AmMat2 {
    float Elements[2][2];
    AmVec2 Columns[2];

#ifdef __cplusplus
    inline AmVec2& operator[](const int& Index)
    {
        return Columns[Index];
    }
#endif
} AmMat2;

typedef union AmMat3 {
    float Elements[3][3];
    AmVec3 Columns[3];

#ifdef __cplusplus
    inline AmVec3& operator[](const int& Index)
    {
        return Columns[Index];
    }
#endif
} AmMat3;

typedef union AmMat4 {
    float Elements[4][4];
    AmVec4 Columns[4];

#ifdef __cplusplus
    inline AmVec4& operator[](const int& Index)
    {
        return Columns[Index];
    }
#endif
} AmMat4;

typedef union AmQuat {
    struct
    {
        union {
            AmVec3 XYZ;
            struct
            {
                float X, Y, Z;
            };
        };

        float W;
    };

    float Elements[4];

#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSE;
#endif
} AmQuat;

typedef signed int AmBool;

/*
 * Angle unit conversion functions
 */
static inline float AM_ToRad(float Angle)
{
#if defined(HANDMADE_MATH_USE_RADIANS)
    float Result = Angle;
#elif defined(HANDMADE_MATH_USE_DEGREES)
    float Result = Angle * AM_DegToRad;
#elif defined(HANDMADE_MATH_USE_TURNS)
    float Result = Angle * AM_TurnToRad;
#endif

    return Result;
}

static inline float AM_ToDeg(float Angle)
{
#if defined(HANDMADE_MATH_USE_RADIANS)
    float Result = Angle * AM_RadToDeg;
#elif defined(HANDMADE_MATH_USE_DEGREES)
    float Result = Angle;
#elif defined(HANDMADE_MATH_USE_TURNS)
    float Result = Angle * AM_TurnToDeg;
#endif

    return Result;
}

static inline float AM_ToTurn(float Angle)
{
#if defined(HANDMADE_MATH_USE_RADIANS)
    float Result = Angle * AM_RadToTurn;
#elif defined(HANDMADE_MATH_USE_DEGREES)
    float Result = Angle * AM_DegToTurn;
#elif defined(HANDMADE_MATH_USE_TURNS)
    float Result = Angle;
#endif

    return Result;
}

/*
 * Floating-point math functions
 */

COVERAGE(AM_SinF, 1)
static inline float AM_SinF(float Angle)
{
    ASSERT_COVERED(AM_SinF);
    return AM_SINF(AM_ANGLE_USER_TO_INTERNAL(Angle));
}

COVERAGE(AM_CosF, 1)
static inline float AM_CosF(float Angle)
{
    ASSERT_COVERED(AM_CosF);
    return AM_COSF(AM_ANGLE_USER_TO_INTERNAL(Angle));
}

COVERAGE(AM_TanF, 1)
static inline float AM_TanF(float Angle)
{
    ASSERT_COVERED(AM_TanF);
    return AM_TANF(AM_ANGLE_USER_TO_INTERNAL(Angle));
}

COVERAGE(AM_ACosF, 1)
static inline float AM_ACosF(float Arg)
{
    ASSERT_COVERED(AM_ACosF);
    return AM_ANGLE_INTERNAL_TO_USER(AM_ACOSF(Arg));
}

COVERAGE(AM_SqrtF, 1)
static inline float AM_SqrtF(float Float)
{
    ASSERT_COVERED(AM_SqrtF);

    float Result;

#ifdef HANDMADE_MATH__USE_SSE
    __m128 In = _mm_set_ss(Float);
    __m128 Out = _mm_sqrt_ss(In);
    Result = _mm_cvtss_f32(Out);
#else
    Result = AM_SQRTF(Float);
#endif

    return Result;
}

COVERAGE(AM_InvSqrtF, 1)
static inline float AM_InvSqrtF(float Float)
{
    ASSERT_COVERED(AM_InvSqrtF);

    float Result;

    Result = 1.0f / AM_SqrtF(Float);

    return Result;
}

/*
 * Utility functions
 */

COVERAGE(AM_Lerp, 1)
static inline float AM_Lerp(float A, float Time, float B)
{
    ASSERT_COVERED(AM_Lerp);
    return (1.0f - Time) * A + Time * B;
}

COVERAGE(AM_Clamp, 1)
static inline float AM_Clamp(float Min, float Value, float Max)
{
    ASSERT_COVERED(AM_Clamp);

    float Result = Value;

    if (Result < Min)
    {
        Result = Min;
    }

    if (Result > Max)
    {
        Result = Max;
    }

    return Result;
}

/*
 * Vector initialization
 */

COVERAGE(AM_V2, 1)
static inline AmVec2 AM_V2(float X, float Y)
{
    ASSERT_COVERED(AM_V2);

    AmVec2 Result;
    Result.X = X;
    Result.Y = Y;

    return Result;
}

COVERAGE(AM_V3, 1)
static inline AmVec3 AM_V3(float X, float Y, float Z)
{
    ASSERT_COVERED(AM_V3);

    AmVec3 Result;
    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;

    return Result;
}

COVERAGE(AM_V4, 1)
static inline AmVec4 AM_V4(float X, float Y, float Z, float W)
{
    ASSERT_COVERED(AM_V4);

    AmVec4 Result;

#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_setr_ps(X, Y, Z, W);
#else
    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;
    Result.W = W;
#endif

    return Result;
}

COVERAGE(AM_V4V, 1)
static inline AmVec4 AM_V4V(AmVec3 Vector, float W)
{
    ASSERT_COVERED(AM_V4V);

    AmVec4 Result;

#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_setr_ps(Vector.X, Vector.Y, Vector.Z, W);
#else
    Result.XYZ = Vector;
    Result.W = W;
#endif

    return Result;
}

/*
 * Binary vector operations
 */

COVERAGE(AM_AddV2, 1)
static inline AmVec2 AM_AddV2(AmVec2 Left, AmVec2 Right)
{
    ASSERT_COVERED(AM_AddV2);

    AmVec2 Result;
    Result.X = Left.X + Right.X;
    Result.Y = Left.Y + Right.Y;

    return Result;
}

COVERAGE(AM_AddV3, 1)
static inline AmVec3 AM_AddV3(AmVec3 Left, AmVec3 Right)
{
    ASSERT_COVERED(AM_AddV3);

    AmVec3 Result;
    Result.X = Left.X + Right.X;
    Result.Y = Left.Y + Right.Y;
    Result.Z = Left.Z + Right.Z;

    return Result;
}

COVERAGE(AM_AddV4, 1)
static inline AmVec4 AM_AddV4(AmVec4 Left, AmVec4 Right)
{
    ASSERT_COVERED(AM_AddV4);

    AmVec4 Result;

#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_add_ps(Left.SSE, Right.SSE);
#else
    Result.X = Left.X + Right.X;
    Result.Y = Left.Y + Right.Y;
    Result.Z = Left.Z + Right.Z;
    Result.W = Left.W + Right.W;
#endif

    return Result;
}

COVERAGE(AM_SubV2, 1)
static inline AmVec2 AM_SubV2(AmVec2 Left, AmVec2 Right)
{
    ASSERT_COVERED(AM_SubV2);

    AmVec2 Result;
    Result.X = Left.X - Right.X;
    Result.Y = Left.Y - Right.Y;

    return Result;
}

COVERAGE(AM_SubV3, 1)
static inline AmVec3 AM_SubV3(AmVec3 Left, AmVec3 Right)
{
    ASSERT_COVERED(AM_SubV3);

    AmVec3 Result;
    Result.X = Left.X - Right.X;
    Result.Y = Left.Y - Right.Y;
    Result.Z = Left.Z - Right.Z;

    return Result;
}

COVERAGE(AM_SubV4, 1)
static inline AmVec4 AM_SubV4(AmVec4 Left, AmVec4 Right)
{
    ASSERT_COVERED(AM_SubV4);

    AmVec4 Result;

#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_sub_ps(Left.SSE, Right.SSE);
#else
    Result.X = Left.X - Right.X;
    Result.Y = Left.Y - Right.Y;
    Result.Z = Left.Z - Right.Z;
    Result.W = Left.W - Right.W;
#endif

    return Result;
}

COVERAGE(AM_MulV2, 1)
static inline AmVec2 AM_MulV2(AmVec2 Left, AmVec2 Right)
{
    ASSERT_COVERED(AM_MulV2);

    AmVec2 Result;
    Result.X = Left.X * Right.X;
    Result.Y = Left.Y * Right.Y;

    return Result;
}

COVERAGE(AM_MulV2F, 1)
static inline AmVec2 AM_MulV2F(AmVec2 Left, float Right)
{
    ASSERT_COVERED(AM_MulV2F);

    AmVec2 Result;
    Result.X = Left.X * Right;
    Result.Y = Left.Y * Right;

    return Result;
}

COVERAGE(AM_MulV3, 1)
static inline AmVec3 AM_MulV3(AmVec3 Left, AmVec3 Right)
{
    ASSERT_COVERED(AM_MulV3);

    AmVec3 Result;
    Result.X = Left.X * Right.X;
    Result.Y = Left.Y * Right.Y;
    Result.Z = Left.Z * Right.Z;

    return Result;
}

COVERAGE(AM_MulV3F, 1)
static inline AmVec3 AM_MulV3F(AmVec3 Left, float Right)
{
    ASSERT_COVERED(AM_MulV3F);

    AmVec3 Result;
    Result.X = Left.X * Right;
    Result.Y = Left.Y * Right;
    Result.Z = Left.Z * Right;

    return Result;
}

COVERAGE(AM_MulV4, 1)
static inline AmVec4 AM_MulV4(AmVec4 Left, AmVec4 Right)
{
    ASSERT_COVERED(AM_MulV4);

    AmVec4 Result;

#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_mul_ps(Left.SSE, Right.SSE);
#else
    Result.X = Left.X * Right.X;
    Result.Y = Left.Y * Right.Y;
    Result.Z = Left.Z * Right.Z;
    Result.W = Left.W * Right.W;
#endif

    return Result;
}

COVERAGE(AM_MulV4F, 1)
static inline AmVec4 AM_MulV4F(AmVec4 Left, float Right)
{
    ASSERT_COVERED(AM_MulV4F);

    AmVec4 Result;

#ifdef HANDMADE_MATH__USE_SSE
    __m128 Scalar = _mm_set1_ps(Right);
    Result.SSE = _mm_mul_ps(Left.SSE, Scalar);
#else
    Result.X = Left.X * Right;
    Result.Y = Left.Y * Right;
    Result.Z = Left.Z * Right;
    Result.W = Left.W * Right;
#endif

    return Result;
}

COVERAGE(AM_DivV2, 1)
static inline AmVec2 AM_DivV2(AmVec2 Left, AmVec2 Right)
{
    ASSERT_COVERED(AM_DivV2);

    AmVec2 Result;
    Result.X = Left.X / Right.X;
    Result.Y = Left.Y / Right.Y;

    return Result;
}

COVERAGE(AM_DivV2F, 1)
static inline AmVec2 AM_DivV2F(AmVec2 Left, float Right)
{
    ASSERT_COVERED(AM_DivV2F);

    AmVec2 Result;
    Result.X = Left.X / Right;
    Result.Y = Left.Y / Right;

    return Result;
}

COVERAGE(AM_DivV3, 1)
static inline AmVec3 AM_DivV3(AmVec3 Left, AmVec3 Right)
{
    ASSERT_COVERED(AM_DivV3);

    AmVec3 Result;
    Result.X = Left.X / Right.X;
    Result.Y = Left.Y / Right.Y;
    Result.Z = Left.Z / Right.Z;

    return Result;
}

COVERAGE(AM_DivV3F, 1)
static inline AmVec3 AM_DivV3F(AmVec3 Left, float Right)
{
    ASSERT_COVERED(AM_DivV3F);

    AmVec3 Result;
    Result.X = Left.X / Right;
    Result.Y = Left.Y / Right;
    Result.Z = Left.Z / Right;

    return Result;
}

COVERAGE(AM_DivV4, 1)
static inline AmVec4 AM_DivV4(AmVec4 Left, AmVec4 Right)
{
    ASSERT_COVERED(AM_DivV4);

    AmVec4 Result;

#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_div_ps(Left.SSE, Right.SSE);
#else
    Result.X = Left.X / Right.X;
    Result.Y = Left.Y / Right.Y;
    Result.Z = Left.Z / Right.Z;
    Result.W = Left.W / Right.W;
#endif

    return Result;
}

COVERAGE(AM_DivV4F, 1)
static inline AmVec4 AM_DivV4F(AmVec4 Left, float Right)
{
    ASSERT_COVERED(AM_DivV4F);

    AmVec4 Result;

#ifdef HANDMADE_MATH__USE_SSE
    __m128 Scalar = _mm_set1_ps(Right);
    Result.SSE = _mm_div_ps(Left.SSE, Scalar);
#else
    Result.X = Left.X / Right;
    Result.Y = Left.Y / Right;
    Result.Z = Left.Z / Right;
    Result.W = Left.W / Right;
#endif

    return Result;
}

COVERAGE(AM_EqV2, 1)
static inline AmBool AM_EqV2(AmVec2 Left, AmVec2 Right)
{
    ASSERT_COVERED(AM_EqV2);
    return Left.X == Right.X && Left.Y == Right.Y;
}

COVERAGE(AM_EqV3, 1)
static inline AmBool AM_EqV3(AmVec3 Left, AmVec3 Right)
{
    ASSERT_COVERED(AM_EqV3);
    return Left.X == Right.X && Left.Y == Right.Y && Left.Z == Right.Z;
}

COVERAGE(AM_EqV4, 1)
static inline AmBool AM_EqV4(AmVec4 Left, AmVec4 Right)
{
    ASSERT_COVERED(AM_EqV4);
    return Left.X == Right.X && Left.Y == Right.Y && Left.Z == Right.Z && Left.W == Right.W;
}

COVERAGE(AM_DotV2, 1)
static inline float AM_DotV2(AmVec2 Left, AmVec2 Right)
{
    ASSERT_COVERED(AM_DotV2);
    return (Left.X * Right.X) + (Left.Y * Right.Y);
}

COVERAGE(AM_DotV3, 1)
static inline float AM_DotV3(AmVec3 Left, AmVec3 Right)
{
    ASSERT_COVERED(AM_DotV3);
    return (Left.X * Right.X) + (Left.Y * Right.Y) + (Left.Z * Right.Z);
}

COVERAGE(AM_DotV4, 1)
static inline float AM_DotV4(AmVec4 Left, AmVec4 Right)
{
    ASSERT_COVERED(AM_DotV4);

    float Result;

    // NOTE(zak): IN the future if we wanna check what version SSE is support
    // we can use _mm_dp_ps (4.3) but for now we will use the old way.
    // Or a r = _mm_mul_ps(v1, v2), r = _mm_hadd_ps(r, r), r = _mm_hadd_ps(r, r) for SSE3
#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSEResultOne = _mm_mul_ps(Left.SSE, Right.SSE);
    __m128 SSEResultTwo = _mm_shuffle_ps(SSEResultOne, SSEResultOne, _MM_SHUFFLE(2, 3, 0, 1));
    SSEResultOne = _mm_add_ps(SSEResultOne, SSEResultTwo);
    SSEResultTwo = _mm_shuffle_ps(SSEResultOne, SSEResultOne, _MM_SHUFFLE(0, 1, 2, 3));
    SSEResultOne = _mm_add_ps(SSEResultOne, SSEResultTwo);
    _mm_store_ss(&Result, SSEResultOne);
#else
    Result = ((Left.X * Right.X) + (Left.Z * Right.Z)) + ((Left.Y * Right.Y) + (Left.W * Right.W));
#endif

    return Result;
}

COVERAGE(AM_Cross, 1)
static inline AmVec3 AM_Cross(AmVec3 Left, AmVec3 Right)
{
    ASSERT_COVERED(AM_Cross);

    AmVec3 Result;
    Result.X = (Left.Y * Right.Z) - (Left.Z * Right.Y);
    Result.Y = (Left.Z * Right.X) - (Left.X * Right.Z);
    Result.Z = (Left.X * Right.Y) - (Left.Y * Right.X);

    return Result;
}

/*
 * Unary vector operations
 */

COVERAGE(AM_LenSqrV2, 1)
static inline float AM_LenSqrV2(AmVec2 A)
{
    ASSERT_COVERED(AM_LenSqrV2);
    return AM_DotV2(A, A);
}

COVERAGE(AM_LenSqrV3, 1)
static inline float AM_LenSqrV3(AmVec3 A)
{
    ASSERT_COVERED(AM_LenSqrV3);
    return AM_DotV3(A, A);
}

COVERAGE(AM_LenSqrV4, 1)
static inline float AM_LenSqrV4(AmVec4 A)
{
    ASSERT_COVERED(AM_LenSqrV4);
    return AM_DotV4(A, A);
}

COVERAGE(AM_LenV2, 1)
static inline float AM_LenV2(AmVec2 A)
{
    ASSERT_COVERED(AM_LenV2);
    return AM_SqrtF(AM_LenSqrV2(A));
}

COVERAGE(AM_LenV3, 1)
static inline float AM_LenV3(AmVec3 A)
{
    ASSERT_COVERED(AM_LenV3);
    return AM_SqrtF(AM_LenSqrV3(A));
}

COVERAGE(AM_LenV4, 1)
static inline float AM_LenV4(AmVec4 A)
{
    ASSERT_COVERED(AM_LenV4);
    return AM_SqrtF(AM_LenSqrV4(A));
}

COVERAGE(AM_NormV2, 1)
static inline AmVec2 AM_NormV2(AmVec2 A)
{
    ASSERT_COVERED(AM_NormV2);
    return AM_MulV2F(A, AM_InvSqrtF(AM_DotV2(A, A)));
}

COVERAGE(AM_NormV3, 1)
static inline AmVec3 AM_NormV3(AmVec3 A)
{
    ASSERT_COVERED(AM_NormV3);
    return AM_MulV3F(A, AM_InvSqrtF(AM_DotV3(A, A)));
}

COVERAGE(AM_NormV4, 1)
static inline AmVec4 AM_NormV4(AmVec4 A)
{
    ASSERT_COVERED(AM_NormV4);
    return AM_MulV4F(A, AM_InvSqrtF(AM_DotV4(A, A)));
}

/*
 * Utility vector functions
 */

COVERAGE(AM_LerpV2, 1)
static inline AmVec2 AM_LerpV2(AmVec2 A, float Time, AmVec2 B)
{
    ASSERT_COVERED(AM_LerpV2);
    return AM_AddV2(AM_MulV2F(A, 1.0f - Time), AM_MulV2F(B, Time));
}

COVERAGE(AM_LerpV3, 1)
static inline AmVec3 AM_LerpV3(AmVec3 A, float Time, AmVec3 B)
{
    ASSERT_COVERED(AM_LerpV3);
    return AM_AddV3(AM_MulV3F(A, 1.0f - Time), AM_MulV3F(B, Time));
}

COVERAGE(AM_LerpV4, 1)
static inline AmVec4 AM_LerpV4(AmVec4 A, float Time, AmVec4 B)
{
    ASSERT_COVERED(AM_LerpV4);
    return AM_AddV4(AM_MulV4F(A, 1.0f - Time), AM_MulV4F(B, Time));
}

/*
 * SSE stuff
 */

COVERAGE(AM_LinearCombineV4M4, 1)
static inline AmVec4 AM_LinearCombineV4M4(AmVec4 Left, AmMat4 Right)
{
    ASSERT_COVERED(AM_LinearCombineV4M4);

    AmVec4 Result;
#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_mul_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, 0x00), Right.Columns[0].SSE);
    Result.SSE = _mm_add_ps(Result.SSE, _mm_mul_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, 0x55), Right.Columns[1].SSE));
    Result.SSE = _mm_add_ps(Result.SSE, _mm_mul_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, 0xaa), Right.Columns[2].SSE));
    Result.SSE = _mm_add_ps(Result.SSE, _mm_mul_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, 0xff), Right.Columns[3].SSE));
#else
    Result.X = Left.Elements[0] * Right.Columns[0].X;
    Result.Y = Left.Elements[0] * Right.Columns[0].Y;
    Result.Z = Left.Elements[0] * Right.Columns[0].Z;
    Result.W = Left.Elements[0] * Right.Columns[0].W;

    Result.X += Left.Elements[1] * Right.Columns[1].X;
    Result.Y += Left.Elements[1] * Right.Columns[1].Y;
    Result.Z += Left.Elements[1] * Right.Columns[1].Z;
    Result.W += Left.Elements[1] * Right.Columns[1].W;

    Result.X += Left.Elements[2] * Right.Columns[2].X;
    Result.Y += Left.Elements[2] * Right.Columns[2].Y;
    Result.Z += Left.Elements[2] * Right.Columns[2].Z;
    Result.W += Left.Elements[2] * Right.Columns[2].W;

    Result.X += Left.Elements[3] * Right.Columns[3].X;
    Result.Y += Left.Elements[3] * Right.Columns[3].Y;
    Result.Z += Left.Elements[3] * Right.Columns[3].Z;
    Result.W += Left.Elements[3] * Right.Columns[3].W;
#endif

    return Result;
}

/*
 * 2x2 Matrices
 */

COVERAGE(AM_M2, 1)
static inline AmMat2 AM_M2(void)
{
    ASSERT_COVERED(AM_M2);
    AmMat2 Result = { 0 };
    return Result;
}

COVERAGE(AM_M2D, 1)
static inline AmMat2 AM_M2D(float Diagonal)
{
    ASSERT_COVERED(AM_M2D);

    AmMat2 Result = { 0 };
    Result.Elements[0][0] = Diagonal;
    Result.Elements[1][1] = Diagonal;

    return Result;
}

COVERAGE(AM_TransposeM2, 1)
static inline AmMat2 AM_TransposeM2(AmMat2 Matrix)
{
    ASSERT_COVERED(AM_TransposeM2);

    AmMat2 Result = Matrix;

    Result.Elements[0][1] = Matrix.Elements[1][0];
    Result.Elements[1][0] = Matrix.Elements[0][1];

    return Result;
}

COVERAGE(AM_AddM2, 1)
static inline AmMat2 AM_AddM2(AmMat2 Left, AmMat2 Right)
{
    ASSERT_COVERED(AM_AddM2);

    AmMat2 Result;

    Result.Elements[0][0] = Left.Elements[0][0] + Right.Elements[0][0];
    Result.Elements[0][1] = Left.Elements[0][1] + Right.Elements[0][1];
    Result.Elements[1][0] = Left.Elements[1][0] + Right.Elements[1][0];
    Result.Elements[1][1] = Left.Elements[1][1] + Right.Elements[1][1];

    return Result;
}

COVERAGE(AM_SubM2, 1)
static inline AmMat2 AM_SubM2(AmMat2 Left, AmMat2 Right)
{
    ASSERT_COVERED(AM_SubM2);

    AmMat2 Result;

    Result.Elements[0][0] = Left.Elements[0][0] - Right.Elements[0][0];
    Result.Elements[0][1] = Left.Elements[0][1] - Right.Elements[0][1];
    Result.Elements[1][0] = Left.Elements[1][0] - Right.Elements[1][0];
    Result.Elements[1][1] = Left.Elements[1][1] - Right.Elements[1][1];

    return Result;
}

COVERAGE(AM_MulM2V2, 1)
static inline AmVec2 AM_MulM2V2(AmMat2 Matrix, AmVec2 Vector)
{
    ASSERT_COVERED(AM_MulM2V2);

    AmVec2 Result;

    Result.X = Vector.Elements[0] * Matrix.Columns[0].X;
    Result.Y = Vector.Elements[0] * Matrix.Columns[0].Y;

    Result.X += Vector.Elements[1] * Matrix.Columns[1].X;
    Result.Y += Vector.Elements[1] * Matrix.Columns[1].Y;

    return Result;
}

COVERAGE(AM_MulM2, 1)
static inline AmMat2 AM_MulM2(AmMat2 Left, AmMat2 Right)
{
    ASSERT_COVERED(AM_MulM2);

    AmMat2 Result;
    Result.Columns[0] = AM_MulM2V2(Left, Right.Columns[0]);
    Result.Columns[1] = AM_MulM2V2(Left, Right.Columns[1]);

    return Result;
}

COVERAGE(AM_MulM2F, 1)
static inline AmMat2 AM_MulM2F(AmMat2 Matrix, float Scalar)
{
    ASSERT_COVERED(AM_MulM2F);

    AmMat2 Result;

    Result.Elements[0][0] = Matrix.Elements[0][0] * Scalar;
    Result.Elements[0][1] = Matrix.Elements[0][1] * Scalar;
    Result.Elements[1][0] = Matrix.Elements[1][0] * Scalar;
    Result.Elements[1][1] = Matrix.Elements[1][1] * Scalar;

    return Result;
}

COVERAGE(AM_DivM2F, 1)
static inline AmMat2 AM_DivM2F(AmMat2 Matrix, float Scalar)
{
    ASSERT_COVERED(AM_DivM2F);

    AmMat2 Result;

    Result.Elements[0][0] = Matrix.Elements[0][0] / Scalar;
    Result.Elements[0][1] = Matrix.Elements[0][1] / Scalar;
    Result.Elements[1][0] = Matrix.Elements[1][0] / Scalar;
    Result.Elements[1][1] = Matrix.Elements[1][1] / Scalar;

    return Result;
}

COVERAGE(AM_DeterminantM2, 1)
static inline float AM_DeterminantM2(AmMat2 Matrix)
{
    ASSERT_COVERED(AM_DeterminantM2);
    return Matrix.Elements[0][0] * Matrix.Elements[1][1] - Matrix.Elements[0][1] * Matrix.Elements[1][0];
}

COVERAGE(AM_InvGeneralM2, 1)
static inline AmMat2 AM_InvGeneralM2(AmMat2 Matrix)
{
    ASSERT_COVERED(AM_InvGeneralM2);

    AmMat2 Result;
    float InvDeterminant = 1.0f / AM_DeterminantM2(Matrix);
    Result.Elements[0][0] = InvDeterminant * +Matrix.Elements[1][1];
    Result.Elements[1][1] = InvDeterminant * +Matrix.Elements[0][0];
    Result.Elements[0][1] = InvDeterminant * -Matrix.Elements[0][1];
    Result.Elements[1][0] = InvDeterminant * -Matrix.Elements[1][0];

    return Result;
}

/*
 * 3x3 Matrices
 */

COVERAGE(AM_M3, 1)
static inline AmMat3 AM_M3(void)
{
    ASSERT_COVERED(AM_M3);
    AmMat3 Result = { 0 };
    return Result;
}

COVERAGE(AM_M3D, 1)
static inline AmMat3 AM_M3D(float Diagonal)
{
    ASSERT_COVERED(AM_M3D);

    AmMat3 Result = { 0 };
    Result.Elements[0][0] = Diagonal;
    Result.Elements[1][1] = Diagonal;
    Result.Elements[2][2] = Diagonal;

    return Result;
}

COVERAGE(AM_TransposeM3, 1)
static inline AmMat3 AM_TransposeM3(AmMat3 Matrix)
{
    ASSERT_COVERED(AM_TransposeM3);

    AmMat3 Result = Matrix;

    Result.Elements[0][1] = Matrix.Elements[1][0];
    Result.Elements[0][2] = Matrix.Elements[2][0];
    Result.Elements[1][0] = Matrix.Elements[0][1];
    Result.Elements[1][2] = Matrix.Elements[2][1];
    Result.Elements[2][1] = Matrix.Elements[1][2];
    Result.Elements[2][0] = Matrix.Elements[0][2];

    return Result;
}

COVERAGE(AM_AddM3, 1)
static inline AmMat3 AM_AddM3(AmMat3 Left, AmMat3 Right)
{
    ASSERT_COVERED(AM_AddM3);

    AmMat3 Result;

    Result.Elements[0][0] = Left.Elements[0][0] + Right.Elements[0][0];
    Result.Elements[0][1] = Left.Elements[0][1] + Right.Elements[0][1];
    Result.Elements[0][2] = Left.Elements[0][2] + Right.Elements[0][2];
    Result.Elements[1][0] = Left.Elements[1][0] + Right.Elements[1][0];
    Result.Elements[1][1] = Left.Elements[1][1] + Right.Elements[1][1];
    Result.Elements[1][2] = Left.Elements[1][2] + Right.Elements[1][2];
    Result.Elements[2][0] = Left.Elements[2][0] + Right.Elements[2][0];
    Result.Elements[2][1] = Left.Elements[2][1] + Right.Elements[2][1];
    Result.Elements[2][2] = Left.Elements[2][2] + Right.Elements[2][2];

    return Result;
}

COVERAGE(AM_SubM3, 1)
static inline AmMat3 AM_SubM3(AmMat3 Left, AmMat3 Right)
{
    ASSERT_COVERED(AM_SubM3);

    AmMat3 Result;

    Result.Elements[0][0] = Left.Elements[0][0] - Right.Elements[0][0];
    Result.Elements[0][1] = Left.Elements[0][1] - Right.Elements[0][1];
    Result.Elements[0][2] = Left.Elements[0][2] - Right.Elements[0][2];
    Result.Elements[1][0] = Left.Elements[1][0] - Right.Elements[1][0];
    Result.Elements[1][1] = Left.Elements[1][1] - Right.Elements[1][1];
    Result.Elements[1][2] = Left.Elements[1][2] - Right.Elements[1][2];
    Result.Elements[2][0] = Left.Elements[2][0] - Right.Elements[2][0];
    Result.Elements[2][1] = Left.Elements[2][1] - Right.Elements[2][1];
    Result.Elements[2][2] = Left.Elements[2][2] - Right.Elements[2][2];

    return Result;
}

COVERAGE(AM_MulM3V3, 1)
static inline AmVec3 AM_MulM3V3(AmMat3 Matrix, AmVec3 Vector)
{
    ASSERT_COVERED(AM_MulM3V3);

    AmVec3 Result;

    Result.X = Vector.Elements[0] * Matrix.Columns[0].X;
    Result.Y = Vector.Elements[0] * Matrix.Columns[0].Y;
    Result.Z = Vector.Elements[0] * Matrix.Columns[0].Z;

    Result.X += Vector.Elements[1] * Matrix.Columns[1].X;
    Result.Y += Vector.Elements[1] * Matrix.Columns[1].Y;
    Result.Z += Vector.Elements[1] * Matrix.Columns[1].Z;

    Result.X += Vector.Elements[2] * Matrix.Columns[2].X;
    Result.Y += Vector.Elements[2] * Matrix.Columns[2].Y;
    Result.Z += Vector.Elements[2] * Matrix.Columns[2].Z;

    return Result;
}

COVERAGE(AM_MulM3, 1)
static inline AmMat3 AM_MulM3(AmMat3 Left, AmMat3 Right)
{
    ASSERT_COVERED(AM_MulM3);

    AmMat3 Result;
    Result.Columns[0] = AM_MulM3V3(Left, Right.Columns[0]);
    Result.Columns[1] = AM_MulM3V3(Left, Right.Columns[1]);
    Result.Columns[2] = AM_MulM3V3(Left, Right.Columns[2]);

    return Result;
}

COVERAGE(AM_MulM3F, 1)
static inline AmMat3 AM_MulM3F(AmMat3 Matrix, float Scalar)
{
    ASSERT_COVERED(AM_MulM3F);

    AmMat3 Result;

    Result.Elements[0][0] = Matrix.Elements[0][0] * Scalar;
    Result.Elements[0][1] = Matrix.Elements[0][1] * Scalar;
    Result.Elements[0][2] = Matrix.Elements[0][2] * Scalar;
    Result.Elements[1][0] = Matrix.Elements[1][0] * Scalar;
    Result.Elements[1][1] = Matrix.Elements[1][1] * Scalar;
    Result.Elements[1][2] = Matrix.Elements[1][2] * Scalar;
    Result.Elements[2][0] = Matrix.Elements[2][0] * Scalar;
    Result.Elements[2][1] = Matrix.Elements[2][1] * Scalar;
    Result.Elements[2][2] = Matrix.Elements[2][2] * Scalar;

    return Result;
}

COVERAGE(AM_DivM3, 1)
static inline AmMat3 AM_DivM3F(AmMat3 Matrix, float Scalar)
{
    ASSERT_COVERED(AM_DivM3);

    AmMat3 Result;

    Result.Elements[0][0] = Matrix.Elements[0][0] / Scalar;
    Result.Elements[0][1] = Matrix.Elements[0][1] / Scalar;
    Result.Elements[0][2] = Matrix.Elements[0][2] / Scalar;
    Result.Elements[1][0] = Matrix.Elements[1][0] / Scalar;
    Result.Elements[1][1] = Matrix.Elements[1][1] / Scalar;
    Result.Elements[1][2] = Matrix.Elements[1][2] / Scalar;
    Result.Elements[2][0] = Matrix.Elements[2][0] / Scalar;
    Result.Elements[2][1] = Matrix.Elements[2][1] / Scalar;
    Result.Elements[2][2] = Matrix.Elements[2][2] / Scalar;

    return Result;
}

COVERAGE(AM_DeterminantM3, 1)
static inline float AM_DeterminantM3(AmMat3 Matrix)
{
    ASSERT_COVERED(AM_DeterminantM3);

    AmMat3 Cross;
    Cross.Columns[0] = AM_Cross(Matrix.Columns[1], Matrix.Columns[2]);
    Cross.Columns[1] = AM_Cross(Matrix.Columns[2], Matrix.Columns[0]);
    Cross.Columns[2] = AM_Cross(Matrix.Columns[0], Matrix.Columns[1]);

    return AM_DotV3(Cross.Columns[2], Matrix.Columns[2]);
}

COVERAGE(AM_InvGeneralM3, 1)
static inline AmMat3 AM_InvGeneralM3(AmMat3 Matrix)
{
    ASSERT_COVERED(AM_InvGeneralM3);

    AmMat3 Cross;
    Cross.Columns[0] = AM_Cross(Matrix.Columns[1], Matrix.Columns[2]);
    Cross.Columns[1] = AM_Cross(Matrix.Columns[2], Matrix.Columns[0]);
    Cross.Columns[2] = AM_Cross(Matrix.Columns[0], Matrix.Columns[1]);

    float InvDeterminant = 1.0f / AM_DotV3(Cross.Columns[2], Matrix.Columns[2]);

    AmMat3 Result;
    Result.Columns[0] = AM_MulV3F(Cross.Columns[0], InvDeterminant);
    Result.Columns[1] = AM_MulV3F(Cross.Columns[1], InvDeterminant);
    Result.Columns[2] = AM_MulV3F(Cross.Columns[2], InvDeterminant);

    return AM_TransposeM3(Result);
}

/*
 * 4x4 Matrices
 */

COVERAGE(AM_M4, 1)
static inline AmMat4 AM_M4(void)
{
    ASSERT_COVERED(AM_M4);
    AmMat4 Result = { 0 };
    return Result;
}

COVERAGE(AM_M4D, 1)
static inline AmMat4 AM_M4D(float Diagonal)
{
    ASSERT_COVERED(AM_M4D);

    AmMat4 Result = { 0 };
    Result.Elements[0][0] = Diagonal;
    Result.Elements[1][1] = Diagonal;
    Result.Elements[2][2] = Diagonal;
    Result.Elements[3][3] = Diagonal;

    return Result;
}

COVERAGE(AM_TransposeM4, 1)
static inline AmMat4 AM_TransposeM4(AmMat4 Matrix)
{
    ASSERT_COVERED(AM_TransposeM4);

    AmMat4 Result = Matrix;
#ifdef HANDMADE_MATH__USE_SSE
    _MM_TRANSPOSE4_PS(Result.Columns[0].SSE, Result.Columns[1].SSE, Result.Columns[2].SSE, Result.Columns[3].SSE);
#else
    Result.Elements[0][1] = Matrix.Elements[1][0];
    Result.Elements[0][2] = Matrix.Elements[2][0];
    Result.Elements[0][3] = Matrix.Elements[3][0];
    Result.Elements[1][0] = Matrix.Elements[0][1];
    Result.Elements[1][2] = Matrix.Elements[2][1];
    Result.Elements[1][3] = Matrix.Elements[3][1];
    Result.Elements[2][1] = Matrix.Elements[1][2];
    Result.Elements[2][0] = Matrix.Elements[0][2];
    Result.Elements[2][3] = Matrix.Elements[3][2];
    Result.Elements[3][1] = Matrix.Elements[1][3];
    Result.Elements[3][2] = Matrix.Elements[2][3];
    Result.Elements[3][0] = Matrix.Elements[0][3];
#endif

    return Result;
}

COVERAGE(AM_AddM4, 1)
static inline AmMat4 AM_AddM4(AmMat4 Left, AmMat4 Right)
{
    ASSERT_COVERED(AM_AddM4);

    AmMat4 Result;

#ifdef HANDMADE_MATH__USE_SSE
    Result.Columns[0].SSE = _mm_add_ps(Left.Columns[0].SSE, Right.Columns[0].SSE);
    Result.Columns[1].SSE = _mm_add_ps(Left.Columns[1].SSE, Right.Columns[1].SSE);
    Result.Columns[2].SSE = _mm_add_ps(Left.Columns[2].SSE, Right.Columns[2].SSE);
    Result.Columns[3].SSE = _mm_add_ps(Left.Columns[3].SSE, Right.Columns[3].SSE);
#else
    Result.Elements[0][0] = Left.Elements[0][0] + Right.Elements[0][0];
    Result.Elements[0][1] = Left.Elements[0][1] + Right.Elements[0][1];
    Result.Elements[0][2] = Left.Elements[0][2] + Right.Elements[0][2];
    Result.Elements[0][3] = Left.Elements[0][3] + Right.Elements[0][3];
    Result.Elements[1][0] = Left.Elements[1][0] + Right.Elements[1][0];
    Result.Elements[1][1] = Left.Elements[1][1] + Right.Elements[1][1];
    Result.Elements[1][2] = Left.Elements[1][2] + Right.Elements[1][2];
    Result.Elements[1][3] = Left.Elements[1][3] + Right.Elements[1][3];
    Result.Elements[2][0] = Left.Elements[2][0] + Right.Elements[2][0];
    Result.Elements[2][1] = Left.Elements[2][1] + Right.Elements[2][1];
    Result.Elements[2][2] = Left.Elements[2][2] + Right.Elements[2][2];
    Result.Elements[2][3] = Left.Elements[2][3] + Right.Elements[2][3];
    Result.Elements[3][0] = Left.Elements[3][0] + Right.Elements[3][0];
    Result.Elements[3][1] = Left.Elements[3][1] + Right.Elements[3][1];
    Result.Elements[3][2] = Left.Elements[3][2] + Right.Elements[3][2];
    Result.Elements[3][3] = Left.Elements[3][3] + Right.Elements[3][3];
#endif

    return Result;
}

COVERAGE(AM_SubM4, 1)
static inline AmMat4 AM_SubM4(AmMat4 Left, AmMat4 Right)
{
    ASSERT_COVERED(AM_SubM4);

    AmMat4 Result;

#ifdef HANDMADE_MATH__USE_SSE
    Result.Columns[0].SSE = _mm_sub_ps(Left.Columns[0].SSE, Right.Columns[0].SSE);
    Result.Columns[1].SSE = _mm_sub_ps(Left.Columns[1].SSE, Right.Columns[1].SSE);
    Result.Columns[2].SSE = _mm_sub_ps(Left.Columns[2].SSE, Right.Columns[2].SSE);
    Result.Columns[3].SSE = _mm_sub_ps(Left.Columns[3].SSE, Right.Columns[3].SSE);
#else
    Result.Elements[0][0] = Left.Elements[0][0] - Right.Elements[0][0];
    Result.Elements[0][1] = Left.Elements[0][1] - Right.Elements[0][1];
    Result.Elements[0][2] = Left.Elements[0][2] - Right.Elements[0][2];
    Result.Elements[0][3] = Left.Elements[0][3] - Right.Elements[0][3];
    Result.Elements[1][0] = Left.Elements[1][0] - Right.Elements[1][0];
    Result.Elements[1][1] = Left.Elements[1][1] - Right.Elements[1][1];
    Result.Elements[1][2] = Left.Elements[1][2] - Right.Elements[1][2];
    Result.Elements[1][3] = Left.Elements[1][3] - Right.Elements[1][3];
    Result.Elements[2][0] = Left.Elements[2][0] - Right.Elements[2][0];
    Result.Elements[2][1] = Left.Elements[2][1] - Right.Elements[2][1];
    Result.Elements[2][2] = Left.Elements[2][2] - Right.Elements[2][2];
    Result.Elements[2][3] = Left.Elements[2][3] - Right.Elements[2][3];
    Result.Elements[3][0] = Left.Elements[3][0] - Right.Elements[3][0];
    Result.Elements[3][1] = Left.Elements[3][1] - Right.Elements[3][1];
    Result.Elements[3][2] = Left.Elements[3][2] - Right.Elements[3][2];
    Result.Elements[3][3] = Left.Elements[3][3] - Right.Elements[3][3];
#endif

    return Result;
}

COVERAGE(AM_MulM4, 1)
static inline AmMat4 AM_MulM4(AmMat4 Left, AmMat4 Right)
{
    ASSERT_COVERED(AM_MulM4);

    AmMat4 Result;
    Result.Columns[0] = AM_LinearCombineV4M4(Right.Columns[0], Left);
    Result.Columns[1] = AM_LinearCombineV4M4(Right.Columns[1], Left);
    Result.Columns[2] = AM_LinearCombineV4M4(Right.Columns[2], Left);
    Result.Columns[3] = AM_LinearCombineV4M4(Right.Columns[3], Left);

    return Result;
}

COVERAGE(AM_MulM4F, 1)
static inline AmMat4 AM_MulM4F(AmMat4 Matrix, float Scalar)
{
    ASSERT_COVERED(AM_MulM4F);

    AmMat4 Result;

#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSEScalar = _mm_set1_ps(Scalar);
    Result.Columns[0].SSE = _mm_mul_ps(Matrix.Columns[0].SSE, SSEScalar);
    Result.Columns[1].SSE = _mm_mul_ps(Matrix.Columns[1].SSE, SSEScalar);
    Result.Columns[2].SSE = _mm_mul_ps(Matrix.Columns[2].SSE, SSEScalar);
    Result.Columns[3].SSE = _mm_mul_ps(Matrix.Columns[3].SSE, SSEScalar);
#else
    Result.Elements[0][0] = Matrix.Elements[0][0] * Scalar;
    Result.Elements[0][1] = Matrix.Elements[0][1] * Scalar;
    Result.Elements[0][2] = Matrix.Elements[0][2] * Scalar;
    Result.Elements[0][3] = Matrix.Elements[0][3] * Scalar;
    Result.Elements[1][0] = Matrix.Elements[1][0] * Scalar;
    Result.Elements[1][1] = Matrix.Elements[1][1] * Scalar;
    Result.Elements[1][2] = Matrix.Elements[1][2] * Scalar;
    Result.Elements[1][3] = Matrix.Elements[1][3] * Scalar;
    Result.Elements[2][0] = Matrix.Elements[2][0] * Scalar;
    Result.Elements[2][1] = Matrix.Elements[2][1] * Scalar;
    Result.Elements[2][2] = Matrix.Elements[2][2] * Scalar;
    Result.Elements[2][3] = Matrix.Elements[2][3] * Scalar;
    Result.Elements[3][0] = Matrix.Elements[3][0] * Scalar;
    Result.Elements[3][1] = Matrix.Elements[3][1] * Scalar;
    Result.Elements[3][2] = Matrix.Elements[3][2] * Scalar;
    Result.Elements[3][3] = Matrix.Elements[3][3] * Scalar;
#endif

    return Result;
}

COVERAGE(AM_MulM4V4, 1)
static inline AmVec4 AM_MulM4V4(AmMat4 Matrix, AmVec4 Vector)
{
    ASSERT_COVERED(AM_MulM4V4);
    return AM_LinearCombineV4M4(Vector, Matrix);
}

COVERAGE(AM_DivM4F, 1)
static inline AmMat4 AM_DivM4F(AmMat4 Matrix, float Scalar)
{
    ASSERT_COVERED(AM_DivM4F);

    AmMat4 Result;

#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSEScalar = _mm_set1_ps(Scalar);
    Result.Columns[0].SSE = _mm_div_ps(Matrix.Columns[0].SSE, SSEScalar);
    Result.Columns[1].SSE = _mm_div_ps(Matrix.Columns[1].SSE, SSEScalar);
    Result.Columns[2].SSE = _mm_div_ps(Matrix.Columns[2].SSE, SSEScalar);
    Result.Columns[3].SSE = _mm_div_ps(Matrix.Columns[3].SSE, SSEScalar);
#else
    Result.Elements[0][0] = Matrix.Elements[0][0] / Scalar;
    Result.Elements[0][1] = Matrix.Elements[0][1] / Scalar;
    Result.Elements[0][2] = Matrix.Elements[0][2] / Scalar;
    Result.Elements[0][3] = Matrix.Elements[0][3] / Scalar;
    Result.Elements[1][0] = Matrix.Elements[1][0] / Scalar;
    Result.Elements[1][1] = Matrix.Elements[1][1] / Scalar;
    Result.Elements[1][2] = Matrix.Elements[1][2] / Scalar;
    Result.Elements[1][3] = Matrix.Elements[1][3] / Scalar;
    Result.Elements[2][0] = Matrix.Elements[2][0] / Scalar;
    Result.Elements[2][1] = Matrix.Elements[2][1] / Scalar;
    Result.Elements[2][2] = Matrix.Elements[2][2] / Scalar;
    Result.Elements[2][3] = Matrix.Elements[2][3] / Scalar;
    Result.Elements[3][0] = Matrix.Elements[3][0] / Scalar;
    Result.Elements[3][1] = Matrix.Elements[3][1] / Scalar;
    Result.Elements[3][2] = Matrix.Elements[3][2] / Scalar;
    Result.Elements[3][3] = Matrix.Elements[3][3] / Scalar;
#endif

    return Result;
}

COVERAGE(AM_DeterminantM4, 1)
static inline float AM_DeterminantM4(AmMat4 Matrix)
{
    ASSERT_COVERED(AM_DeterminantM4);

    AmVec3 C01 = AM_Cross(Matrix.Columns[0].XYZ, Matrix.Columns[1].XYZ);
    AmVec3 C23 = AM_Cross(Matrix.Columns[2].XYZ, Matrix.Columns[3].XYZ);
    AmVec3 B10 = AM_SubV3(AM_MulV3F(Matrix.Columns[0].XYZ, Matrix.Columns[1].W), AM_MulV3F(Matrix.Columns[1].XYZ, Matrix.Columns[0].W));
    AmVec3 B32 = AM_SubV3(AM_MulV3F(Matrix.Columns[2].XYZ, Matrix.Columns[3].W), AM_MulV3F(Matrix.Columns[3].XYZ, Matrix.Columns[2].W));

    return AM_DotV3(C01, B32) + AM_DotV3(C23, B10);
}

COVERAGE(AM_InvGeneralM4, 1)
// Returns a general-purpose inverse of an AmMat4. Note that special-purpose inverses of many transformations
// are available and will be more efficient.
static inline AmMat4 AM_InvGeneralM4(AmMat4 Matrix)
{
    ASSERT_COVERED(AM_InvGeneralM4);

    AmVec3 C01 = AM_Cross(Matrix.Columns[0].XYZ, Matrix.Columns[1].XYZ);
    AmVec3 C23 = AM_Cross(Matrix.Columns[2].XYZ, Matrix.Columns[3].XYZ);
    AmVec3 B10 = AM_SubV3(AM_MulV3F(Matrix.Columns[0].XYZ, Matrix.Columns[1].W), AM_MulV3F(Matrix.Columns[1].XYZ, Matrix.Columns[0].W));
    AmVec3 B32 = AM_SubV3(AM_MulV3F(Matrix.Columns[2].XYZ, Matrix.Columns[3].W), AM_MulV3F(Matrix.Columns[3].XYZ, Matrix.Columns[2].W));

    float InvDeterminant = 1.0f / (AM_DotV3(C01, B32) + AM_DotV3(C23, B10));
    C01 = AM_MulV3F(C01, InvDeterminant);
    C23 = AM_MulV3F(C23, InvDeterminant);
    B10 = AM_MulV3F(B10, InvDeterminant);
    B32 = AM_MulV3F(B32, InvDeterminant);

    AmMat4 Result;
    Result.Columns[0] =
        AM_V4V(AM_AddV3(AM_Cross(Matrix.Columns[1].XYZ, B32), AM_MulV3F(C23, Matrix.Columns[1].W)), -AM_DotV3(Matrix.Columns[1].XYZ, C23));
    Result.Columns[1] =
        AM_V4V(AM_SubV3(AM_Cross(B32, Matrix.Columns[0].XYZ), AM_MulV3F(C23, Matrix.Columns[0].W)), +AM_DotV3(Matrix.Columns[0].XYZ, C23));
    Result.Columns[2] =
        AM_V4V(AM_AddV3(AM_Cross(Matrix.Columns[3].XYZ, B10), AM_MulV3F(C01, Matrix.Columns[3].W)), -AM_DotV3(Matrix.Columns[3].XYZ, C01));
    Result.Columns[3] =
        AM_V4V(AM_SubV3(AM_Cross(B10, Matrix.Columns[2].XYZ), AM_MulV3F(C01, Matrix.Columns[2].W)), +AM_DotV3(Matrix.Columns[2].XYZ, C01));

    return AM_TransposeM4(Result);
}

/*
 * Common graphics transformations
 */

COVERAGE(AM_Orthographic_RH_NO, 1)
// Produces a right-handed orthographic projection matrix with Z ranging from -1 to 1 (the GL convention).
// Left, Right, Bottom, and Top specify the coordinates of their respective clipping planes.
// Near and Far specify the distances to the near and far clipping planes.
static inline AmMat4 AM_Orthographic_RH_NO(float Left, float Right, float Bottom, float Top, float Near, float Far)
{
    ASSERT_COVERED(AM_Orthographic_RH_NO);

    AmMat4 Result = { 0 };

    Result.Elements[0][0] = 2.0f / (Right - Left);
    Result.Elements[1][1] = 2.0f / (Top - Bottom);
    Result.Elements[2][2] = 2.0f / (Near - Far);
    Result.Elements[3][3] = 1.0f;

    Result.Elements[3][0] = (Left + Right) / (Left - Right);
    Result.Elements[3][1] = (Bottom + Top) / (Bottom - Top);
    Result.Elements[3][2] = (Near + Far) / (Near - Far);

    return Result;
}

COVERAGE(AM_Orthographic_RH_ZO, 1)
// Produces a right-handed orthographic projection matrix with Z ranging from 0 to 1 (the DirectX convention).
// Left, Right, Bottom, and Top specify the coordinates of their respective clipping planes.
// Near and Far specify the distances to the near and far clipping planes.
static inline AmMat4 AM_Orthographic_RH_ZO(float Left, float Right, float Bottom, float Top, float Near, float Far)
{
    ASSERT_COVERED(AM_Orthographic_RH_ZO);

    AmMat4 Result = { 0 };

    Result.Elements[0][0] = 2.0f / (Right - Left);
    Result.Elements[1][1] = 2.0f / (Top - Bottom);
    Result.Elements[2][2] = 1.0f / (Near - Far);
    Result.Elements[3][3] = 1.0f;

    Result.Elements[3][0] = (Left + Right) / (Left - Right);
    Result.Elements[3][1] = (Bottom + Top) / (Bottom - Top);
    Result.Elements[3][2] = (Near) / (Near - Far);

    return Result;
}

COVERAGE(AM_Orthographic_LH_NO, 1)
// Produces a left-handed orthographic projection matrix with Z ranging from -1 to 1 (the GL convention).
// Left, Right, Bottom, and Top specify the coordinates of their respective clipping planes.
// Near and Far specify the distances to the near and far clipping planes.
static inline AmMat4 AM_Orthographic_LH_NO(float Left, float Right, float Bottom, float Top, float Near, float Far)
{
    ASSERT_COVERED(AM_Orthographic_LH_NO);

    AmMat4 Result = AM_Orthographic_RH_NO(Left, Right, Bottom, Top, Near, Far);
    Result.Elements[2][2] = -Result.Elements[2][2];

    return Result;
}

COVERAGE(AM_Orthographic_LH_ZO, 1)
// Produces a left-handed orthographic projection matrix with Z ranging from 0 to 1 (the DirectX convention).
// Left, Right, Bottom, and Top specify the coordinates of their respective clipping planes.
// Near and Far specify the distances to the near and far clipping planes.
static inline AmMat4 AM_Orthographic_LH_ZO(float Left, float Right, float Bottom, float Top, float Near, float Far)
{
    ASSERT_COVERED(AM_Orthographic_LH_ZO);

    AmMat4 Result = AM_Orthographic_RH_ZO(Left, Right, Bottom, Top, Near, Far);
    Result.Elements[2][2] = -Result.Elements[2][2];

    return Result;
}

COVERAGE(AM_InvOrthographic, 1)
// Returns an inverse for the given orthographic projection matrix. Works for all orthographic
// projection matrices, regardless of handedness or NDC convention.
static inline AmMat4 AM_InvOrthographic(AmMat4 OrthoMatrix)
{
    ASSERT_COVERED(AM_InvOrthographic);

    AmMat4 Result = { 0 };
    Result.Elements[0][0] = 1.0f / OrthoMatrix.Elements[0][0];
    Result.Elements[1][1] = 1.0f / OrthoMatrix.Elements[1][1];
    Result.Elements[2][2] = 1.0f / OrthoMatrix.Elements[2][2];
    Result.Elements[3][3] = 1.0f;

    Result.Elements[3][0] = -OrthoMatrix.Elements[3][0] * Result.Elements[0][0];
    Result.Elements[3][1] = -OrthoMatrix.Elements[3][1] * Result.Elements[1][1];
    Result.Elements[3][2] = -OrthoMatrix.Elements[3][2] * Result.Elements[2][2];

    return Result;
}

COVERAGE(AM_Perspective_RH_NO, 1)
static inline AmMat4 AM_Perspective_RH_NO(float FOV, float AspectRatio, float Near, float Far)
{
    ASSERT_COVERED(AM_Perspective_RH_NO);

    AmMat4 Result = { 0 };

    // See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml

    float Cotangent = 1.0f / AM_TanF(FOV / 2.0f);
    Result.Elements[0][0] = Cotangent / AspectRatio;
    Result.Elements[1][1] = Cotangent;
    Result.Elements[2][3] = -1.0f;

    Result.Elements[2][2] = (Near + Far) / (Near - Far);
    Result.Elements[3][2] = (2.0f * Near * Far) / (Near - Far);

    return Result;
}

COVERAGE(AM_Perspective_RH_ZO, 1)
static inline AmMat4 AM_Perspective_RH_ZO(float FOV, float AspectRatio, float Near, float Far)
{
    ASSERT_COVERED(AM_Perspective_RH_ZO);

    AmMat4 Result = { 0 };

    // See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml

    float Cotangent = 1.0f / AM_TanF(FOV / 2.0f);
    Result.Elements[0][0] = Cotangent / AspectRatio;
    Result.Elements[1][1] = Cotangent;
    Result.Elements[2][3] = -1.0f;

    Result.Elements[2][2] = (Far) / (Near - Far);
    Result.Elements[3][2] = (Near * Far) / (Near - Far);

    return Result;
}

COVERAGE(AM_Perspective_LH_NO, 1)
static inline AmMat4 AM_Perspective_LH_NO(float FOV, float AspectRatio, float Near, float Far)
{
    ASSERT_COVERED(AM_Perspective_LH_NO);

    AmMat4 Result = AM_Perspective_RH_NO(FOV, AspectRatio, Near, Far);
    Result.Elements[2][2] = -Result.Elements[2][2];
    Result.Elements[2][3] = -Result.Elements[2][3];

    return Result;
}

COVERAGE(AM_Perspective_LH_ZO, 1)
static inline AmMat4 AM_Perspective_LH_ZO(float FOV, float AspectRatio, float Near, float Far)
{
    ASSERT_COVERED(AM_Perspective_LH_ZO);

    AmMat4 Result = AM_Perspective_RH_ZO(FOV, AspectRatio, Near, Far);
    Result.Elements[2][2] = -Result.Elements[2][2];
    Result.Elements[2][3] = -Result.Elements[2][3];

    return Result;
}

COVERAGE(AM_InvPerspective_RH, 1)
static inline AmMat4 AM_InvPerspective_RH(AmMat4 PerspectiveMatrix)
{
    ASSERT_COVERED(AM_InvPerspective_RH);

    AmMat4 Result = { 0 };
    Result.Elements[0][0] = 1.0f / PerspectiveMatrix.Elements[0][0];
    Result.Elements[1][1] = 1.0f / PerspectiveMatrix.Elements[1][1];
    Result.Elements[2][2] = 0.0f;

    Result.Elements[2][3] = 1.0f / PerspectiveMatrix.Elements[3][2];
    Result.Elements[3][3] = PerspectiveMatrix.Elements[2][2] * Result.Elements[2][3];
    Result.Elements[3][2] = PerspectiveMatrix.Elements[2][3];

    return Result;
}

COVERAGE(AM_InvPerspective_LH, 1)
static inline AmMat4 AM_InvPerspective_LH(AmMat4 PerspectiveMatrix)
{
    ASSERT_COVERED(AM_InvPerspective_LH);

    AmMat4 Result = { 0 };
    Result.Elements[0][0] = 1.0f / PerspectiveMatrix.Elements[0][0];
    Result.Elements[1][1] = 1.0f / PerspectiveMatrix.Elements[1][1];
    Result.Elements[2][2] = 0.0f;

    Result.Elements[2][3] = 1.0f / PerspectiveMatrix.Elements[3][2];
    Result.Elements[3][3] = PerspectiveMatrix.Elements[2][2] * -Result.Elements[2][3];
    Result.Elements[3][2] = PerspectiveMatrix.Elements[2][3];

    return Result;
}

COVERAGE(AM_Translate, 1)
static inline AmMat4 AM_Translate(AmVec3 Translation)
{
    ASSERT_COVERED(AM_Translate);

    AmMat4 Result = AM_M4D(1.0f);
    Result.Elements[3][0] = Translation.X;
    Result.Elements[3][1] = Translation.Y;
    Result.Elements[3][2] = Translation.Z;

    return Result;
}

COVERAGE(AM_InvTranslate, 1)
static inline AmMat4 AM_InvTranslate(AmMat4 TranslationMatrix)
{
    ASSERT_COVERED(AM_InvTranslate);

    AmMat4 Result = TranslationMatrix;
    Result.Elements[3][0] = -Result.Elements[3][0];
    Result.Elements[3][1] = -Result.Elements[3][1];
    Result.Elements[3][2] = -Result.Elements[3][2];

    return Result;
}

COVERAGE(AM_Rotate_RH, 1)
static inline AmMat4 AM_Rotate_RH(float Angle, AmVec3 Axis)
{
    ASSERT_COVERED(AM_Rotate_RH);

    AmMat4 Result = AM_M4D(1.0f);

    Axis = AM_NormV3(Axis);

    float SinTheta = AM_SinF(Angle);
    float CosTheta = AM_CosF(Angle);
    float CosValue = 1.0f - CosTheta;

    Result.Elements[0][0] = (Axis.X * Axis.X * CosValue) + CosTheta;
    Result.Elements[0][1] = (Axis.X * Axis.Y * CosValue) + (Axis.Z * SinTheta);
    Result.Elements[0][2] = (Axis.X * Axis.Z * CosValue) - (Axis.Y * SinTheta);

    Result.Elements[1][0] = (Axis.Y * Axis.X * CosValue) - (Axis.Z * SinTheta);
    Result.Elements[1][1] = (Axis.Y * Axis.Y * CosValue) + CosTheta;
    Result.Elements[1][2] = (Axis.Y * Axis.Z * CosValue) + (Axis.X * SinTheta);

    Result.Elements[2][0] = (Axis.Z * Axis.X * CosValue) + (Axis.Y * SinTheta);
    Result.Elements[2][1] = (Axis.Z * Axis.Y * CosValue) - (Axis.X * SinTheta);
    Result.Elements[2][2] = (Axis.Z * Axis.Z * CosValue) + CosTheta;

    return Result;
}

COVERAGE(AM_Rotate_LH, 1)
static inline AmMat4 AM_Rotate_LH(float Angle, AmVec3 Axis)
{
    ASSERT_COVERED(AM_Rotate_LH);
    /* NOTE(lcf): Matrix will be inverse/transpose of RH. */
    return AM_Rotate_RH(-Angle, Axis);
}

COVERAGE(AM_InvRotate, 1)
static inline AmMat4 AM_InvRotate(AmMat4 RotationMatrix)
{
    ASSERT_COVERED(AM_InvRotate);
    return AM_TransposeM4(RotationMatrix);
}

COVERAGE(AM_Scale, 1)
static inline AmMat4 AM_Scale(AmVec3 Scale)
{
    ASSERT_COVERED(AM_Scale);

    AmMat4 Result = AM_M4D(1.0f);
    Result.Elements[0][0] = Scale.X;
    Result.Elements[1][1] = Scale.Y;
    Result.Elements[2][2] = Scale.Z;

    return Result;
}

COVERAGE(AM_InvScale, 1)
static inline AmMat4 AM_InvScale(AmMat4 ScaleMatrix)
{
    ASSERT_COVERED(AM_InvScale);

    AmMat4 Result = ScaleMatrix;
    Result.Elements[0][0] = 1.0f / Result.Elements[0][0];
    Result.Elements[1][1] = 1.0f / Result.Elements[1][1];
    Result.Elements[2][2] = 1.0f / Result.Elements[2][2];

    return Result;
}

static inline AmMat4 _AM_LookAt(AmVec3 F, AmVec3 S, AmVec3 U, AmVec3 Eye)
{
    AmMat4 Result;

    Result.Elements[0][0] = S.X;
    Result.Elements[0][1] = U.X;
    Result.Elements[0][2] = -F.X;
    Result.Elements[0][3] = 0.0f;

    Result.Elements[1][0] = S.Y;
    Result.Elements[1][1] = U.Y;
    Result.Elements[1][2] = -F.Y;
    Result.Elements[1][3] = 0.0f;

    Result.Elements[2][0] = S.Z;
    Result.Elements[2][1] = U.Z;
    Result.Elements[2][2] = -F.Z;
    Result.Elements[2][3] = 0.0f;

    Result.Elements[3][0] = -AM_DotV3(S, Eye);
    Result.Elements[3][1] = -AM_DotV3(U, Eye);
    Result.Elements[3][2] = AM_DotV3(F, Eye);
    Result.Elements[3][3] = 1.0f;

    return Result;
}

COVERAGE(AM_LookAt_RH, 1)
static inline AmMat4 AM_LookAt_RH(AmVec3 Eye, AmVec3 Center, AmVec3 Up)
{
    ASSERT_COVERED(AM_LookAt_RH);

    AmVec3 F = AM_NormV3(AM_SubV3(Center, Eye));
    AmVec3 S = AM_NormV3(AM_Cross(F, Up));
    AmVec3 U = AM_Cross(S, F);

    return _AM_LookAt(F, S, U, Eye);
}

COVERAGE(AM_LookAt_LH, 1)
static inline AmMat4 AM_LookAt_LH(AmVec3 Eye, AmVec3 Center, AmVec3 Up)
{
    ASSERT_COVERED(AM_LookAt_LH);

    AmVec3 F = AM_NormV3(AM_SubV3(Eye, Center));
    AmVec3 S = AM_NormV3(AM_Cross(F, Up));
    AmVec3 U = AM_Cross(S, F);

    return _AM_LookAt(F, S, U, Eye);
}

COVERAGE(AM_InvLookAt, 1)
static inline AmMat4 AM_InvLookAt(AmMat4 Matrix)
{
    ASSERT_COVERED(AM_InvLookAt);
    AmMat4 Result;

    AmMat3 Rotation = { 0 };
    Rotation.Columns[0] = Matrix.Columns[0].XYZ;
    Rotation.Columns[1] = Matrix.Columns[1].XYZ;
    Rotation.Columns[2] = Matrix.Columns[2].XYZ;
    Rotation = AM_TransposeM3(Rotation);

    Result.Columns[0] = AM_V4V(Rotation.Columns[0], 0.0f);
    Result.Columns[1] = AM_V4V(Rotation.Columns[1], 0.0f);
    Result.Columns[2] = AM_V4V(Rotation.Columns[2], 0.0f);
    Result.Columns[3] = AM_MulV4F(Matrix.Columns[3], -1.0f);
    Result.Elements[3][0] = -1.0f * Matrix.Elements[3][0] / (Rotation.Elements[0][0] + Rotation.Elements[0][1] + Rotation.Elements[0][2]);
    Result.Elements[3][1] = -1.0f * Matrix.Elements[3][1] / (Rotation.Elements[1][0] + Rotation.Elements[1][1] + Rotation.Elements[1][2]);
    Result.Elements[3][2] = -1.0f * Matrix.Elements[3][2] / (Rotation.Elements[2][0] + Rotation.Elements[2][1] + Rotation.Elements[2][2]);
    Result.Elements[3][3] = 1.0f;

    return Result;
}

/*
 * Quaternion operations
 */

COVERAGE(AM_Q, 1)
static inline AmQuat AM_Q(float X, float Y, float Z, float W)
{
    ASSERT_COVERED(AM_Q);

    AmQuat Result;

#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_setr_ps(X, Y, Z, W);
#else
    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;
    Result.W = W;
#endif

    return Result;
}

COVERAGE(AM_QV4, 1)
static inline AmQuat AM_QV4(AmVec4 Vector)
{
    ASSERT_COVERED(AM_QV4);

    AmQuat Result;

#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = Vector.SSE;
#else
    Result.X = Vector.X;
    Result.Y = Vector.Y;
    Result.Z = Vector.Z;
    Result.W = Vector.W;
#endif

    return Result;
}

COVERAGE(AM_AddQ, 1)
static inline AmQuat AM_AddQ(AmQuat Left, AmQuat Right)
{
    ASSERT_COVERED(AM_AddQ);

    AmQuat Result;

#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_add_ps(Left.SSE, Right.SSE);
#else

    Result.X = Left.X + Right.X;
    Result.Y = Left.Y + Right.Y;
    Result.Z = Left.Z + Right.Z;
    Result.W = Left.W + Right.W;
#endif

    return Result;
}

COVERAGE(AM_SubQ, 1)
static inline AmQuat AM_SubQ(AmQuat Left, AmQuat Right)
{
    ASSERT_COVERED(AM_SubQ);

    AmQuat Result;

#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_sub_ps(Left.SSE, Right.SSE);
#else
    Result.X = Left.X - Right.X;
    Result.Y = Left.Y - Right.Y;
    Result.Z = Left.Z - Right.Z;
    Result.W = Left.W - Right.W;
#endif

    return Result;
}

COVERAGE(AM_MulQ, 1)
static inline AmQuat AM_MulQ(AmQuat Left, AmQuat Right)
{
    ASSERT_COVERED(AM_MulQ);

    AmQuat Result;

#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSEResultOne = _mm_xor_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, _MM_SHUFFLE(0, 0, 0, 0)), _mm_setr_ps(0.f, -0.f, 0.f, -0.f));
    __m128 SSEResultTwo = _mm_shuffle_ps(Right.SSE, Right.SSE, _MM_SHUFFLE(0, 1, 2, 3));
    __m128 SSEResultThree = _mm_mul_ps(SSEResultTwo, SSEResultOne);

    SSEResultOne = _mm_xor_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, _MM_SHUFFLE(1, 1, 1, 1)), _mm_setr_ps(0.f, 0.f, -0.f, -0.f));
    SSEResultTwo = _mm_shuffle_ps(Right.SSE, Right.SSE, _MM_SHUFFLE(1, 0, 3, 2));
    SSEResultThree = _mm_add_ps(SSEResultThree, _mm_mul_ps(SSEResultTwo, SSEResultOne));

    SSEResultOne = _mm_xor_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, _MM_SHUFFLE(2, 2, 2, 2)), _mm_setr_ps(-0.f, 0.f, 0.f, -0.f));
    SSEResultTwo = _mm_shuffle_ps(Right.SSE, Right.SSE, _MM_SHUFFLE(2, 3, 0, 1));
    SSEResultThree = _mm_add_ps(SSEResultThree, _mm_mul_ps(SSEResultTwo, SSEResultOne));

    SSEResultOne = _mm_shuffle_ps(Left.SSE, Left.SSE, _MM_SHUFFLE(3, 3, 3, 3));
    SSEResultTwo = _mm_shuffle_ps(Right.SSE, Right.SSE, _MM_SHUFFLE(3, 2, 1, 0));
    Result.SSE = _mm_add_ps(SSEResultThree, _mm_mul_ps(SSEResultTwo, SSEResultOne));
#else
    Result.X = Right.Elements[3] * +Left.Elements[0];
    Result.Y = Right.Elements[2] * -Left.Elements[0];
    Result.Z = Right.Elements[1] * +Left.Elements[0];
    Result.W = Right.Elements[0] * -Left.Elements[0];

    Result.X += Right.Elements[2] * +Left.Elements[1];
    Result.Y += Right.Elements[3] * +Left.Elements[1];
    Result.Z += Right.Elements[0] * -Left.Elements[1];
    Result.W += Right.Elements[1] * -Left.Elements[1];

    Result.X += Right.Elements[1] * -Left.Elements[2];
    Result.Y += Right.Elements[0] * +Left.Elements[2];
    Result.Z += Right.Elements[3] * +Left.Elements[2];
    Result.W += Right.Elements[2] * -Left.Elements[2];

    Result.X += Right.Elements[0] * +Left.Elements[3];
    Result.Y += Right.Elements[1] * +Left.Elements[3];
    Result.Z += Right.Elements[2] * +Left.Elements[3];
    Result.W += Right.Elements[3] * +Left.Elements[3];
#endif

    return Result;
}

COVERAGE(AM_MulQF, 1)
static inline AmQuat AM_MulQF(AmQuat Left, float Multiplicative)
{
    ASSERT_COVERED(AM_MulQF);

    AmQuat Result;

#ifdef HANDMADE_MATH__USE_SSE
    __m128 Scalar = _mm_set1_ps(Multiplicative);
    Result.SSE = _mm_mul_ps(Left.SSE, Scalar);
#else
    Result.X = Left.X * Multiplicative;
    Result.Y = Left.Y * Multiplicative;
    Result.Z = Left.Z * Multiplicative;
    Result.W = Left.W * Multiplicative;
#endif

    return Result;
}

COVERAGE(AM_DivQF, 1)
static inline AmQuat AM_DivQF(AmQuat Left, float Divnd)
{
    ASSERT_COVERED(AM_DivQF);

    AmQuat Result;

#ifdef HANDMADE_MATH__USE_SSE
    __m128 Scalar = _mm_set1_ps(Divnd);
    Result.SSE = _mm_div_ps(Left.SSE, Scalar);
#else
    Result.X = Left.X / Divnd;
    Result.Y = Left.Y / Divnd;
    Result.Z = Left.Z / Divnd;
    Result.W = Left.W / Divnd;
#endif

    return Result;
}

COVERAGE(AM_DotQ, 1)
static inline float AM_DotQ(AmQuat Left, AmQuat Right)
{
    ASSERT_COVERED(AM_DotQ);

    float Result;

#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSEResultOne = _mm_mul_ps(Left.SSE, Right.SSE);
    __m128 SSEResultTwo = _mm_shuffle_ps(SSEResultOne, SSEResultOne, _MM_SHUFFLE(2, 3, 0, 1));
    SSEResultOne = _mm_add_ps(SSEResultOne, SSEResultTwo);
    SSEResultTwo = _mm_shuffle_ps(SSEResultOne, SSEResultOne, _MM_SHUFFLE(0, 1, 2, 3));
    SSEResultOne = _mm_add_ps(SSEResultOne, SSEResultTwo);
    _mm_store_ss(&Result, SSEResultOne);
#else
    Result = ((Left.X * Right.X) + (Left.Z * Right.Z)) + ((Left.Y * Right.Y) + (Left.W * Right.W));
#endif

    return Result;
}

COVERAGE(AM_InvQ, 1)
static inline AmQuat AM_InvQ(AmQuat Left)
{
    ASSERT_COVERED(AM_InvQ);

    AmQuat Result;
    Result.X = -Left.X;
    Result.Y = -Left.Y;
    Result.Z = -Left.Z;
    Result.W = Left.W;

    return AM_DivQF(Result, (AM_DotQ(Left, Left)));
}

COVERAGE(AM_NormQ, 1)
static inline AmQuat AM_NormQ(AmQuat Quat)
{
    ASSERT_COVERED(AM_NormQ);

    /* NOTE(lcf): Take advantage of SSE implementation in AM_NormV4 */
    AmVec4 Vec = { Quat.X, Quat.Y, Quat.Z, Quat.W };
    Vec = AM_NormV4(Vec);
    AmQuat Result = { Vec.X, Vec.Y, Vec.Z, Vec.W };

    return Result;
}

static inline AmQuat _AM_MixQ(AmQuat Left, float MixLeft, AmQuat Right, float MixRight)
{
    AmQuat Result;

#ifdef HANDMADE_MATH__USE_SSE
    __m128 ScalarLeft = _mm_set1_ps(MixLeft);
    __m128 ScalarRight = _mm_set1_ps(MixRight);
    __m128 SSEResultOne = _mm_mul_ps(Left.SSE, ScalarLeft);
    __m128 SSEResultTwo = _mm_mul_ps(Right.SSE, ScalarRight);
    Result.SSE = _mm_add_ps(SSEResultOne, SSEResultTwo);
#else
    Result.X = Left.X * MixLeft + Right.X * MixRight;
    Result.Y = Left.Y * MixLeft + Right.Y * MixRight;
    Result.Z = Left.Z * MixLeft + Right.Z * MixRight;
    Result.W = Left.W * MixLeft + Right.W * MixRight;
#endif

    return Result;
}

COVERAGE(AM_NLerp, 1)
static inline AmQuat AM_NLerp(AmQuat Left, float Time, AmQuat Right)
{
    ASSERT_COVERED(AM_NLerp);

    AmQuat Result = _AM_MixQ(Left, 1.0f - Time, Right, Time);
    Result = AM_NormQ(Result);

    return Result;
}

COVERAGE(AM_SLerp, 1)
static inline AmQuat AM_SLerp(AmQuat Left, float Time, AmQuat Right)
{
    ASSERT_COVERED(AM_SLerp);

    AmQuat Result;

    float Cos_Theta = AM_DotQ(Left, Right);

    if (Cos_Theta < 0.0f)
    { /* NOTE(lcf): Take shortest path on Hyper-sphere */
        Cos_Theta = -Cos_Theta;
        Right = AM_Q(-Right.X, -Right.Y, -Right.Z, -Right.W);
    }

    /* NOTE(lcf): Use Normalized Linear interpolation when vectors are roughly not L.I. */
    if (Cos_Theta > 0.9995f)
    {
        Result = AM_NLerp(Left, Time, Right);
    }
    else
    {
        float Angle = AM_ACosF(Cos_Theta);
        float MixLeft = AM_SinF((1.0f - Time) * Angle);
        float MixRight = AM_SinF(Time * Angle);

        Result = _AM_MixQ(Left, MixLeft, Right, MixRight);
        Result = AM_NormQ(Result);
    }

    return Result;
}

COVERAGE(AM_QToM4, 1)
static inline AmMat4 AM_QToM4(AmQuat Left)
{
    ASSERT_COVERED(AM_QToM4);

    AmMat4 Result;

    AmQuat NormalizedQ = AM_NormQ(Left);

    float XX, YY, ZZ, XY, XZ, YZ, WX, WY, WZ;

    XX = NormalizedQ.X * NormalizedQ.X;
    YY = NormalizedQ.Y * NormalizedQ.Y;
    ZZ = NormalizedQ.Z * NormalizedQ.Z;
    XY = NormalizedQ.X * NormalizedQ.Y;
    XZ = NormalizedQ.X * NormalizedQ.Z;
    YZ = NormalizedQ.Y * NormalizedQ.Z;
    WX = NormalizedQ.W * NormalizedQ.X;
    WY = NormalizedQ.W * NormalizedQ.Y;
    WZ = NormalizedQ.W * NormalizedQ.Z;

    Result.Elements[0][0] = 1.0f - 2.0f * (YY + ZZ);
    Result.Elements[0][1] = 2.0f * (XY + WZ);
    Result.Elements[0][2] = 2.0f * (XZ - WY);
    Result.Elements[0][3] = 0.0f;

    Result.Elements[1][0] = 2.0f * (XY - WZ);
    Result.Elements[1][1] = 1.0f - 2.0f * (XX + ZZ);
    Result.Elements[1][2] = 2.0f * (YZ + WX);
    Result.Elements[1][3] = 0.0f;

    Result.Elements[2][0] = 2.0f * (XZ + WY);
    Result.Elements[2][1] = 2.0f * (YZ - WX);
    Result.Elements[2][2] = 1.0f - 2.0f * (XX + YY);
    Result.Elements[2][3] = 0.0f;

    Result.Elements[3][0] = 0.0f;
    Result.Elements[3][1] = 0.0f;
    Result.Elements[3][2] = 0.0f;
    Result.Elements[3][3] = 1.0f;

    return Result;
}

// This method taken from Mike Day at Insomniac Games.
// https://d3cw3dd2w32x2b.cloudfront.net/wp-content/uploads/2015/01/matrix-to-quat.pdf
//
// Note that as mentioned at the top of the paper, the paper assumes the matrix
// would be *post*-multiplied to a vector to rotate it, meaning the matrix is
// the transpose of what we're dealing with. But, because our matrices are
// stored in column-major order, the indices *appear* to match the paper.
//
// For example, m12 in the paper is row 1, column 2. We need to transpose it to
// row 2, column 1. But, because the column comes first when referencing
// elements, it looks like M.Elements[1][2].
//
// Don't be confused! Or if you must be confused, at least trust this
// comment. :)
COVERAGE(AM_M4ToQ_RH, 4)
static inline AmQuat AM_M4ToQ_RH(AmMat4 M)
{
    float T;
    AmQuat Q;

    if (M.Elements[2][2] < 0.0f)
    {
        if (M.Elements[0][0] > M.Elements[1][1])
        {
            ASSERT_COVERED(AM_M4ToQ_RH);

            T = 1 + M.Elements[0][0] - M.Elements[1][1] - M.Elements[2][2];
            Q = AM_Q(T, M.Elements[0][1] + M.Elements[1][0], M.Elements[2][0] + M.Elements[0][2], M.Elements[1][2] - M.Elements[2][1]);
        }
        else
        {
            ASSERT_COVERED(AM_M4ToQ_RH);

            T = 1 - M.Elements[0][0] + M.Elements[1][1] - M.Elements[2][2];
            Q = AM_Q(M.Elements[0][1] + M.Elements[1][0], T, M.Elements[1][2] + M.Elements[2][1], M.Elements[2][0] - M.Elements[0][2]);
        }
    }
    else
    {
        if (M.Elements[0][0] < -M.Elements[1][1])
        {
            ASSERT_COVERED(AM_M4ToQ_RH);

            T = 1 - M.Elements[0][0] - M.Elements[1][1] + M.Elements[2][2];
            Q = AM_Q(M.Elements[2][0] + M.Elements[0][2], M.Elements[1][2] + M.Elements[2][1], T, M.Elements[0][1] - M.Elements[1][0]);
        }
        else
        {
            ASSERT_COVERED(AM_M4ToQ_RH);

            T = 1 + M.Elements[0][0] + M.Elements[1][1] + M.Elements[2][2];
            Q = AM_Q(M.Elements[1][2] - M.Elements[2][1], M.Elements[2][0] - M.Elements[0][2], M.Elements[0][1] - M.Elements[1][0], T);
        }
    }

    Q = AM_MulQF(Q, 0.5f / AM_SqrtF(T));

    return Q;
}

COVERAGE(AM_M4ToQ_LH, 4)
static inline AmQuat AM_M4ToQ_LH(AmMat4 M)
{
    float T;
    AmQuat Q;

    if (M.Elements[2][2] < 0.0f)
    {
        if (M.Elements[0][0] > M.Elements[1][1])
        {
            ASSERT_COVERED(AM_M4ToQ_LH);

            T = 1 + M.Elements[0][0] - M.Elements[1][1] - M.Elements[2][2];
            Q = AM_Q(T, M.Elements[0][1] + M.Elements[1][0], M.Elements[2][0] + M.Elements[0][2], M.Elements[2][1] - M.Elements[1][2]);
        }
        else
        {
            ASSERT_COVERED(AM_M4ToQ_LH);

            T = 1 - M.Elements[0][0] + M.Elements[1][1] - M.Elements[2][2];
            Q = AM_Q(M.Elements[0][1] + M.Elements[1][0], T, M.Elements[1][2] + M.Elements[2][1], M.Elements[0][2] - M.Elements[2][0]);
        }
    }
    else
    {
        if (M.Elements[0][0] < -M.Elements[1][1])
        {
            ASSERT_COVERED(AM_M4ToQ_LH);

            T = 1 - M.Elements[0][0] - M.Elements[1][1] + M.Elements[2][2];
            Q = AM_Q(M.Elements[2][0] + M.Elements[0][2], M.Elements[1][2] + M.Elements[2][1], T, M.Elements[1][0] - M.Elements[0][1]);
        }
        else
        {
            ASSERT_COVERED(AM_M4ToQ_LH);

            T = 1 + M.Elements[0][0] + M.Elements[1][1] + M.Elements[2][2];
            Q = AM_Q(M.Elements[2][1] - M.Elements[1][2], M.Elements[0][2] - M.Elements[2][0], M.Elements[1][0] - M.Elements[0][2], T);
        }
    }

    Q = AM_MulQF(Q, 0.5f / AM_SqrtF(T));

    return Q;
}

COVERAGE(AM_QFromAxisAngle_RH, 1)
static inline AmQuat AM_QFromAxisAngle_RH(AmVec3 Axis, float AngleOfRotation)
{
    ASSERT_COVERED(AM_QFromAxisAngle_RH);

    AmQuat Result;

    AmVec3 AxisNormalized = AM_NormV3(Axis);
    float SineOfRotation = AM_SinF(AngleOfRotation / 2.0f);

    Result.XYZ = AM_MulV3F(AxisNormalized, SineOfRotation);
    Result.W = AM_CosF(AngleOfRotation / 2.0f);

    return Result;
}

COVERAGE(AM_QFromAxisAngle_LH, 1)
static inline AmQuat AM_QFromAxisAngle_LH(AmVec3 Axis, float AngleOfRotation)
{
    ASSERT_COVERED(AM_QFromAxisAngle_LH);

    return AM_QFromAxisAngle_RH(Axis, -AngleOfRotation);
}

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

COVERAGE(AM_LenV2CPP, 1)
static inline float AM_Len(AmVec2 A)
{
    ASSERT_COVERED(AM_LenV2CPP);
    return AM_LenV2(A);
}

COVERAGE(AM_LenV3CPP, 1)
static inline float AM_Len(AmVec3 A)
{
    ASSERT_COVERED(AM_LenV3CPP);
    return AM_LenV3(A);
}

COVERAGE(AM_LenV4CPP, 1)
static inline float AM_Len(AmVec4 A)
{
    ASSERT_COVERED(AM_LenV4CPP);
    return AM_LenV4(A);
}

COVERAGE(AM_LenSqrV2CPP, 1)
static inline float AM_LenSqr(AmVec2 A)
{
    ASSERT_COVERED(AM_LenSqrV2CPP);
    return AM_LenSqrV2(A);
}

COVERAGE(AM_LenSqrV3CPP, 1)
static inline float AM_LenSqr(AmVec3 A)
{
    ASSERT_COVERED(AM_LenSqrV3CPP);
    return AM_LenSqrV3(A);
}

COVERAGE(AM_LenSqrV4CPP, 1)
static inline float AM_LenSqr(AmVec4 A)
{
    ASSERT_COVERED(AM_LenSqrV4CPP);
    return AM_LenSqrV4(A);
}

COVERAGE(AM_NormV2CPP, 1)
static inline AmVec2 AM_Norm(AmVec2 A)
{
    ASSERT_COVERED(AM_NormV2CPP);
    return AM_NormV2(A);
}

COVERAGE(AM_NormV3CPP, 1)
static inline AmVec3 AM_Norm(AmVec3 A)
{
    ASSERT_COVERED(AM_NormV3CPP);
    return AM_NormV3(A);
}

COVERAGE(AM_NormV4CPP, 1)
static inline AmVec4 AM_Norm(AmVec4 A)
{
    ASSERT_COVERED(AM_NormV4CPP);
    return AM_NormV4(A);
}

COVERAGE(AM_NormQCPP, 1)
static inline AmQuat AM_Norm(AmQuat A)
{
    ASSERT_COVERED(AM_NormQCPP);
    return AM_NormQ(A);
}

COVERAGE(AM_DotV2CPP, 1)
static inline float AM_Dot(AmVec2 Left, AmVec2 VecTwo)
{
    ASSERT_COVERED(AM_DotV2CPP);
    return AM_DotV2(Left, VecTwo);
}

COVERAGE(AM_DotV3CPP, 1)
static inline float AM_Dot(AmVec3 Left, AmVec3 VecTwo)
{
    ASSERT_COVERED(AM_DotV3CPP);
    return AM_DotV3(Left, VecTwo);
}

COVERAGE(AM_DotV4CPP, 1)
static inline float AM_Dot(AmVec4 Left, AmVec4 VecTwo)
{
    ASSERT_COVERED(AM_DotV4CPP);
    return AM_DotV4(Left, VecTwo);
}

COVERAGE(AM_LerpV2CPP, 1)
static inline AmVec2 AM_Lerp(AmVec2 Left, float Time, AmVec2 Right)
{
    ASSERT_COVERED(AM_LerpV2CPP);
    return AM_LerpV2(Left, Time, Right);
}

COVERAGE(AM_LerpV3CPP, 1)
static inline AmVec3 AM_Lerp(AmVec3 Left, float Time, AmVec3 Right)
{
    ASSERT_COVERED(AM_LerpV3CPP);
    return AM_LerpV3(Left, Time, Right);
}

COVERAGE(AM_LerpV4CPP, 1)
static inline AmVec4 AM_Lerp(AmVec4 Left, float Time, AmVec4 Right)
{
    ASSERT_COVERED(AM_LerpV4CPP);
    return AM_LerpV4(Left, Time, Right);
}

COVERAGE(AM_TransposeM2CPP, 1)
static inline AmMat2 AM_Transpose(AmMat2 Matrix)
{
    ASSERT_COVERED(AM_TransposeM2CPP);
    return AM_TransposeM2(Matrix);
}

COVERAGE(AM_TransposeM3CPP, 1)
static inline AmMat3 AM_Transpose(AmMat3 Matrix)
{
    ASSERT_COVERED(AM_TransposeM3CPP);
    return AM_TransposeM3(Matrix);
}

COVERAGE(AM_TransposeM4CPP, 1)
static inline AmMat4 AM_Transpose(AmMat4 Matrix)
{
    ASSERT_COVERED(AM_TransposeM4CPP);
    return AM_TransposeM4(Matrix);
}

COVERAGE(AM_DeterminantM2CPP, 1)
static inline float AM_Determinant(AmMat2 Matrix)
{
    ASSERT_COVERED(AM_DeterminantM2CPP);
    return AM_DeterminantM2(Matrix);
}

COVERAGE(AM_DeterminantM3CPP, 1)
static inline float AM_Determinant(AmMat3 Matrix)
{
    ASSERT_COVERED(AM_DeterminantM3CPP);
    return AM_DeterminantM3(Matrix);
}

COVERAGE(AM_DeterminantM4CPP, 1)
static inline float AM_Determinant(AmMat4 Matrix)
{
    ASSERT_COVERED(AM_DeterminantM4CPP);
    return AM_DeterminantM4(Matrix);
}

COVERAGE(AM_InvGeneralM2CPP, 1)
static inline AmMat2 AM_InvGeneral(AmMat2 Matrix)
{
    ASSERT_COVERED(AM_InvGeneralM2CPP);
    return AM_InvGeneralM2(Matrix);
}

COVERAGE(AM_InvGeneralM3CPP, 1)
static inline AmMat3 AM_InvGeneral(AmMat3 Matrix)
{
    ASSERT_COVERED(AM_InvGeneralM3CPP);
    return AM_InvGeneralM3(Matrix);
}

COVERAGE(AM_InvGeneralM4CPP, 1)
static inline AmMat4 AM_InvGeneral(AmMat4 Matrix)
{
    ASSERT_COVERED(AM_InvGeneralM4CPP);
    return AM_InvGeneralM4(Matrix);
}

COVERAGE(AM_DotQCPP, 1)
static inline float AM_Dot(AmQuat QuatOne, AmQuat QuatTwo)
{
    ASSERT_COVERED(AM_DotQCPP);
    return AM_DotQ(QuatOne, QuatTwo);
}

COVERAGE(AM_AddV2CPP, 1)
static inline AmVec2 AM_Add(AmVec2 Left, AmVec2 Right)
{
    ASSERT_COVERED(AM_AddV2CPP);
    return AM_AddV2(Left, Right);
}

COVERAGE(AM_AddV3CPP, 1)
static inline AmVec3 AM_Add(AmVec3 Left, AmVec3 Right)
{
    ASSERT_COVERED(AM_AddV3CPP);
    return AM_AddV3(Left, Right);
}

COVERAGE(AM_AddV4CPP, 1)
static inline AmVec4 AM_Add(AmVec4 Left, AmVec4 Right)
{
    ASSERT_COVERED(AM_AddV4CPP);
    return AM_AddV4(Left, Right);
}

COVERAGE(AM_AddM2CPP, 1)
static inline AmMat2 AM_Add(AmMat2 Left, AmMat2 Right)
{
    ASSERT_COVERED(AM_AddM2CPP);
    return AM_AddM2(Left, Right);
}

COVERAGE(AM_AddM3CPP, 1)
static inline AmMat3 AM_Add(AmMat3 Left, AmMat3 Right)
{
    ASSERT_COVERED(AM_AddM3CPP);
    return AM_AddM3(Left, Right);
}

COVERAGE(AM_AddM4CPP, 1)
static inline AmMat4 AM_Add(AmMat4 Left, AmMat4 Right)
{
    ASSERT_COVERED(AM_AddM4CPP);
    return AM_AddM4(Left, Right);
}

COVERAGE(AM_AddQCPP, 1)
static inline AmQuat AM_Add(AmQuat Left, AmQuat Right)
{
    ASSERT_COVERED(AM_AddQCPP);
    return AM_AddQ(Left, Right);
}

COVERAGE(AM_SubV2CPP, 1)
static inline AmVec2 AM_Sub(AmVec2 Left, AmVec2 Right)
{
    ASSERT_COVERED(AM_SubV2CPP);
    return AM_SubV2(Left, Right);
}

COVERAGE(AM_SubV3CPP, 1)
static inline AmVec3 AM_Sub(AmVec3 Left, AmVec3 Right)
{
    ASSERT_COVERED(AM_SubV3CPP);
    return AM_SubV3(Left, Right);
}

COVERAGE(AM_SubV4CPP, 1)
static inline AmVec4 AM_Sub(AmVec4 Left, AmVec4 Right)
{
    ASSERT_COVERED(AM_SubV4CPP);
    return AM_SubV4(Left, Right);
}

COVERAGE(AM_SubM2CPP, 1)
static inline AmMat2 AM_Sub(AmMat2 Left, AmMat2 Right)
{
    ASSERT_COVERED(AM_SubM2CPP);
    return AM_SubM2(Left, Right);
}

COVERAGE(AM_SubM3CPP, 1)
static inline AmMat3 AM_Sub(AmMat3 Left, AmMat3 Right)
{
    ASSERT_COVERED(AM_SubM3CPP);
    return AM_SubM3(Left, Right);
}

COVERAGE(AM_SubM4CPP, 1)
static inline AmMat4 AM_Sub(AmMat4 Left, AmMat4 Right)
{
    ASSERT_COVERED(AM_SubM4CPP);
    return AM_SubM4(Left, Right);
}

COVERAGE(AM_SubQCPP, 1)
static inline AmQuat AM_Sub(AmQuat Left, AmQuat Right)
{
    ASSERT_COVERED(AM_SubQCPP);
    return AM_SubQ(Left, Right);
}

COVERAGE(AM_MulV2CPP, 1)
static inline AmVec2 AM_Mul(AmVec2 Left, AmVec2 Right)
{
    ASSERT_COVERED(AM_MulV2CPP);
    return AM_MulV2(Left, Right);
}

COVERAGE(AM_MulV2FCPP, 1)
static inline AmVec2 AM_Mul(AmVec2 Left, float Right)
{
    ASSERT_COVERED(AM_MulV2FCPP);
    return AM_MulV2F(Left, Right);
}

COVERAGE(AM_MulV3CPP, 1)
static inline AmVec3 AM_Mul(AmVec3 Left, AmVec3 Right)
{
    ASSERT_COVERED(AM_MulV3CPP);
    return AM_MulV3(Left, Right);
}

COVERAGE(AM_MulV3FCPP, 1)
static inline AmVec3 AM_Mul(AmVec3 Left, float Right)
{
    ASSERT_COVERED(AM_MulV3FCPP);
    return AM_MulV3F(Left, Right);
}

COVERAGE(AM_MulV4CPP, 1)
static inline AmVec4 AM_Mul(AmVec4 Left, AmVec4 Right)
{
    ASSERT_COVERED(AM_MulV4CPP);
    return AM_MulV4(Left, Right);
}

COVERAGE(AM_MulV4FCPP, 1)
static inline AmVec4 AM_Mul(AmVec4 Left, float Right)
{
    ASSERT_COVERED(AM_MulV4FCPP);
    return AM_MulV4F(Left, Right);
}

COVERAGE(AM_MulM2CPP, 1)
static inline AmMat2 AM_Mul(AmMat2 Left, AmMat2 Right)
{
    ASSERT_COVERED(AM_MulM2CPP);
    return AM_MulM2(Left, Right);
}

COVERAGE(AM_MulM3CPP, 1)
static inline AmMat3 AM_Mul(AmMat3 Left, AmMat3 Right)
{
    ASSERT_COVERED(AM_MulM3CPP);
    return AM_MulM3(Left, Right);
}

COVERAGE(AM_MulM4CPP, 1)
static inline AmMat4 AM_Mul(AmMat4 Left, AmMat4 Right)
{
    ASSERT_COVERED(AM_MulM4CPP);
    return AM_MulM4(Left, Right);
}

COVERAGE(AM_MulM2FCPP, 1)
static inline AmMat2 AM_Mul(AmMat2 Left, float Right)
{
    ASSERT_COVERED(AM_MulM2FCPP);
    return AM_MulM2F(Left, Right);
}

COVERAGE(AM_MulM3FCPP, 1)
static inline AmMat3 AM_Mul(AmMat3 Left, float Right)
{
    ASSERT_COVERED(AM_MulM3FCPP);
    return AM_MulM3F(Left, Right);
}

COVERAGE(AM_MulM4FCPP, 1)
static inline AmMat4 AM_Mul(AmMat4 Left, float Right)
{
    ASSERT_COVERED(AM_MulM4FCPP);
    return AM_MulM4F(Left, Right);
}

COVERAGE(AM_MulM2V2CPP, 1)
static inline AmVec2 AM_Mul(AmMat2 Matrix, AmVec2 Vector)
{
    ASSERT_COVERED(AM_MulM2V2CPP);
    return AM_MulM2V2(Matrix, Vector);
}

COVERAGE(AM_MulM3V3CPP, 1)
static inline AmVec3 AM_Mul(AmMat3 Matrix, AmVec3 Vector)
{
    ASSERT_COVERED(AM_MulM3V3CPP);
    return AM_MulM3V3(Matrix, Vector);
}

COVERAGE(AM_MulM4V4CPP, 1)
static inline AmVec4 AM_Mul(AmMat4 Matrix, AmVec4 Vector)
{
    ASSERT_COVERED(AM_MulM4V4CPP);
    return AM_MulM4V4(Matrix, Vector);
}

COVERAGE(AM_MulQCPP, 1)
static inline AmQuat AM_Mul(AmQuat Left, AmQuat Right)
{
    ASSERT_COVERED(AM_MulQCPP);
    return AM_MulQ(Left, Right);
}

COVERAGE(AM_MulQFCPP, 1)
static inline AmQuat AM_Mul(AmQuat Left, float Right)
{
    ASSERT_COVERED(AM_MulQFCPP);
    return AM_MulQF(Left, Right);
}

COVERAGE(AM_DivV2CPP, 1)
static inline AmVec2 AM_Div(AmVec2 Left, AmVec2 Right)
{
    ASSERT_COVERED(AM_DivV2CPP);
    return AM_DivV2(Left, Right);
}

COVERAGE(AM_DivV2FCPP, 1)
static inline AmVec2 AM_Div(AmVec2 Left, float Right)
{
    ASSERT_COVERED(AM_DivV2FCPP);
    return AM_DivV2F(Left, Right);
}

COVERAGE(AM_DivV3CPP, 1)
static inline AmVec3 AM_Div(AmVec3 Left, AmVec3 Right)
{
    ASSERT_COVERED(AM_DivV3CPP);
    return AM_DivV3(Left, Right);
}

COVERAGE(AM_DivV3FCPP, 1)
static inline AmVec3 AM_Div(AmVec3 Left, float Right)
{
    ASSERT_COVERED(AM_DivV3FCPP);
    return AM_DivV3F(Left, Right);
}

COVERAGE(AM_DivV4CPP, 1)
static inline AmVec4 AM_Div(AmVec4 Left, AmVec4 Right)
{
    ASSERT_COVERED(AM_DivV4CPP);
    return AM_DivV4(Left, Right);
}

COVERAGE(AM_DivV4FCPP, 1)
static inline AmVec4 AM_Div(AmVec4 Left, float Right)
{
    ASSERT_COVERED(AM_DivV4FCPP);
    return AM_DivV4F(Left, Right);
}

COVERAGE(AM_DivM2FCPP, 1)
static inline AmMat2 AM_Div(AmMat2 Left, float Right)
{
    ASSERT_COVERED(AM_DivM2FCPP);
    return AM_DivM2F(Left, Right);
}

COVERAGE(AM_DivM3FCPP, 1)
static inline AmMat3 AM_Div(AmMat3 Left, float Right)
{
    ASSERT_COVERED(AM_DivM3FCPP);
    return AM_DivM3F(Left, Right);
}

COVERAGE(AM_DivM4FCPP, 1)
static inline AmMat4 AM_Div(AmMat4 Left, float Right)
{
    ASSERT_COVERED(AM_DivM4FCPP);
    return AM_DivM4F(Left, Right);
}

COVERAGE(AM_DivQFCPP, 1)
static inline AmQuat AM_Div(AmQuat Left, float Right)
{
    ASSERT_COVERED(AM_DivQFCPP);
    return AM_DivQF(Left, Right);
}

COVERAGE(AM_EqV2CPP, 1)
static inline AmBool AM_Eq(AmVec2 Left, AmVec2 Right)
{
    ASSERT_COVERED(AM_EqV2CPP);
    return AM_EqV2(Left, Right);
}

COVERAGE(AM_EqV3CPP, 1)
static inline AmBool AM_Eq(AmVec3 Left, AmVec3 Right)
{
    ASSERT_COVERED(AM_EqV3CPP);
    return AM_EqV3(Left, Right);
}

COVERAGE(AM_EqV4CPP, 1)
static inline AmBool AM_Eq(AmVec4 Left, AmVec4 Right)
{
    ASSERT_COVERED(AM_EqV4CPP);
    return AM_EqV4(Left, Right);
}

COVERAGE(AM_AddV2Op, 1)
static inline AmVec2 operator+(AmVec2 Left, AmVec2 Right)
{
    ASSERT_COVERED(AM_AddV2Op);
    return AM_AddV2(Left, Right);
}

COVERAGE(AM_AddV3Op, 1)
static inline AmVec3 operator+(AmVec3 Left, AmVec3 Right)
{
    ASSERT_COVERED(AM_AddV3Op);
    return AM_AddV3(Left, Right);
}

COVERAGE(AM_AddV4Op, 1)
static inline AmVec4 operator+(AmVec4 Left, AmVec4 Right)
{
    ASSERT_COVERED(AM_AddV4Op);
    return AM_AddV4(Left, Right);
}

COVERAGE(AM_AddM2Op, 1)
static inline AmMat2 operator+(AmMat2 Left, AmMat2 Right)
{
    ASSERT_COVERED(AM_AddM2Op);
    return AM_AddM2(Left, Right);
}

COVERAGE(AM_AddM3Op, 1)
static inline AmMat3 operator+(AmMat3 Left, AmMat3 Right)
{
    ASSERT_COVERED(AM_AddM3Op);
    return AM_AddM3(Left, Right);
}

COVERAGE(AM_AddM4Op, 1)
static inline AmMat4 operator+(AmMat4 Left, AmMat4 Right)
{
    ASSERT_COVERED(AM_AddM4Op);
    return AM_AddM4(Left, Right);
}

COVERAGE(AM_AddQOp, 1)
static inline AmQuat operator+(AmQuat Left, AmQuat Right)
{
    ASSERT_COVERED(AM_AddQOp);
    return AM_AddQ(Left, Right);
}

COVERAGE(AM_SubV2Op, 1)
static inline AmVec2 operator-(AmVec2 Left, AmVec2 Right)
{
    ASSERT_COVERED(AM_SubV2Op);
    return AM_SubV2(Left, Right);
}

COVERAGE(AM_SubV3Op, 1)
static inline AmVec3 operator-(AmVec3 Left, AmVec3 Right)
{
    ASSERT_COVERED(AM_SubV3Op);
    return AM_SubV3(Left, Right);
}

COVERAGE(AM_SubV4Op, 1)
static inline AmVec4 operator-(AmVec4 Left, AmVec4 Right)
{
    ASSERT_COVERED(AM_SubV4Op);
    return AM_SubV4(Left, Right);
}

COVERAGE(AM_SubM2Op, 1)
static inline AmMat2 operator-(AmMat2 Left, AmMat2 Right)
{
    ASSERT_COVERED(AM_SubM2Op);
    return AM_SubM2(Left, Right);
}

COVERAGE(AM_SubM3Op, 1)
static inline AmMat3 operator-(AmMat3 Left, AmMat3 Right)
{
    ASSERT_COVERED(AM_SubM3Op);
    return AM_SubM3(Left, Right);
}

COVERAGE(AM_SubM4Op, 1)
static inline AmMat4 operator-(AmMat4 Left, AmMat4 Right)
{
    ASSERT_COVERED(AM_SubM4Op);
    return AM_SubM4(Left, Right);
}

COVERAGE(AM_SubQOp, 1)
static inline AmQuat operator-(AmQuat Left, AmQuat Right)
{
    ASSERT_COVERED(AM_SubQOp);
    return AM_SubQ(Left, Right);
}

COVERAGE(AM_MulV2Op, 1)
static inline AmVec2 operator*(AmVec2 Left, AmVec2 Right)
{
    ASSERT_COVERED(AM_MulV2Op);
    return AM_MulV2(Left, Right);
}

COVERAGE(AM_MulV3Op, 1)
static inline AmVec3 operator*(AmVec3 Left, AmVec3 Right)
{
    ASSERT_COVERED(AM_MulV3Op);
    return AM_MulV3(Left, Right);
}

COVERAGE(AM_MulV4Op, 1)
static inline AmVec4 operator*(AmVec4 Left, AmVec4 Right)
{
    ASSERT_COVERED(AM_MulV4Op);
    return AM_MulV4(Left, Right);
}

COVERAGE(AM_MulM2Op, 1)
static inline AmMat2 operator*(AmMat2 Left, AmMat2 Right)
{
    ASSERT_COVERED(AM_MulM2Op);
    return AM_MulM2(Left, Right);
}

COVERAGE(AM_MulM3Op, 1)
static inline AmMat3 operator*(AmMat3 Left, AmMat3 Right)
{
    ASSERT_COVERED(AM_MulM3Op);
    return AM_MulM3(Left, Right);
}

COVERAGE(AM_MulM4Op, 1)
static inline AmMat4 operator*(AmMat4 Left, AmMat4 Right)
{
    ASSERT_COVERED(AM_MulM4Op);
    return AM_MulM4(Left, Right);
}

COVERAGE(AM_MulQOp, 1)
static inline AmQuat operator*(AmQuat Left, AmQuat Right)
{
    ASSERT_COVERED(AM_MulQOp);
    return AM_MulQ(Left, Right);
}

COVERAGE(AM_MulV2FOp, 1)
static inline AmVec2 operator*(AmVec2 Left, float Right)
{
    ASSERT_COVERED(AM_MulV2FOp);
    return AM_MulV2F(Left, Right);
}

COVERAGE(AM_MulV3FOp, 1)
static inline AmVec3 operator*(AmVec3 Left, float Right)
{
    ASSERT_COVERED(AM_MulV3FOp);
    return AM_MulV3F(Left, Right);
}

COVERAGE(AM_MulV4FOp, 1)
static inline AmVec4 operator*(AmVec4 Left, float Right)
{
    ASSERT_COVERED(AM_MulV4FOp);
    return AM_MulV4F(Left, Right);
}

COVERAGE(AM_MulM2FOp, 1)
static inline AmMat2 operator*(AmMat2 Left, float Right)
{
    ASSERT_COVERED(AM_MulM2FOp);
    return AM_MulM2F(Left, Right);
}

COVERAGE(AM_MulM3FOp, 1)
static inline AmMat3 operator*(AmMat3 Left, float Right)
{
    ASSERT_COVERED(AM_MulM3FOp);
    return AM_MulM3F(Left, Right);
}

COVERAGE(AM_MulM4FOp, 1)
static inline AmMat4 operator*(AmMat4 Left, float Right)
{
    ASSERT_COVERED(AM_MulM4FOp);
    return AM_MulM4F(Left, Right);
}

COVERAGE(AM_MulQFOp, 1)
static inline AmQuat operator*(AmQuat Left, float Right)
{
    ASSERT_COVERED(AM_MulQFOp);
    return AM_MulQF(Left, Right);
}

COVERAGE(AM_MulV2FOpLeft, 1)
static inline AmVec2 operator*(float Left, AmVec2 Right)
{
    ASSERT_COVERED(AM_MulV2FOpLeft);
    return AM_MulV2F(Right, Left);
}

COVERAGE(AM_MulV3FOpLeft, 1)
static inline AmVec3 operator*(float Left, AmVec3 Right)
{
    ASSERT_COVERED(AM_MulV3FOpLeft);
    return AM_MulV3F(Right, Left);
}

COVERAGE(AM_MulV4FOpLeft, 1)
static inline AmVec4 operator*(float Left, AmVec4 Right)
{
    ASSERT_COVERED(AM_MulV4FOpLeft);
    return AM_MulV4F(Right, Left);
}

COVERAGE(AM_MulM2FOpLeft, 1)
static inline AmMat2 operator*(float Left, AmMat2 Right)
{
    ASSERT_COVERED(AM_MulM2FOpLeft);
    return AM_MulM2F(Right, Left);
}

COVERAGE(AM_MulM3FOpLeft, 1)
static inline AmMat3 operator*(float Left, AmMat3 Right)
{
    ASSERT_COVERED(AM_MulM3FOpLeft);
    return AM_MulM3F(Right, Left);
}

COVERAGE(AM_MulM4FOpLeft, 1)
static inline AmMat4 operator*(float Left, AmMat4 Right)
{
    ASSERT_COVERED(AM_MulM4FOpLeft);
    return AM_MulM4F(Right, Left);
}

COVERAGE(AM_MulQFOpLeft, 1)
static inline AmQuat operator*(float Left, AmQuat Right)
{
    ASSERT_COVERED(AM_MulQFOpLeft);
    return AM_MulQF(Right, Left);
}

COVERAGE(AM_MulM2V2Op, 1)
static inline AmVec2 operator*(AmMat2 Matrix, AmVec2 Vector)
{
    ASSERT_COVERED(AM_MulM2V2Op);
    return AM_MulM2V2(Matrix, Vector);
}

COVERAGE(AM_MulM3V3Op, 1)
static inline AmVec3 operator*(AmMat3 Matrix, AmVec3 Vector)
{
    ASSERT_COVERED(AM_MulM3V3Op);
    return AM_MulM3V3(Matrix, Vector);
}

COVERAGE(AM_MulM4V4Op, 1)
static inline AmVec4 operator*(AmMat4 Matrix, AmVec4 Vector)
{
    ASSERT_COVERED(AM_MulM4V4Op);
    return AM_MulM4V4(Matrix, Vector);
}

COVERAGE(AM_DivV2Op, 1)
static inline AmVec2 operator/(AmVec2 Left, AmVec2 Right)
{
    ASSERT_COVERED(AM_DivV2Op);
    return AM_DivV2(Left, Right);
}

COVERAGE(AM_DivV3Op, 1)
static inline AmVec3 operator/(AmVec3 Left, AmVec3 Right)
{
    ASSERT_COVERED(AM_DivV3Op);
    return AM_DivV3(Left, Right);
}

COVERAGE(AM_DivV4Op, 1)
static inline AmVec4 operator/(AmVec4 Left, AmVec4 Right)
{
    ASSERT_COVERED(AM_DivV4Op);
    return AM_DivV4(Left, Right);
}

COVERAGE(AM_DivV2FOp, 1)
static inline AmVec2 operator/(AmVec2 Left, float Right)
{
    ASSERT_COVERED(AM_DivV2FOp);
    return AM_DivV2F(Left, Right);
}

COVERAGE(AM_DivV3FOp, 1)
static inline AmVec3 operator/(AmVec3 Left, float Right)
{
    ASSERT_COVERED(AM_DivV3FOp);
    return AM_DivV3F(Left, Right);
}

COVERAGE(AM_DivV4FOp, 1)
static inline AmVec4 operator/(AmVec4 Left, float Right)
{
    ASSERT_COVERED(AM_DivV4FOp);
    return AM_DivV4F(Left, Right);
}

COVERAGE(AM_DivM4FOp, 1)
static inline AmMat4 operator/(AmMat4 Left, float Right)
{
    ASSERT_COVERED(AM_DivM4FOp);
    return AM_DivM4F(Left, Right);
}

COVERAGE(AM_DivM3FOp, 1)
static inline AmMat3 operator/(AmMat3 Left, float Right)
{
    ASSERT_COVERED(AM_DivM3FOp);
    return AM_DivM3F(Left, Right);
}

COVERAGE(AM_DivM2FOp, 1)
static inline AmMat2 operator/(AmMat2 Left, float Right)
{
    ASSERT_COVERED(AM_DivM2FOp);
    return AM_DivM2F(Left, Right);
}

COVERAGE(AM_DivQFOp, 1)
static inline AmQuat operator/(AmQuat Left, float Right)
{
    ASSERT_COVERED(AM_DivQFOp);
    return AM_DivQF(Left, Right);
}

COVERAGE(AM_AddV2Assign, 1)
static inline AmVec2& operator+=(AmVec2& Left, AmVec2 Right)
{
    ASSERT_COVERED(AM_AddV2Assign);
    return Left = Left + Right;
}

COVERAGE(AM_AddV3Assign, 1)
static inline AmVec3& operator+=(AmVec3& Left, AmVec3 Right)
{
    ASSERT_COVERED(AM_AddV3Assign);
    return Left = Left + Right;
}

COVERAGE(AM_AddV4Assign, 1)
static inline AmVec4& operator+=(AmVec4& Left, AmVec4 Right)
{
    ASSERT_COVERED(AM_AddV4Assign);
    return Left = Left + Right;
}

COVERAGE(AM_AddM2Assign, 1)
static inline AmMat2& operator+=(AmMat2& Left, AmMat2 Right)
{
    ASSERT_COVERED(AM_AddM2Assign);
    return Left = Left + Right;
}

COVERAGE(AM_AddM3Assign, 1)
static inline AmMat3& operator+=(AmMat3& Left, AmMat3 Right)
{
    ASSERT_COVERED(AM_AddM3Assign);
    return Left = Left + Right;
}

COVERAGE(AM_AddM4Assign, 1)
static inline AmMat4& operator+=(AmMat4& Left, AmMat4 Right)
{
    ASSERT_COVERED(AM_AddM4Assign);
    return Left = Left + Right;
}

COVERAGE(AM_AddQAssign, 1)
static inline AmQuat& operator+=(AmQuat& Left, AmQuat Right)
{
    ASSERT_COVERED(AM_AddQAssign);
    return Left = Left + Right;
}

COVERAGE(AM_SubV2Assign, 1)
static inline AmVec2& operator-=(AmVec2& Left, AmVec2 Right)
{
    ASSERT_COVERED(AM_SubV2Assign);
    return Left = Left - Right;
}

COVERAGE(AM_SubV3Assign, 1)
static inline AmVec3& operator-=(AmVec3& Left, AmVec3 Right)
{
    ASSERT_COVERED(AM_SubV3Assign);
    return Left = Left - Right;
}

COVERAGE(AM_SubV4Assign, 1)
static inline AmVec4& operator-=(AmVec4& Left, AmVec4 Right)
{
    ASSERT_COVERED(AM_SubV4Assign);
    return Left = Left - Right;
}

COVERAGE(AM_SubM2Assign, 1)
static inline AmMat2& operator-=(AmMat2& Left, AmMat2 Right)
{
    ASSERT_COVERED(AM_SubM2Assign);
    return Left = Left - Right;
}

COVERAGE(AM_SubM3Assign, 1)
static inline AmMat3& operator-=(AmMat3& Left, AmMat3 Right)
{
    ASSERT_COVERED(AM_SubM3Assign);
    return Left = Left - Right;
}

COVERAGE(AM_SubM4Assign, 1)
static inline AmMat4& operator-=(AmMat4& Left, AmMat4 Right)
{
    ASSERT_COVERED(AM_SubM4Assign);
    return Left = Left - Right;
}

COVERAGE(AM_SubQAssign, 1)
static inline AmQuat& operator-=(AmQuat& Left, AmQuat Right)
{
    ASSERT_COVERED(AM_SubQAssign);
    return Left = Left - Right;
}

COVERAGE(AM_MulV2Assign, 1)
static inline AmVec2& operator*=(AmVec2& Left, AmVec2 Right)
{
    ASSERT_COVERED(AM_MulV2Assign);
    return Left = Left * Right;
}

COVERAGE(AM_MulV3Assign, 1)
static inline AmVec3& operator*=(AmVec3& Left, AmVec3 Right)
{
    ASSERT_COVERED(AM_MulV3Assign);
    return Left = Left * Right;
}

COVERAGE(AM_MulV4Assign, 1)
static inline AmVec4& operator*=(AmVec4& Left, AmVec4 Right)
{
    ASSERT_COVERED(AM_MulV4Assign);
    return Left = Left * Right;
}

COVERAGE(AM_MulV2FAssign, 1)
static inline AmVec2& operator*=(AmVec2& Left, float Right)
{
    ASSERT_COVERED(AM_MulV2FAssign);
    return Left = Left * Right;
}

COVERAGE(AM_MulV3FAssign, 1)
static inline AmVec3& operator*=(AmVec3& Left, float Right)
{
    ASSERT_COVERED(AM_MulV3FAssign);
    return Left = Left * Right;
}

COVERAGE(AM_MulV4FAssign, 1)
static inline AmVec4& operator*=(AmVec4& Left, float Right)
{
    ASSERT_COVERED(AM_MulV4FAssign);
    return Left = Left * Right;
}

COVERAGE(AM_MulM2FAssign, 1)
static inline AmMat2& operator*=(AmMat2& Left, float Right)
{
    ASSERT_COVERED(AM_MulM2FAssign);
    return Left = Left * Right;
}

COVERAGE(AM_MulM3FAssign, 1)
static inline AmMat3& operator*=(AmMat3& Left, float Right)
{
    ASSERT_COVERED(AM_MulM3FAssign);
    return Left = Left * Right;
}

COVERAGE(AM_MulM4FAssign, 1)
static inline AmMat4& operator*=(AmMat4& Left, float Right)
{
    ASSERT_COVERED(AM_MulM4FAssign);
    return Left = Left * Right;
}

COVERAGE(AM_MulQFAssign, 1)
static inline AmQuat& operator*=(AmQuat& Left, float Right)
{
    ASSERT_COVERED(AM_MulQFAssign);
    return Left = Left * Right;
}

COVERAGE(AM_DivV2Assign, 1)
static inline AmVec2& operator/=(AmVec2& Left, AmVec2 Right)
{
    ASSERT_COVERED(AM_DivV2Assign);
    return Left = Left / Right;
}

COVERAGE(AM_DivV3Assign, 1)
static inline AmVec3& operator/=(AmVec3& Left, AmVec3 Right)
{
    ASSERT_COVERED(AM_DivV3Assign);
    return Left = Left / Right;
}

COVERAGE(AM_DivV4Assign, 1)
static inline AmVec4& operator/=(AmVec4& Left, AmVec4 Right)
{
    ASSERT_COVERED(AM_DivV4Assign);
    return Left = Left / Right;
}

COVERAGE(AM_DivV2FAssign, 1)
static inline AmVec2& operator/=(AmVec2& Left, float Right)
{
    ASSERT_COVERED(AM_DivV2FAssign);
    return Left = Left / Right;
}

COVERAGE(AM_DivV3FAssign, 1)
static inline AmVec3& operator/=(AmVec3& Left, float Right)
{
    ASSERT_COVERED(AM_DivV3FAssign);
    return Left = Left / Right;
}

COVERAGE(AM_DivV4FAssign, 1)
static inline AmVec4& operator/=(AmVec4& Left, float Right)
{
    ASSERT_COVERED(AM_DivV4FAssign);
    return Left = Left / Right;
}

COVERAGE(AM_DivM4FAssign, 1)
static inline AmMat4& operator/=(AmMat4& Left, float Right)
{
    ASSERT_COVERED(AM_DivM4FAssign);
    return Left = Left / Right;
}

COVERAGE(AM_DivQFAssign, 1)
static inline AmQuat& operator/=(AmQuat& Left, float Right)
{
    ASSERT_COVERED(AM_DivQFAssign);
    return Left = Left / Right;
}

COVERAGE(AM_EqV2Op, 1)
static inline AmBool operator==(AmVec2 Left, AmVec2 Right)
{
    ASSERT_COVERED(AM_EqV2Op);
    return AM_EqV2(Left, Right);
}

COVERAGE(AM_EqV3Op, 1)
static inline AmBool operator==(AmVec3 Left, AmVec3 Right)
{
    ASSERT_COVERED(AM_EqV3Op);
    return AM_EqV3(Left, Right);
}

COVERAGE(AM_EqV4Op, 1)
static inline AmBool operator==(AmVec4 Left, AmVec4 Right)
{
    ASSERT_COVERED(AM_EqV4Op);
    return AM_EqV4(Left, Right);
}

COVERAGE(AM_EqV2OpNot, 1)
static inline AmBool operator!=(AmVec2 Left, AmVec2 Right)
{
    ASSERT_COVERED(AM_EqV2OpNot);
    return !AM_EqV2(Left, Right);
}

COVERAGE(AM_EqV3OpNot, 1)
static inline AmBool operator!=(AmVec3 Left, AmVec3 Right)
{
    ASSERT_COVERED(AM_EqV3OpNot);
    return !AM_EqV3(Left, Right);
}

COVERAGE(AM_EqV4OpNot, 1)
static inline AmBool operator!=(AmVec4 Left, AmVec4 Right)
{
    ASSERT_COVERED(AM_EqV4OpNot);
    return !AM_EqV4(Left, Right);
}

COVERAGE(AM_UnaryMinusV2, 1)
static inline AmVec2 operator-(AmVec2 In)
{
    ASSERT_COVERED(AM_UnaryMinusV2);

    AmVec2 Result;
    Result.X = -In.X;
    Result.Y = -In.Y;

    return Result;
}

COVERAGE(AM_UnaryMinusV3, 1)
static inline AmVec3 operator-(AmVec3 In)
{
    ASSERT_COVERED(AM_UnaryMinusV3);

    AmVec3 Result;
    Result.X = -In.X;
    Result.Y = -In.Y;
    Result.Z = -In.Z;

    return Result;
}

COVERAGE(AM_UnaryMinusV4, 1)
static inline AmVec4 operator-(AmVec4 In)
{
    ASSERT_COVERED(AM_UnaryMinusV4);

    AmVec4 Result;
#if HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_xor_ps(In.SSE, _mm_set1_ps(-0.0f));
#else
    Result.X = -In.X;
    Result.Y = -In.Y;
    Result.Z = -In.Z;
    Result.W = -In.W;
#endif

    return Result;
}

#endif /* __cplusplus*/

#ifdef HANDMADE_MATH__USE_C11_GENERICS
#define AM_Add(A, B)                                                                                                                       \
    _Generic((A), AmVec2                                                                                                                   \
             : AM_AddV2, AmVec3                                                                                                            \
             : AM_AddV3, AmVec4                                                                                                            \
             : AM_AddV4, AmMat2                                                                                                            \
             : AM_AddM2, AmMat3                                                                                                            \
             : AM_AddM3, AmMat4                                                                                                            \
             : AM_AddM4, AmQuat                                                                                                            \
             : AM_AddQ)(A, B)

#define AM_Sub(A, B)                                                                                                                       \
    _Generic((A), AmVec2                                                                                                                   \
             : AM_SubV2, AmVec3                                                                                                            \
             : AM_SubV3, AmVec4                                                                                                            \
             : AM_SubV4, AmMat2                                                                                                            \
             : AM_SubM2, AmMat3                                                                                                            \
             : AM_SubM3, AmMat4                                                                                                            \
             : AM_SubM4, AmQuat                                                                                                            \
             : AM_SubQ)(A, B)

#define AM_Mul(A, B)                                                                                                                       \
    _Generic((B), float                                                                                                                    \
             : _Generic((A), AmVec2                                                                                                        \
                        : AM_MulV2F, AmVec3                                                                                                \
                        : AM_MulV3F, AmVec4                                                                                                \
                        : AM_MulV4F, AmMat2                                                                                                \
                        : AM_MulM2F, AmMat3                                                                                                \
                        : AM_MulM3F, AmMat4                                                                                                \
                        : AM_MulM4F, AmQuat                                                                                                \
                        : AM_MulQF),                                                                                                       \
               AmMat2                                                                                                                      \
             : AM_MulM2, AmMat3                                                                                                            \
             : AM_MulM3, AmMat4                                                                                                            \
             : AM_MulM4, AmQuat                                                                                                            \
             : AM_MulQ, default                                                                                                            \
             : _Generic((A), AmVec2                                                                                                        \
                        : AM_MulV2, AmVec3                                                                                                 \
                        : AM_MulV3, AmVec4                                                                                                 \
                        : AM_MulV4, AmMat2                                                                                                 \
                        : AM_MulM2V2, AmMat3                                                                                               \
                        : AM_MulM3V3, AmMat4                                                                                               \
                        : AM_MulM4V4))(A, B)

#define AM_Div(A, B)                                                                                                                       \
    _Generic((B), float                                                                                                                    \
             : _Generic((A), AmMat2                                                                                                        \
                        : AM_DivM2F, AmMat3                                                                                                \
                        : AM_DivM3F, AmMat4                                                                                                \
                        : AM_DivM4F, AmVec2                                                                                                \
                        : AM_DivV2F, AmVec3                                                                                                \
                        : AM_DivV3F, AmVec4                                                                                                \
                        : AM_DivV4F, AmQuat                                                                                                \
                        : AM_DivQF),                                                                                                       \
               AmMat2                                                                                                                      \
             : AM_DivM2, AmMat3                                                                                                            \
             : AM_DivM3, AmMat4                                                                                                            \
             : AM_DivM4, AmQuat                                                                                                            \
             : AM_DivQ, default                                                                                                            \
             : _Generic((A), AmVec2                                                                                                        \
                        : AM_DivV2, AmVec3                                                                                                 \
                        : AM_DivV3, AmVec4                                                                                                 \
                        : AM_DivV4))(A, B)

#define AM_Len(A) _Generic((A), AmVec2 : AM_LenV2, AmVec3 : AM_LenV3, AmVec4 : AM_LenV4)(A)

#define AM_LenSqr(A) _Generic((A), AmVec2 : AM_LenSqrV2, AmVec3 : AM_LenSqrV3, AmVec4 : AM_LenSqrV4)(A)

#define AM_Norm(A) _Generic((A), AmVec2 : AM_NormV2, AmVec3 : AM_NormV3, AmVec4 : AM_NormV4)(A)

#define AM_Dot(A, B) _Generic((A), AmVec2 : AM_DotV2, AmVec3 : AM_DotV3, AmVec4 : AM_DotV4)(A, B)

#define AM_Lerp(A, T, B) _Generic((A), float : AM_Lerp, AmVec2 : AM_LerpV2, AmVec3 : AM_LerpV3, AmVec4 : AM_LerpV4)(A, T, B)

#define AM_Eq(A, B) _Generic((A), AmVec2 : AM_EqV2, AmVec3 : AM_EqV3, AmVec4 : AM_EqV4)(A, B)

#define AM_Transpose(M) _Generic((M), AmMat2 : AM_TransposeM2, AmMat3 : AM_TransposeM3, AmMat4 : AM_TransposeM4)(M)

#define AM_Determinant(M) _Generic((M), AmMat2 : AM_DeterminantM2, AmMat3 : AM_DeterminantM3, AmMat4 : AM_DeterminantM4)(M)

#define AM_InvGeneral(M) _Generic((M), AmMat2 : AM_InvGeneralM2, AmMat3 : AM_InvGeneralM3, AmMat4 : AM_InvGeneralM4)(M)

#endif

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif /* HANDMADE_MATH_H */
