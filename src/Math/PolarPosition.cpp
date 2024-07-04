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

#include <SparkyStudios/Audio/Amplitude/Math/PolarPosition.h>

namespace SparkyStudios::Audio::Amplitude
{
    PolarPosition::PolarPosition(AmReal32 azimuth, AmReal32 elevation, AmReal32 radius)
        : m_Azimuth(azimuth)
        , m_Elevation(elevation)
        , m_Radius(radius)
    {}

    PolarPosition::PolarPosition(AmVec3 originPosition, AmVec3 originDirection, AmVec3 originUp, AmVec3 position)
    {
        if (originPosition == position)
        {
            m_Azimuth = 0;
            m_Elevation = 0;
            m_Radius = 0;
            return;
        }

        AmVec3 fwd = AM_Norm(originDirection);
        AmVec3 right = AM_Norm(AM_Cross(fwd, originUp));
        AmVec3 up = AM_Cross(right, fwd);
        AmVec3 pos = position - originPosition;

        m_Radius = AM_Len(pos);

        pos = AM_V3(AM_Dot(pos, right), AM_Dot(pos, up), AM_Dot(pos, fwd));

        if (AM_EqV3(up, AM_V3(0, 1, 0)))
        {
            m_Azimuth = std::atan2(pos.X, pos.Z);

            AmVec3 proj = AM_V3(pos.X, 0, pos.Z);
            m_Elevation = std::acos(AM_Dot(AM_Norm(pos), AM_Norm(proj)));

            if (pos.Y < 0)
                m_Elevation = -m_Elevation;
        }
        else if (AM_EqV3(up, AM_V3(0, 0, 1)))
        {
            m_Azimuth = std::atan2(pos.X, pos.Y);

            AmVec3 proj = AM_V3(pos.X, pos.Y, 0);
            m_Elevation = std::acos(AM_Dot(AM_Norm(pos), AM_Norm(proj)));

            if (pos.Z < 0)
                m_Elevation = -m_Elevation;
        }
    }

    AmVec3 PolarPosition::ToCartesian(GameEngineUpAxis upAxis) const
    {
        switch (upAxis)
        {
        case eUpAxis_Y:
            {
                // Translates spherical to cartesian, where Y - up, Z - forward, X - right
                const AmReal32 x = +m_Radius * std::cos(m_Elevation) * std::cos(m_Azimuth);
                const AmReal32 y = +m_Radius * std::sin(m_Elevation);
                const AmReal32 z = -m_Radius * std::cos(m_Elevation) * std::sin(m_Azimuth);

                return AM_V3(x, y, z);
            }
        case eUpAxis_Z:
            {
                // Translates spherical to cartesian, where Z - up, Y - forward, X - right
                const AmReal32 x = +m_Radius * std::cos(m_Elevation) * std::cos(m_Azimuth);
                const AmReal32 y = -m_Radius * std::cos(m_Elevation) * std::sin(m_Azimuth);
                const AmReal32 z = +m_Radius * std::sin(m_Elevation);

                return AM_V3(x, y, z);
            }
        }

        AMPLITUDE_ASSERT(false);
        return AM_V3(0, 0, 0);
    }
} // namespace SparkyStudios::Audio::Amplitude
