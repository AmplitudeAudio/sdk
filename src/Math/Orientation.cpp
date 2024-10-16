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

namespace SparkyStudios::Audio::Amplitude
{
    Orientation Orientation::Zero()
    {
        return { 0.0f, 0.0f, 0.0 };
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

    Orientation::Orientation(AmVec3 forward, AmVec3 up)
        : _forward(AM_Norm(forward))
        , _yaw(0)
        , _up(AM_Norm(up))
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

    Orientation::Orientation(AmQuat quaternion)
        : _forward()
        , _yaw(0)
        , _up()
        , _pitch(0)
        , _roll(0)
        , _alpha(0)
        , _beta(0)
        , _gamma(0)
        , _quaternion(quaternion)
    {
        _forward = AM_RotateV3Q(AM_V3(0, 1, 0), quaternion);
        _up = AM_RotateV3Q(AM_V3(0, 0, 1), quaternion);

        ComputeZYXAngles();
        ComputeZYZAngles();
    }

    AmMat4 Orientation::GetRotationMatrix() const
    {
        const AmMat4 rZ = AM_Rotate_RH(_yaw, AM_V3(0, 0, 1));
        const AmMat4 rY = AM_Rotate_RH(_pitch, AM_V3(0, 1, 0));
        const AmMat4 rX = AM_Rotate_RH(_roll, AM_V3(1, 0, 0));

        return (rZ * rY) * rX;
    }

    AmMat4 Orientation::GetLookAtMatrix(AmVec3 eye) const
    {
        const AmVec3 yAxis = _forward;
        const AmVec3 xAxis = AM_Cross(yAxis, _up);
        const AmVec3 zAxis = AM_Cross(xAxis, yAxis);

        AmMat4 rotation = AM_M4D(1.0f);
        rotation[0][0] = xAxis.X;
        rotation[0][1] = yAxis.X;
        rotation[0][2] = zAxis.X;
        rotation[1][0] = xAxis.Y;
        rotation[1][1] = yAxis.Y;
        rotation[1][2] = zAxis.Y;
        rotation[2][0] = xAxis.Z;
        rotation[2][1] = yAxis.Z;
        rotation[2][2] = zAxis.Z;

        AmMat4 translation = AM_M4D(1.0f);
        translation[3][0] = -eye.X;
        translation[3][1] = -eye.Y;
        translation[3][2] = -eye.Z;

        return rotation * translation;
    }

    void Orientation::ComputeForwardAndUpVectors()
    {
        _forward = AM_V3(0, 1, 0);
        _up = AM_V3(0, 0, 1);

        const AmMat4& rotation = GetRotationMatrix();

        _forward = (rotation * AM_V4V(_forward, 1.0f)).XYZ;
        _up = (rotation * AM_V4V(_up, 1.0f)).XYZ;
    }

    void Orientation::ComputeZYXAngles()
    {
        AmVec3 right = AM_Cross(_forward, _up);
        AmVec3 up = AM_Cross(right, _forward);

        if (AM_Dot(up, _up) < 0)
            up = -up;

        // Compute yaw (rotation around Z-axis)
        _yaw = -std::atan2(_forward.X, right.X);

        // Compute pitch (rotation around Y-axis)
        _pitch = std::asin(-up.X);

        // Compute roll (rotation around X-axis)
        _roll = -std::atan2(_up.Y, _up.Z);
    }

    void Orientation::ComputeZYZAngles()
    {
        if (const AmMat4 rotation = GetRotationMatrix(); std::abs(rotation[2][2]) - 1.0f < 0.0f)
        {
            _alpha = std::atan2(rotation[1][2], rotation[0][2]);
            _beta = std::acos(rotation[2][2]);
            _gamma = std::atan2(rotation[2][1], -rotation[2][0]);
        }
        else
        {
            _alpha = 0;
            _beta = rotation[2][2] < 0 ? AM_PI32 : 0.0f;
            _gamma = std::atan2(rotation[0][1], rotation[0][0]);
        }
    }

    void Orientation::ComputeQuaternion()
    {
        _quaternion = AM_M4ToQ_RH(GetRotationMatrix());
    }
} // namespace SparkyStudios::Audio::Amplitude