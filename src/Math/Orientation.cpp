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

#include <SparkyStudios/Audio/Amplitude/Math/Orientation.h>

namespace SparkyStudios::Audio::Amplitude
{
    Orientation Orientation::Zero()
    {
        return { 0.0f, 0.0f, 0.0 };
    }

    Orientation::Orientation(AmReal32 pitch, AmReal32 yaw, AmReal32 roll)
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

    void Orientation::ComputeForwardAndUpVectors()
    {
        _forward.X = std::sin(_yaw) * std::cos(_pitch);
        _forward.Y = std::cos(_yaw) * std::cos(_pitch);
        _forward.Z = std::sin(_roll);

        _up.X = std::sin(_yaw) * std::sin(_pitch);
        _up.Y = std::cos(_yaw) * std::sin(_pitch);
        _up.Z = std::cos(_pitch);
    }

    void Orientation::ComputeZYXAngles()
    {
        // Yaw is the bearing of the forward vector's shadow in the xy plane.
        _yaw = std::atan2(_forward.Y, _forward.X);

        // Find the vector in the xy plane.
        const AmReal32 planeRightX = std::sin(_yaw);
        const AmReal32 planeRightY = std::cos(_yaw);

        // Pitch is the leftward lean of our up vector, computed here using a dot product.
        _pitch = std::asin(_up.X * planeRightX + _up.Y * planeRightY);

        // If we're twisted upside-down, return a roll in the range +-(pi/2, pi)
        if (_up.Z < 0)
            _pitch = static_cast<AmReal32>((0.0f <= _pitch) - (_pitch < 0.0f)) * AM_PI32 - _pitch;

        // Roll is the altitude of the forward vector off the xy plane, toward the up direction.
        _roll = std::asin(_forward.Z);
    }

    void Orientation::ComputeZYZAngles()
    {
        const AmReal32 cosYaw = std::cos(_yaw);
        const AmReal32 sinYaw = std::sin(_yaw);
        const AmReal32 cosPitch = std::cos(_pitch);
        const AmReal32 sinPitch = std::sin(_pitch);
        const AmReal32 cosRoll = std::cos(_roll);
        const AmReal32 sinRoll = std::sin(_roll);

        // Convert Yaw-Pitch-Roll (ZYX convention) to Alpha-Beta-Gamma (ZYZ convention)
        if (const AmReal32 m33 = cosPitch * cosRoll; m33 == 1.0f)
        {
            _beta = 0.0f;
            _gamma = 0.0f;
            _alpha = std::atan2(sinYaw, cosYaw);
        }
        else if (m33 == -1.0f)
        {
            _beta = AM_PI32;
            _gamma = 0.0f;
            _alpha = std::atan2(-sinYaw, cosYaw);
        }
        else
        {
            const AmReal32 m32 = sinYaw * sinPitch * sinRoll + cosYaw * cosRoll;
            const AmReal32 m31 = cosYaw * sinPitch * sinRoll - sinYaw * cosRoll;
            _alpha = std::atan2(m32, m31);

            _beta = std::acos(m33);

            const AmReal32 m23 = cosPitch * sinRoll;
            const AmReal32 m13 = sinPitch;
            _gamma = std::atan2(m23, m13);
        }
    }

    void Orientation::ComputeQuaternion()
    {
        const AmMat4 rZ = AM_Rotate_LH(_yaw, AM_V3(0, 0, 1));
        const AmMat4 rY = AM_Rotate_LH(_pitch, AM_V3(0, 1, 0));
        const AmMat4 rX = AM_Rotate_LH(_roll, AM_V3(1, 0, 0));

        const AmMat4 rZYX = rZ * rY * rX;

        _quaternion = AM_M4ToQ_LH(rZYX);
    }
} // namespace SparkyStudios::Audio::Amplitude