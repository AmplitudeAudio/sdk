// Copyright (c) 2021-present Sparky Studios. All rights reserved.
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

#include <SparkyStudios/Audio/Amplitude/Core/Engine.h>
#include <SparkyStudios/Audio/Amplitude/Math/Shape.h>

#include <Core/EngineInternalState.h>

#include "common_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    Shape* Shape::Create(const ShapeDefinition* definition)
    {
        if (definition == nullptr)
            return nullptr;

        switch (definition->shape_type())
        {
        default:
            return nullptr;

        case AbstractShapeDefinition_Box:
            {
                const BoxShapeDefinition* box = definition->shape_as_Box();
                return new BoxShape(box->half_width(), box->half_height(), box->half_depth());
            }
        }
    }

    Shape::Shape()
        : m_lookAtMatrix(AM_Mat4d(1.0f))
        , m_needUpdate(true)
    {}

    bool Shape::Contains(const Entity& entity)
    {
        const hmm_vec3& x = entity.GetLocation();
        return Contains(x);
    }

    bool Shape::Contains(const Listener& listener)
    {
        const hmm_vec3& x = listener.GetLocation();
        return Contains(x);
    }

    void Shape::SetPosition(const hmm_vec3& location)
    {
        m_location = location;

        m_lookAtMatrix = AM_LookAt(m_location, m_location + m_direction, m_up);
        m_needUpdate = true;
    }

    void Shape::SetOrientation(const hmm_vec3& direction, const hmm_vec3& up)
    {
        m_direction = direction;
        m_up = up;

        m_lookAtMatrix = AM_LookAt(m_location, m_location + m_direction, m_up);
        m_needUpdate = true;
    }

    const hmm_mat4& Shape::GetLookAt() const
    {
        return m_lookAtMatrix;
    }

    BoxShape::BoxShape(AmReal32 halfWidth, AmReal32 halfHeight, AmReal32 halfDepth)
        : Shape()
        , _halfWidth(halfWidth)
        , _halfHeight(halfHeight)
        , _halfDepth(halfDepth)
        , _u()
        , _v()
        , _w()
        , _uP1(0.0)
        , _vP1(0.0)
        , _wP1(0.0)
        , _uP2(0.0)
        , _vP3(0.0)
        , _wP4(0.0)
    {}

    AmReal32 BoxShape::GetHalfWidth() const
    {
        return _halfWidth;
    }

    AmReal32 BoxShape::GetHalfHeight() const
    {
        return _halfHeight;
    }

    AmReal32 BoxShape::GetHalfDepth() const
    {
        return _halfDepth;
    }

    AmReal32 BoxShape::GetWidth() const
    {
        return _halfWidth * 2.0f;
    }

    AmReal32 BoxShape::GetHeight() const
    {
        return _halfHeight * 2.0f;
    }

    AmReal32 BoxShape::GetDepth() const
    {
        return _halfDepth * 2.0f;
    }

    void BoxShape::SetHalfWidth(AmReal32 halfWidth)
    {
        _halfWidth = halfWidth;
        m_needUpdate = true;
    }

    void BoxShape::SetHalfHeight(AmReal32 halfHeight)
    {
        _halfHeight = halfHeight;
        m_needUpdate = true;
    }

    void BoxShape::SetHalfDepth(AmReal32 halfDepth)
    {
        _halfDepth = halfDepth;
        m_needUpdate = true;
    }

    bool BoxShape::Contains(const hmm_vec3& location)
    {
        if (m_needUpdate)
        {
            hmm_vec3 p1, p2, p3, p4;

            switch (amEngine->GetState()->up_axis)
            {
            default:
            case GameEngineUpAxis_Y:
                p1 = AM_Multiply(m_lookAtMatrix, AM_Vec4(-_halfWidth, -_halfHeight, -_halfDepth, 1.0f)).XYZ;
                p2 = AM_Multiply(m_lookAtMatrix, AM_Vec4(-_halfWidth, -_halfHeight, _halfDepth, 1.0f)).XYZ;
                p3 = AM_Multiply(m_lookAtMatrix, AM_Vec4(_halfWidth, -_halfHeight, -_halfDepth, 1.0f)).XYZ;
                p4 = AM_Multiply(m_lookAtMatrix, AM_Vec4(-_halfWidth, _halfHeight, -_halfDepth, 1.0f)).XYZ;
                break;

            case GameEngineUpAxis_Z:
                p1 = AM_Multiply(m_lookAtMatrix, AM_Vec4(-_halfWidth, -_halfDepth, -_halfHeight, 1.0f)).XYZ;
                p2 = AM_Multiply(m_lookAtMatrix, AM_Vec4(-_halfWidth, _halfDepth, -_halfHeight, 1.0f)).XYZ;
                p3 = AM_Multiply(m_lookAtMatrix, AM_Vec4(_halfWidth, -_halfDepth, -_halfHeight, 1.0f)).XYZ;
                p4 = AM_Multiply(m_lookAtMatrix, AM_Vec4(-_halfWidth, -_halfDepth, _halfHeight, 1.0f)).XYZ;
                break;
            }

            _u = AM_Normalize(p2 - p1);
            _v = AM_Normalize(p3 - p1);
            _w = AM_Normalize(p4 - p1);

            _uP1 = AM_Dot(_u, p1);
            _vP1 = AM_Dot(_v, p1);
            _wP1 = AM_Dot(_w, p1);

            _uP2 = AM_Dot(_u, p2);
            _vP3 = AM_Dot(_v, p3);
            _wP4 = AM_Dot(_w, p4);

            m_needUpdate = false;
        }

        const AmReal32 ux = AM_Dot(_u, location);
        const AmReal32 vx = AM_Dot(_v, location);
        const AmReal32 wx = AM_Dot(_w, location);

        if (AM_BETWEEN(ux, _uP1, _uP2) && AM_BETWEEN(vx, _vP1, _vP3) && AM_BETWEEN(wx, _wP1, _wP4))
            return true;

        return false;
    }

    CapsuleShape::CapsuleShape(AmReal32 radius, AmReal32 halfHeight)
        : Shape()
        , _radius(0.0)
        , _halfHeight(0.0)
    {}

    AmReal32 CapsuleShape::GetRadius() const
    {
        return _radius;
    }

    AmReal32 CapsuleShape::GetHalfHeight() const
    {
        return _halfHeight;
    }

    AmReal32 CapsuleShape::GetDiameter() const
    {
        return _radius * 2.0f;
    }

    AmReal32 CapsuleShape::GetHeight() const
    {
        return _halfHeight * 2.0f;
    }

    void CapsuleShape::SetRadius(AmReal32 radius)
    {
        _radius = radius;
        m_needUpdate = true;
    }

    void CapsuleShape::SetHalfHeight(AmReal32 halfHeight)
    {
        _halfHeight = halfHeight;
        m_needUpdate = true;
    }

    bool CapsuleShape::Contains(const hmm_vec3& location)
    {
        const AmReal32 distanceToOrigin = AM_Length(location - m_location);
        const AmReal32 halfHeight = _halfHeight - _radius;

        hmm_vec3 a, b;

        switch (amEngine->GetState()->up_axis)
        {
        default:
        case GameEngineUpAxis_Y:
            a = AM_Multiply(m_lookAtMatrix, AM_Vec4(0.0f, halfHeight, 0.0f, 1.0f)).XYZ;
            b = AM_Multiply(m_lookAtMatrix, AM_Vec4(0.0f, -halfHeight, 0.0f, 1.0f)).XYZ;
            break;

        case GameEngineUpAxis_Z:
            a = AM_Multiply(m_lookAtMatrix, AM_Vec4(0.0f, 0.0f, halfHeight, 1.0f)).XYZ;
            b = AM_Multiply(m_lookAtMatrix, AM_Vec4(0.0f, 0.0f, -halfHeight, 1.0f)).XYZ;
            break;
        }

        const hmm_vec3 e = b - a;
        const hmm_vec3 m = AM_Cross(a, b);

        const AmReal32 distanceToAxis = AM_Length(m + AM_Cross(e, location)) / AM_Length(e);

        // Check if we are in the cylinder part of the capsule
        if (distanceToAxis <= _radius && distanceToOrigin <= halfHeight)
            return true;

        const AmReal32 distanceToA = AM_Length(location - a);
        const AmReal32 distanceToB = AM_Length(location - b);

        // Check if we are in one of the spherical parts of the capsule
        if (distanceToA <= _radius || distanceToB <= _radius)
            return true;

        return false;
    }

    ConeShape::ConeShape(AmReal32 radius, AmReal32 height)
        : Shape()
        , _radius(radius)
        , _height(height)
    {}

    AmReal32 ConeShape::GetRadius() const
    {
        return _radius;
    }

    AmReal32 ConeShape::GetDiameter() const
    {
        return _radius * 2.0f;
    }

    AmReal32 ConeShape::GetHeight() const
    {
        return _height;
    }

    void ConeShape::SetRadius(AmReal32 radius)
    {
        _radius = radius;
        m_needUpdate = true;
    }

    void ConeShape::SetHeight(AmReal32 height)
    {
        _height = height;
        m_needUpdate = true;
    }

    bool ConeShape::Contains(const hmm_vec3& location)
    {
        const hmm_vec3& shapeToLocation = location - m_location;
        const AmReal32 coneDist = AM_Dot(shapeToLocation, m_direction);

        if (coneDist < 0.0f || coneDist > _height)
            return false;

        const AmReal32 coneRadius = coneDist / _height * _radius;
        const AmReal32 d = AM_Length(shapeToLocation - coneDist * m_direction);

        if (d <= coneRadius)
            return true;

        return false;
    }

    SphereShape::SphereShape(AmReal32 radius)
        : Shape()
        , _radius(radius)
    {}

    AmReal32 SphereShape::GetRadius() const
    {
        return _radius;
    }

    AmReal32 SphereShape::GetDiameter() const
    {
        return _radius * 2.0f;
    }

    void SphereShape::SetRadius(AmReal32 radius)
    {
        _radius = radius;
        m_needUpdate = true;
    }

    bool SphereShape::Contains(const hmm_vec3& location)
    {
        const hmm_vec3& shapeToLocation = location - m_location;
        const AmReal32 distance = AM_Length(shapeToLocation);

        if (distance > _radius)
            return false;

        return true;
    }
} // namespace SparkyStudios::Audio::Amplitude
