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

#ifndef _AM_IMPLEMENTATION_MATH_LINEAR_ALGEBRA_H
#define _AM_IMPLEMENTATION_MATH_LINEAR_ALGEBRA_H

#include <SparkyStudios/Audio/Amplitude/Math/LinearAlgebra.h>

#include <Eigen/Dense>

namespace SparkyStudios::Audio::Amplitude
{
    AM_INLINE Eigen::Vector2f Vec2ToEigen(const AmVector2& v)
    {
        return Eigen::Vector2f(v.x, v.y);
    }

    AM_INLINE AmVector2 EigenToVec2(const Eigen::Vector2f& v)
    {
        return { v.x(), v.y() };
    }

    AM_INLINE Eigen::Vector3f Vec3ToEigen(const AmVector3& v)
    {
        return Eigen::Vector3f(v.x, v.y, v.z);
    }

    AM_INLINE AmVector3 EigenToVec3(const Eigen::Vector3f& v)
    {
        return { v.x(), v.y(), v.z() };
    }

    AM_INLINE Eigen::Vector4f Vec4ToEigen(const AmVector4& v)
    {
        return Eigen::Vector4f(v.x, v.y, v.z, v.w);
    }

    AM_INLINE AmVector4 EigenToVec4(const Eigen::Vector4f& v)
    {
        return { v.x(), v.y(), v.z(), v.w() };
    }

    AM_INLINE Eigen::Vector2f VecToEigen(const AmVector2& v)
    {
        return Vec2ToEigen(v);
    }

    AM_INLINE AmVector2 EigenToVec(const Eigen::Vector2f& v)
    {
        return EigenToVec2(v);
    }

    AM_INLINE Eigen::Vector3f VecToEigen(const AmVector3& v)
    {
        return Vec3ToEigen(v);
    }

    AM_INLINE AmVector3 EigenToVec(const Eigen::Vector3f& v)
    {
        return EigenToVec3(v);
    }

    AM_INLINE Eigen::Vector4f VecToEigen(const AmVector4& v)
    {
        return Vec4ToEigen(v);
    }

    AM_INLINE AmVector4 EigenToVec(const Eigen::Vector4f& v)
    {
        return EigenToVec4(v);
    }

    AM_INLINE Eigen::Quaternionf QuatToEigen(const AmQuaternion& q)
    {
        return Eigen::Quaternionf(q.w, q.x, q.y, q.z);
    }

    AM_INLINE AmQuaternion EigenToQuat(const Eigen::Quaternionf& q)
    {
        return { q.w(), q.x(), q.y(), q.z() };
    }

    AM_INLINE Eigen::Matrix3f Mat3ToEigen(const AmMatrix3& m)
    {
        return Eigen::Map<const Eigen::Matrix3f>(m.ptr());
    }

    AM_INLINE AmMatrix3 EigenToMat3(const Eigen::Matrix3f& m)
    {
        AmMatrix3 result;
        std::memcpy(result.ptr(), m.data(), sizeof(AmMatrix3));
        return result;
    }

    AM_INLINE Eigen::Matrix4f Mat4ToEigen(const AmMatrix4& m)
    {
        return Eigen::Map<const Eigen::Matrix4f>(m.ptr());
    }

    AM_INLINE AmMatrix4 EigenToMat4(const Eigen::Matrix4f& m)
    {
        AmMatrix4 result;
        std::memcpy(result.ptr(), m.data(), sizeof(AmMatrix4));
        return result;
    }

    AM_INLINE Eigen::Matrix3f MatToEigen(const AmMatrix3& m)
    {
        return Mat3ToEigen(m);
    }

    AM_INLINE AmMatrix3 EigenToMat(const Eigen::Matrix3f& m)
    {
        return EigenToMat3(m);
    }

    AM_INLINE Eigen::Matrix4f MatToEigen(const AmMatrix4& m)
    {
        return Mat4ToEigen(m);
    }

    AM_INLINE AmMatrix4 EigenToMat(const Eigen::Matrix4f& m)
    {
        return EigenToMat4(m);
    }

    AM_INLINE AmVector2 Sub(const AmVector2& a, const AmVector2& b)
    {
        return EigenToVec2(Vec2ToEigen(a) - Vec2ToEigen(b));
    }

    AM_INLINE AmVector2 Add(const AmVector2& a, const AmVector2& b)
    {
        return EigenToVec2(Vec2ToEigen(a) + Vec2ToEigen(b));
    }

    AM_INLINE AmVector2 Mul(const AmVector2& a, const AmReal32 b)
    {
        return EigenToVec2(Vec2ToEigen(a) * b);
    }

    AM_INLINE AmVector2 Cross(const AmVector2& a, const AmVector2& b)
    {
        return EigenToVec2(Vec2ToEigen(a).cross(Vec2ToEigen(b)));
    }

    AM_INLINE AmReal32 Dot(const AmVector2& a, const AmVector2& b)
    {
        return Vec2ToEigen(a).dot(Vec2ToEigen(b));
    }

    AM_INLINE AmReal32 Length(const AmVector2& v)
    {
        return Vec2ToEigen(v).norm();
    }

    AM_INLINE AmReal32 SquaredLength(const AmVector2& v)
    {
        return Vec2ToEigen(v).squaredNorm();
    }

    AM_INLINE AmVector2 Normalize(const AmVector2& v)
    {
        return EigenToVec2(Vec2ToEigen(v).normalized());
    }

    AM_INLINE AmVector2 Negate(const AmVector2& v)
    {
        return EigenToVec2(-Vec2ToEigen(v));
    }

    AM_INLINE AmVector3 Sub(const AmVector3& a, const AmVector3& b)
    {
        return EigenToVec3(Vec3ToEigen(a) - Vec3ToEigen(b));
    }

    AM_INLINE AmVector3 Add(const AmVector3& a, const AmVector3& b)
    {
        return EigenToVec3(Vec3ToEigen(a) + Vec3ToEigen(b));
    }

    AM_INLINE AmVector3 Mul(const AmVector3& a, const AmReal32 b)
    {
        return EigenToVec3(Vec3ToEigen(a) * b);
    }

    AM_INLINE AmVector3 Cross(const AmVector3& a, const AmVector3& b)
    {
        return EigenToVec3(Vec3ToEigen(a).cross(Vec3ToEigen(b)));
    }

    AM_INLINE AmReal32 Dot(const AmVector3& a, const AmVector3& b)
    {
        return Vec3ToEigen(a).dot(Vec3ToEigen(b));
    }

    AM_INLINE AmReal32 Length(const AmVector3& v)
    {
        return Vec3ToEigen(v).norm();
    }

    AM_INLINE AmReal32 SquaredLength(const AmVector3& v)
    {
        return Vec3ToEigen(v).squaredNorm();
    }

    AM_INLINE AmVector3 Normalize(const AmVector3& v)
    {
        return EigenToVec3(Vec3ToEigen(v).normalized());
    }

    AM_INLINE AmVector3 Negate(const AmVector3& v)
    {
        return EigenToVec3(-Vec3ToEigen(v));
    }

    AM_INLINE AmVector3 RotateVector(const AmVector3& v, const AmQuaternion& q)
    {
        return EigenToVec3(QuatToEigen(q) * Vec3ToEigen(v));
    }

    AM_INLINE AmMatrix3 Mul(const AmMatrix3& a, const AmMatrix3& b)
    {
        return EigenToMat3(Mat3ToEigen(a) * Mat3ToEigen(b));
    }

    AM_INLINE AmMatrix3 Inverse(const AmMatrix3& m)
    {
        return EigenToMat3(Mat3ToEigen(m).inverse());
    }

    AM_INLINE AmMatrix3 Normalize(const AmMatrix3& m)
    {
        return EigenToMat3(Mat3ToEigen(m).normalized());
    }

    AM_INLINE AmMatrix3 Transpose(const AmMatrix3& m)
    {
        return EigenToMat3(Mat3ToEigen(m).transpose());
    }

    AM_INLINE AmReal32 Determinant(const AmMatrix3& m)
    {
        return Mat3ToEigen(m).determinant();
    }

    AM_INLINE AmReal32 Determinant(const AmMatrix4& m)
    {
        return Mat4ToEigen(m).determinant();
    }

    AM_INLINE AmVector3 Transform(const AmMatrix3& m, const AmVector3& v)
    {
        return EigenToVec3(Mat3ToEigen(m) * Vec3ToEigen(v));
    }

    AM_INLINE AmVector4 Transform(const AmMatrix4& m, const AmVector4& v)
    {
        return EigenToVec4(Mat4ToEigen(m) * Vec4ToEigen(v));
    }

    AM_INLINE AmQuaternion FromAxisAngle(const AmVector3& axis, AmReal32 angle)
    {
        return EigenToQuat(Eigen::Quaternionf(Eigen::AngleAxisf(angle, VecToEigen(axis))));
    }

    AM_INLINE AmQuaternion FromRotationMatrix(const AmMatrix3& m)
    {
        return EigenToQuat(Eigen::Quaternionf(Mat3ToEigen(m)));
    }

    AM_INLINE AmQuaternion Slerp(const AmQuaternion& a, const AmQuaternion& b, AmReal32 t)
    {
        return EigenToQuat(QuatToEigen(a).slerp(t, QuatToEigen(b)));
    }

    AM_INLINE AmQuaternion Conjugate(const AmQuaternion& q)
    {
        return EigenToQuat(QuatToEigen(q).conjugate());
    }

    AM_INLINE AmQuaternion Inverse(const AmQuaternion& q)
    {
        return EigenToQuat(QuatToEigen(q).inverse());
    }

    AM_INLINE AmMatrix3 Identity3()
    {
        return EigenToMat3(Eigen::Matrix3f::Identity());
    }

    AM_INLINE AmMatrix4 Identity4()
    {
        return EigenToMat4(Eigen::Matrix4f::Identity());
    }
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MATH_LINEAR_ALGEBRA_H