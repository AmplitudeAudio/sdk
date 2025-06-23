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

#include <Math/LinearAlgebra.h>

namespace SparkyStudios::Audio::Amplitude
{
    bool BarycentricCoordinates::RayTriangleIntersection(
        const AmVector3& rayOrigin, const AmVector3& rayDirection, const Triangle& triangle, BarycentricCoordinates& result)
    {
        const auto e1 = Sub(triangle[1], triangle[0]);
        const auto e2 = Sub(triangle[2], triangle[0]);
        const auto r2 = Cross(rayDirection, e2);

        const AmReal32 det = Dot(e1, r2);
        if (det > -kEpsilon && det < kEpsilon)
            return false; // Ray parallel to triangle

        const AmReal32 invDet = 1.0f / det;
        const auto s = Sub(rayOrigin, triangle[0]);
        const AmReal32 v = invDet * Dot(s, r2);

        if (v < -kEpsilon || v > 1.0f + kEpsilon)
            return false;

        const auto s1 = Cross(s, e1);
        const AmReal32 w = invDet * Dot(rayDirection, s1);

        if (w < -kEpsilon || v + w > 1.0f + kEpsilon)
            return false;

        if (const AmReal32 t = invDet * Dot(e2, s1); t >= 0.0f)
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

    BarycentricCoordinates::BarycentricCoordinates(const AmVector3& position, const Triangle& triangle)
        : BarycentricCoordinates()
    {
        const auto ab = Sub(triangle[1], triangle[0]);
        const auto ac = Sub(triangle[2], triangle[0]);
        const auto ap = Sub(position, triangle[0]);

        const AmReal32 d1 = Dot(ab, ab);
        const AmReal32 d2 = Dot(ab, ac);
        const AmReal32 d3 = Dot(ac, ac);
        const AmReal32 d4 = Dot(ap, ab);
        const AmReal32 d5 = Dot(ap, ac);

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
