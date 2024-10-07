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

#include <SparkyStudios/Audio/Amplitude/Math/BarycentricCoordinates.h>

namespace SparkyStudios::Audio::Amplitude
{
    bool BarycentricCoordinates::RayTriangleIntersection(
        const AmVec3& rayOrigin, const AmVec3& rayDirection, const std::array<AmVec3, 3>& triangle, BarycentricCoordinates& result)
    {
        const AmVec3 e1 = triangle.at(1) - triangle.at(0);
        const AmVec3 e2 = triangle.at(2) - triangle.at(0);
        const AmVec3 r2 = AM_Cross(rayDirection, e2);

        const AmReal32 det = AM_Dot(e1, r2);
        if (det > -kEpsilon && det < kEpsilon)
            return false; // Ray parallel to triangle

        const AmReal32 invDet = 1.0f / det;
        const AmVec3 s = rayOrigin - triangle.at(0);
        const AmReal32 v = invDet * AM_Dot(s, r2);

        if (v < -kEpsilon || v > 1.0f + kEpsilon)
            return false;

        const AmVec3 s1 = AM_Cross(s, e1);
        const AmReal32 w = invDet * AM_Dot(rayDirection, s1);

        if (w < -kEpsilon || v + w > 1.0f + kEpsilon)
            return false;

        if (const AmReal32 t = invDet * AM_Dot(e2, s1); t >= 0.0f)
        {
            result.m_V = v;
            result.m_W = w;
            result.m_U = 1.0f - v - w;
            return true;
        }

        return false;
    }

    BarycentricCoordinates::BarycentricCoordinates()
        : m_U(-kEpsilon)
        , m_V(-kEpsilon)
        , m_W(-kEpsilon)
    {}

    BarycentricCoordinates::BarycentricCoordinates(const AmVec3& p, const std::array<AmVec3, 3>& triangle)
        : BarycentricCoordinates()
    {
        const AmVec3 ab = triangle.at(1) - triangle.at(0);
        const AmVec3 ac = triangle.at(2) - triangle.at(0);
        const AmVec3 ap = p - triangle.at(0);

        const AmReal32 d1 = AM_Dot(ab, ab);
        const AmReal32 d2 = AM_Dot(ab, ac);
        const AmReal32 d3 = AM_Dot(ac, ac);
        const AmReal32 d4 = AM_Dot(ap, ab);
        const AmReal32 d5 = AM_Dot(ap, ac);

        const AmReal32 d = d1 * d3 - d2 * d2;

        m_V = (d3 * d4 - d2 * d5) / d;
        m_W = (d1 * d5 - d2 * d4) / d;
        m_U = 1.0f - m_V - m_W;
    }

    bool BarycentricCoordinates::IsValid() const
    {
        return m_U >= -kEpsilon && m_V >= -kEpsilon && m_U + m_V <= 1.0f + kEpsilon;
    }
} // namespace SparkyStudios::Audio::Amplitude
