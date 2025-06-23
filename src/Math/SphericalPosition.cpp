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

#include <SparkyStudios/Audio/Amplitude/Math/SphericalPosition.h>
#include <SparkyStudios/Audio/Amplitude/Math/Utils.h>

#include <Math/LinearAlgebra.h>

namespace SparkyStudios::Audio::Amplitude
{
    SphericalPosition SphericalPosition::FromWorldSpace(const AmVector3& position)
    {
        const AmReal32 radius = Length(position);

        return { -std::atan2(position.y, position.x), std::atan2(position.z, Length(position.xy)), radius };
    }

    SphericalPosition SphericalPosition::ForHRTF(const AmVector3& position)
    {
        SphericalPosition hrtfPosition = FromWorldSpace(position);
        // Apply a 90-degree flip in azimuth to convert position to an Ears-centered coordinate system used for HRTF
        hrtfPosition._azimuth += 90.0f * AM_DegToRad;
        return hrtfPosition;
    }

    SphericalPosition SphericalPosition::FromDegrees(AmReal32 azimuthDegrees, AmReal32 elevationDegrees, AmReal32 radius)
    {
        return { azimuthDegrees * AM_DegToRad, elevationDegrees * AM_DegToRad, radius };
    }

    SphericalPosition::SphericalPosition(AmReal32 azimuth, AmReal32 elevation, AmReal32 radius)
        : _azimuth(azimuth)
        , _elevation(elevation)
        , _radius(radius)
    {}

    SphericalPosition SphericalPosition::FlipAzimuth() const
    {
        SphericalPosition flippedAzimuth = *this;
        flippedAzimuth._azimuth = -_azimuth;
        return flippedAzimuth;
    }

    SphericalPosition SphericalPosition::Rotate(AmQuaternion rotation) const
    {
        const AmVector3 rotatedPosition = RotateVector(ToCartesian(), rotation);
        return FromWorldSpace(rotatedPosition);
    }

    AmVector3 SphericalPosition::ToCartesian() const
    {
        // Translates spherical to cartesian, where positive Z - up, positive Y - forward, positive X - right
        const AmReal32 x = +_radius * std::cos(_elevation) * std::cos(_azimuth);
        const AmReal32 y = -_radius * std::cos(_elevation) * std::sin(_azimuth);
        const AmReal32 z = +_radius * std::sin(_elevation);

        return { x, y, z };
    }

    bool SphericalPosition::operator==(const SphericalPosition& other) const
    {
        return _azimuth == other._azimuth && _elevation == other._elevation && _radius == other._radius;
    }

    bool SphericalPosition::operator!=(const SphericalPosition& other) const
    {
        return !(*this == other);
    }
} // namespace SparkyStudios::Audio::Amplitude
