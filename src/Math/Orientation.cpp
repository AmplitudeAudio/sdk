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
#include <SparkyStudios/Audio/Amplitude/Math/Orientation.h>
#include <SparkyStudios/Audio/Amplitude/Math/Utils.h>

#include <Math/LinearAlgebra.h>

#include <algorithm>

namespace SparkyStudios::Audio::Amplitude
{
    Orientation Orientation::Zero()
    {
        return { 0.0f, 0.0f, 0.0f };
    }

    Orientation::Orientation(AmReal32 yaw, AmReal32 pitch, AmReal32 roll)
        : _forward()
        , _yaw(yaw)
        , _up()
        , _pitch(pitch)
        , _roll(roll)
        , _alpha(0)
        , _beta(0)
        , _gamma(0)
        , _quaternion()
    {
        ComputeForwardAndUpVectors();
        ComputeZYZAngles();
        ComputeQuaternion();
    }

    Orientation::Orientation(AmVector3 forward, AmVector3 up)
        : _forward(Normalize(std::move(forward)))
        , _yaw(0)
        , _up(Normalize(std::move(up)))
        , _pitch(0)
        , _roll(0)
        , _alpha(0)
        , _beta(0)
        , _gamma(0)
        , _quaternion()
    {
        ComputeZYXAngles();
        ComputeZYZAngles();
        ComputeQuaternion();
    }

    Orientation::Orientation(AmQuaternion quaternion)
        : _forward()
        , _yaw(0)
        , _up()
        , _pitch(0)
        , _roll(0)
        , _alpha(0)
        , _beta(0)
        , _gamma(0)
        , _quaternion(std::move(quaternion))
    {
        _forward = RotateVector(kVector3UnitY, _quaternion);
        _up = RotateVector(kVector3UnitZ, _quaternion);

        ComputeZYXAngles();
        ComputeZYZAngles();
    }

    AmMatrix3 Orientation::GetRotationMatrix() const
    {
        const auto rZ = Eigen::AngleAxisf(_yaw, VecToEigen(kVector3UnitZ));
        const auto rY = Eigen::AngleAxisf(_pitch, VecToEigen(kVector3UnitY));
        const auto rX = Eigen::AngleAxisf(_roll, VecToEigen(kVector3UnitX));
        return EigenToMat3((rZ * rY * rX).matrix());
    }

    AmMatrix4 Orientation::GetLookAtMatrix(AmVector3 eye) const
    {
        const auto yAxis = VecToEigen(_forward);
        const auto xAxis = yAxis.cross(VecToEigen(_up));
        const auto zAxis = xAxis.cross(yAxis);

        // Create the rotation matrix
        Eigen::Matrix4f rotation = Eigen::Matrix4f::Identity();
        rotation.block<1, 3>(0, 0) = xAxis;
        rotation.block<1, 3>(1, 0) = yAxis;
        rotation.block<1, 3>(2, 0) = zAxis;

        // Create the translation matrix
        Eigen::Matrix4f translation = Eigen::Matrix4f::Identity();
        translation.block<3, 1>(0, 3) = -VecToEigen(eye);

        return EigenToMat(Eigen::Matrix4f(rotation * translation));
    }

    void Orientation::ComputeForwardAndUpVectors()
    {
        const AmMatrix3& rotation = GetRotationMatrix();

        _forward = Normalize(Transform(rotation, kVector3UnitY));
        _up = Normalize(Transform(rotation, kVector3UnitZ));
    }

    void Orientation::ComputeZYXAngles()
    {
        // Rebuild the orthonormal basis of the rotation. Engine convention:
        // +X right, +Y forward, +Z up (see CartesianCoordinateSystem::Default()).
        const AmVector3 right = Normalize(Cross(_forward, _up));
        const AmVector3 up = Normalize(Cross(right, _forward));

        // Invert R = Rz(yaw) * Ry(pitch) * Rx(roll) from the basis vectors:
        // right = R * +X, forward = R * +Y, up = R * +Z.
        _yaw = std::atan2(right.y, right.x);
        _pitch = std::asin(std::clamp(-right.z, -1.0f, 1.0f));
        _roll = std::atan2(_forward.z, up.z);
    }

    void Orientation::ComputeZYZAngles()
    {
        if (const AmMatrix3 rotation = GetRotationMatrix(); std::abs(rotation[2][2]) - 1.0f < 0.0f)
        {
            _alpha = std::atan2(rotation[2][1], rotation[2][0]);
            _beta = std::acos(rotation[2][2]);
            _gamma = std::atan2(rotation[1][2], -rotation[0][2]);
        }
        else
        {
            _alpha = 0;
            _beta = rotation[2][2] < 0 ? AM_PI32 : 0.0f;
            _gamma = std::atan2(rotation[1][0], rotation[0][0]);
        }
    }

    void Orientation::ComputeQuaternion()
    {
        _quaternion = EigenToQuat(Eigen::Quaternionf(MatToEigen(GetRotationMatrix())).normalized());
    }
} // namespace SparkyStudios::Audio::Amplitude
