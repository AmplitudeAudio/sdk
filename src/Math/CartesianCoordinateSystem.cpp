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

#include <SparkyStudios/Audio/Amplitude/Math/CartesianCoordinateSystem.h>

#include <Math/LinearAlgebra.h>

namespace SparkyStudios::Audio::Amplitude
{
    CartesianCoordinateSystem::Converter::Converter(const CartesianCoordinateSystem& from, const CartesianCoordinateSystem& to)
        : _fromToMatrix()
        , _toFromMatrix()
        , _fromToScalar(0.0f)
        , _toFromScalar(0.0f)
        , _windingSwap(0.0f)
    {
        const auto sourceForwardVector = GetVector(from._forwardAxis);
        const auto sourceRightVector = GetVector(from._rightAxis);
        const auto sourceUpVector = GetVector(from._upAxis);

        const auto targetForwardVector = GetVector(to._forwardAxis);
        const auto targetRightVector = GetVector(to._rightAxis);
        const auto targetUpVector = GetVector(to._upAxis);

        const AmReal32 sourceScale = SquaredLength(sourceForwardVector);

        Eigen::Matrix3f sourceFromId = Eigen::Matrix3f::Zero();
        sourceFromId.block<3, 1>(0, 0) = Vec3ToEigen(sourceRightVector);
        sourceFromId.block<3, 1>(0, 1) = Vec3ToEigen(sourceUpVector);
        sourceFromId.block<3, 1>(0, 2) = Vec3ToEigen(sourceForwardVector);

        const AmReal32 targetScale = SquaredLength(targetForwardVector);
        AMPLITUDE_ASSERT(targetScale == SquaredLength(targetRightVector));
        AMPLITUDE_ASSERT(targetScale == SquaredLength(targetUpVector));

        Eigen::Matrix3f targetFromId = Eigen::Matrix3f::Zero();
        targetFromId.block<3, 1>(0, 0) = Vec3ToEigen(targetRightVector);
        targetFromId.block<3, 1>(0, 1) = Vec3ToEigen(targetUpVector);
        targetFromId.block<3, 1>(0, 2) = Vec3ToEigen(targetForwardVector);

        _fromToMatrix = EigenToMat3((targetFromId * sourceFromId.inverse()).matrix());
        _fromToScalar = (1.0f / std::sqrt(sourceScale)) * std::sqrt(targetScale);

        _toFromMatrix = Inverse(_fromToMatrix);
        _toFromScalar = 1.0f / _fromToScalar;

        _windingSwap = Determinant(_fromToMatrix) < 0 ? -1.0f : 1.0f;
    }

    AmVector3 CartesianCoordinateSystem::Converter::Forward(const AmVector3& vector) const
    {
        return Scale(Transform(_fromToMatrix, vector), _fromToScalar);
    }

    AmQuaternion CartesianCoordinateSystem::Converter::Forward(const AmQuaternion& quaternion) const
    {
        const auto axis = Transform(_fromToMatrix, quaternion.xyz);
        return { quaternion.w * _windingSwap, axis.x, axis.y, axis.z };
    }

    AmReal32 CartesianCoordinateSystem::Converter::Forward(const AmReal32& scalar) const
    {
        return scalar * _fromToScalar;
    }

    AmVector3 CartesianCoordinateSystem::Converter::Backward(const AmVector3& vector) const
    {
        return Scale(Transform(_toFromMatrix, vector), _toFromScalar);
    }

    AmQuaternion CartesianCoordinateSystem::Converter::Backward(const AmQuaternion& quaternion) const
    {
        const auto axis = Transform(_toFromMatrix, quaternion.xyz);
        return { quaternion.w * _windingSwap, axis.x, axis.y, axis.z };
    }

    AmReal32 CartesianCoordinateSystem::Converter::Backward(const AmReal32& scalar) const
    {
        return scalar * _toFromScalar;
    }

    CartesianCoordinateSystem CartesianCoordinateSystem::Default()
    {
        return RightHandedZUp();
    }

    CartesianCoordinateSystem CartesianCoordinateSystem::AmbiX()
    {
        return { Axis::NegativeY, Axis::PositiveX, Axis::PositiveZ };
    }

    CartesianCoordinateSystem CartesianCoordinateSystem::RightHandedYUp()
    {
        return { Axis::PositiveX, Axis::NegativeZ, Axis::PositiveY };
    }

    CartesianCoordinateSystem CartesianCoordinateSystem::LeftHandedYUp()
    {
        return { Axis::PositiveX, Axis::PositiveZ, Axis::PositiveY };
    }

    CartesianCoordinateSystem CartesianCoordinateSystem::RightHandedZUp()
    {
        return { Axis::PositiveX, Axis::PositiveY, Axis::PositiveZ };
    }

    CartesianCoordinateSystem CartesianCoordinateSystem::LeftHandedZUp()
    {
        return { Axis::PositiveX, Axis::NegativeY, Axis::PositiveZ };
    }

    AmVector3 CartesianCoordinateSystem::Convert(
        const AmVector3& vector, const CartesianCoordinateSystem& from, const CartesianCoordinateSystem& to)
    {
        return to.Convert(vector, from);
    }

    AmQuaternion CartesianCoordinateSystem::Convert(
        const AmQuaternion& rotation, const CartesianCoordinateSystem& from, const CartesianCoordinateSystem& to)
    {
        return to.Convert(rotation, from);
    }

    AmReal32 CartesianCoordinateSystem::Convert(AmReal32 scalar, const CartesianCoordinateSystem& from, const CartesianCoordinateSystem& to)
    {
        return to.Convert(scalar, from);
    }

    AmVector3 CartesianCoordinateSystem::ConvertToDefault(const AmVector3& vector, const CartesianCoordinateSystem& from)
    {
        return Convert(vector, from, Default());
    }

    AmQuaternion CartesianCoordinateSystem::ConvertToDefault(const AmQuaternion& rotation, const CartesianCoordinateSystem& from)
    {
        return Convert(rotation, from, Default());
    }

    AmReal32 CartesianCoordinateSystem::ConvertToDefault(AmReal32 scalar, const CartesianCoordinateSystem& from)
    {
        return Convert(scalar, from, Default());
    }

    AmVector3 CartesianCoordinateSystem::GetVector(Axis axis)
    {
        switch (axis)
        {
        case Axis::PositiveX:
            return kVector3UnitX;
        case Axis::NegativeX:
            return { -1.0f, 0.0f, 0.0f };
        case Axis::PositiveY:
            return kVector3UnitY;
        case Axis::NegativeY:
            return { 0.0f, -1.0f, 0.0f };
        case Axis::PositiveZ:
            return kVector3UnitZ;
        case Axis::NegativeZ:
            return { 0.0f, 0.0f, -1.0f };
        default:
            AMPLITUDE_ASSERT(false);
            return kVector3Zero;
        }
    }

    CartesianCoordinateSystem::CartesianCoordinateSystem(Axis right, Axis forward, Axis up)
        : _rightAxis(right)
        , _forwardAxis(forward)
        , _upAxis(up)
    {}

    AmVector3 CartesianCoordinateSystem::Convert(const AmVector3& vector, const CartesianCoordinateSystem& from) const
    {
        const Converter converter(from, *this);
        return converter.Forward(vector);
    }

    AmQuaternion CartesianCoordinateSystem::Convert(const AmQuaternion& quaternion, const CartesianCoordinateSystem& from) const
    {
        const Converter converter(from, *this);
        return converter.Forward(quaternion);
    }

    AmReal32 CartesianCoordinateSystem::Convert(const AmReal32& scalar, const CartesianCoordinateSystem& from) const
    {
        const Converter converter(from, *this);
        return converter.Forward(scalar);
    }
} // namespace SparkyStudios::Audio::Amplitude
