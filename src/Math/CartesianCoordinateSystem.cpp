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

namespace SparkyStudios::Audio::Amplitude
{
    CartesianCoordinateSystem::Converter::Converter(const CartesianCoordinateSystem& from, const CartesianCoordinateSystem& to)
        : _fromToMatrix()
        , _toFromMatrix()
        , _fromToScalar(0.0f)
        , _toFromScalar(0.0f)
        , _windingSwap(0.0f)
    {
        const AmVec3 sourceForwadVector = GetVector(from._forwardAxis);
        const AmVec3 sourceRightVector = GetVector(from._rightAxis);
        const AmVec3 sourceUpVector = GetVector(from._upAxis);

        const AmVec3 targetForwadVector = GetVector(to._forwardAxis);
        const AmVec3 targetRightVector = GetVector(to._rightAxis);
        const AmVec3 targetUpVector = GetVector(to._upAxis);

        const AmReal32 sourceScale = AM_LenSqr(sourceForwadVector);

        AmMat3 sourceFromId;
        sourceFromId[0] = sourceRightVector;
        sourceFromId[1] = sourceUpVector;
        sourceFromId[2] = sourceForwadVector;

        const AmReal32 targetScale = AM_LenSqr(targetForwadVector);
        AMPLITUDE_ASSERT(targetScale == AM_LenSqr(targetRightVector));
        AMPLITUDE_ASSERT(targetScale == AM_LenSqr(targetUpVector));

        AmMat3 targetFromId;
        targetFromId[0] = targetRightVector;
        targetFromId[1] = targetUpVector;
        targetFromId[2] = targetForwadVector;

        _fromToMatrix = targetFromId * AM_InvGeneral(sourceFromId);
        _fromToMatrix[0] = AM_Norm(_fromToMatrix[0]);
        _fromToMatrix[1] = AM_Norm(_fromToMatrix[1]);
        _fromToMatrix[2] = AM_Norm(_fromToMatrix[2]);
        _fromToScalar = 1.0f / std::sqrt(sourceScale) * std::sqrt(targetScale);

        _toFromMatrix = AM_InvGeneral(_fromToMatrix);
        _toFromScalar = 1.0f / _fromToScalar;

        _windingSwap = AM_Determinant(_fromToMatrix) < 0 ? -1.0f : 1.0f;
    }

    AmVec3 CartesianCoordinateSystem::Converter::Forward(const AmVec3& vector) const
    {
        return _fromToMatrix * vector * _fromToScalar;
    }

    AmQuat CartesianCoordinateSystem::Converter::Forward(const AmQuat& quaternion) const
    {
        const AmVec3 axis = _fromToMatrix * quaternion.XYZ;
        return AM_Q(axis.X, axis.Y, axis.Z, quaternion.W * _windingSwap);
    }

    AmReal32 CartesianCoordinateSystem::Converter::Forward(const AmReal32& scalar) const
    {
        return scalar * _fromToScalar;
    }

    AmVec3 CartesianCoordinateSystem::Converter::Backward(const AmVec3& vector) const
    {
        return _toFromMatrix * vector * _toFromScalar;
    }

    AmQuat CartesianCoordinateSystem::Converter::Backward(const AmQuat& quaternion) const
    {
        const AmVec3 axis = _toFromMatrix * quaternion.XYZ;
        return AM_Q(axis.X, axis.Y, axis.Z, quaternion.W * _windingSwap);
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

    AmVec3 CartesianCoordinateSystem::Convert(
        const AmVec3& vector, const CartesianCoordinateSystem& from, const CartesianCoordinateSystem& to)
    {
        return to.Convert(vector, from);
    }

    AmVec3 CartesianCoordinateSystem::ConvertToDefault(const AmVec3& vector, const CartesianCoordinateSystem& from)
    {
        return Convert(vector, from, Default());
    }

    AmVec3 CartesianCoordinateSystem::GetVector(Axis axis)
    {
        switch (axis)
        {
        case Axis::PositiveX:
            return AM_V3(1.0f, 0.0f, 0.0f);
        case Axis::NegativeX:
            return AM_V3(-1.0f, 0.0f, 0.0f);
        case Axis::PositiveY:
            return AM_V3(0.0f, 1.0f, 0.0f);
        case Axis::NegativeY:
            return AM_V3(0.0f, -1.0f, 0.0f);
        case Axis::PositiveZ:
            return AM_V3(0.0f, 0.0f, 1.0f);
        case Axis::NegativeZ:
            return AM_V3(0.0f, 0.0f, -1.0f);
        default:
            AMPLITUDE_ASSERT(false);
            return AM_V3(0.0f, 0.0f, 0.0f);
        }
    }

    CartesianCoordinateSystem::CartesianCoordinateSystem(Axis rigth, Axis forward, Axis up)
        : _rightAxis(rigth)
        , _forwardAxis(forward)
        , _upAxis(up)
    {}

    AmVec3 CartesianCoordinateSystem::Convert(const AmVec3& vector, const CartesianCoordinateSystem& from) const
    {
        const Converter converter(from, *this);
        return converter.Forward(vector);
    }

    AmQuat CartesianCoordinateSystem::Convert(const AmQuat& quaternion, const CartesianCoordinateSystem& from) const
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
