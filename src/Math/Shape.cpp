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
                return BoxShape::Create(box);
            }

        case AbstractShapeDefinition_Capsule:
            {
                const CapsuleShapeDefinition* capsule = definition->shape_as_Capsule();
                return CapsuleShape::Create(capsule);
            }

        case AbstractShapeDefinition_Cone:
            {
                const ConeShapeDefinition* cone = definition->shape_as_Cone();
                return ConeShape::Create(cone);
            }

        case AbstractShapeDefinition_Sphere:
            {
                const SphereShapeDefinition* sphere = definition->shape_as_Sphere();
                return SphereShape::Create(sphere);
            }
        }
    }

    Shape::Shape()
        : m_lookAtMatrix(AM_M4D(1.0f))
        , m_needUpdate(true)
        , m_location()
        , m_direction()
        , m_up()
    {}

    AmReal32 Shape::GetShortestDistanceToEdge(const Entity& entity)
    {
        return GetShortestDistanceToEdge(entity.GetLocation());
    }

    AmReal32 Shape::GetShortestDistanceToEdge(const Listener& listener)
    {
        return GetShortestDistanceToEdge(listener.GetLocation());
    }

    bool Shape::Contains(const Entity& entity)
    {
        const AmVec3& x = entity.GetLocation();
        return Contains(x);
    }

    bool Shape::Contains(const Listener& listener)
    {
        const AmVec3& x = listener.GetLocation();
        return Contains(x);
    }

    void Shape::SetLocation(const AmVec3& location)
    {
        m_location = location;

        m_lookAtMatrix = AM_LookAt_RH(m_location, m_location + m_direction, m_up);
        m_needUpdate = true;
    }

    void Shape::SetOrientation(const AmVec3& direction, const AmVec3& up)
    {
        m_direction = direction;
        m_up = up;

        m_lookAtMatrix = AM_LookAt_RH(m_location, m_location + m_direction, m_up);
        m_needUpdate = true;
    }

    const AmMat4& Shape::GetLookAt() const
    {
        return m_lookAtMatrix;
    }

    const AmVec3& Shape::GetLocation() const
    {
        return m_location;
    }

    const AmVec3& Shape::GetDirection() const
    {
        return m_direction;
    }

    const AmVec3& Shape::GetUp() const
    {
        return m_up;
    }

    Zone::Zone(Shape* inner, Shape* outer)
        : m_innerShape(inner)
        , m_outerShape(outer)
    {}

    void Zone::SetLocation(const AmVec3& location)
    {
        m_innerShape->SetLocation(location);
        m_outerShape->SetLocation(location);
    }

    const AmVec3& Zone::GetLocation() const
    {
        return m_innerShape->GetLocation();
    }

    const AmVec3& Zone::GetDirection() const
    {
        return m_innerShape->GetDirection();
    }

    const AmVec3& Zone::GetUp() const
    {
        return m_innerShape->GetUp();
    }

    void Zone::SetOrientation(const AmVec3& direction, const AmVec3& up)
    {
        m_innerShape->SetOrientation(direction, up);
        m_outerShape->SetOrientation(direction, up);
    }

    BoxShape* BoxShape::Create(const BoxShapeDefinition* definition)
    {
        return new BoxShape(definition->half_width(), definition->half_height(), definition->half_depth());
    }

    BoxShape::BoxShape(const AmReal32 halfWidth, const AmReal32 halfHeight, const AmReal32 halfDepth)
        : _halfWidth(halfWidth)
        , _halfHeight(halfHeight)
        , _halfDepth(halfDepth)
        , _u()
        , _v()
        , _w()
        , _p1()
        , _p2()
        , _p3()
        , _p4()
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

    void BoxShape::SetHalfWidth(const AmReal32 halfWidth)
    {
        _halfWidth = halfWidth;
        m_needUpdate = true;
    }

    void BoxShape::SetHalfHeight(const AmReal32 halfHeight)
    {
        _halfHeight = halfHeight;
        m_needUpdate = true;
    }

    void BoxShape::SetHalfDepth(const AmReal32 halfDepth)
    {
        _halfDepth = halfDepth;
        m_needUpdate = true;
    }

    AmReal32 BoxShape::GetShortestDistanceToEdge(const AmVec3& location)
    {
        if (m_needUpdate)
            _update();

        switch (amEngine->GetState()->up_axis)
        {
        default:
            [[fallthrough]];
        case eGameEngineUpAxis_Y:
            {
                const AmReal32 dP1 = AM_Dot(location - _p1, AM_Norm(_p2 - _p1));
                const AmReal32 dP2 = AM_Dot(location - _p2, AM_Norm(_p1 - _p2));
                const AmReal32 dP3 = AM_Dot(location - _p3, AM_Norm(_p1 - _p3));
                const AmReal32 dP4 = AM_Dot(location - _p4, AM_Norm(_p1 - _p4));
                const AmReal32 dP5 = AM_Dot(location - _p1, AM_Norm(_p3 - _p1));
                const AmReal32 dP6 = AM_Dot(location - _p1, AM_Norm(_p4 - _p1));

                return AM_MIN(dP1, AM_MIN(dP2, AM_MIN(dP3, AM_MIN(dP4, AM_MIN(dP5, dP6)))));
            }

        case eGameEngineUpAxis_Z:
            {
                const AmReal32 dP1 = AM_Dot(location - _p1, AM_Norm(_p2 - _p1));
                const AmReal32 dP2 = AM_Dot(location - _p2, AM_Norm(_p1 - _p2));
                const AmReal32 dP3 = AM_Dot(location - _p3, AM_Norm(_p1 - _p3));
                const AmReal32 dP4 = AM_Dot(location - _p4, AM_Norm(_p1 - _p4));
                const AmReal32 dP5 = AM_Dot(location - _p1, AM_Norm(_p3 - _p1));
                const AmReal32 dP6 = AM_Dot(location - _p1, AM_Norm(_p4 - _p1));

                return AM_MIN(dP1, AM_MIN(dP2, AM_MIN(dP3, AM_MIN(dP4, AM_MIN(dP5, dP6)))));
            }
        }
    }

    bool BoxShape::Contains(const AmVec3& location)
    {
        if (m_needUpdate)
            _update();

        const AmReal32 ux = AM_Dot(_u, location);
        const AmReal32 vx = AM_Dot(_v, location);
        const AmReal32 wx = AM_Dot(_w, location);

        if (AM_BETWEEN(ux, _uP1, _uP2) && AM_BETWEEN(vx, _vP1, _vP3) && AM_BETWEEN(wx, _wP1, _wP4))
            return true;

        return false;
    }

    void BoxShape::_update()
    {
        switch (amEngine->GetState()->up_axis)
        {
        default:
        case eGameEngineUpAxis_Y:
            _p1 = AM_Mul(m_lookAtMatrix, AM_V4(-_halfWidth, -_halfHeight, -_halfDepth, 1.0f)).XYZ;
            _p2 = AM_Mul(m_lookAtMatrix, AM_V4(-_halfWidth, -_halfHeight, _halfDepth, 1.0f)).XYZ;
            _p3 = AM_Mul(m_lookAtMatrix, AM_V4(_halfWidth, -_halfHeight, -_halfDepth, 1.0f)).XYZ;
            _p4 = AM_Mul(m_lookAtMatrix, AM_V4(-_halfWidth, _halfHeight, -_halfDepth, 1.0f)).XYZ;
            break;

        case eGameEngineUpAxis_Z:
            _p1 = AM_Mul(m_lookAtMatrix, AM_V4(-_halfWidth, -_halfDepth, -_halfHeight, 1.0f)).XYZ;
            _p2 = AM_Mul(m_lookAtMatrix, AM_V4(-_halfWidth, _halfDepth, -_halfHeight, 1.0f)).XYZ;
            _p3 = AM_Mul(m_lookAtMatrix, AM_V4(_halfWidth, -_halfDepth, -_halfHeight, 1.0f)).XYZ;
            _p4 = AM_Mul(m_lookAtMatrix, AM_V4(-_halfWidth, -_halfDepth, _halfHeight, 1.0f)).XYZ;
            break;
        }

        _u = AM_Norm(_p2 - _p1);
        _v = AM_Norm(_p3 - _p1);
        _w = AM_Norm(_p4 - _p1);

        _uP1 = AM_Dot(_u, _p1);
        _vP1 = AM_Dot(_v, _p1);
        _wP1 = AM_Dot(_w, _p1);

        _uP2 = AM_Dot(_u, _p2);
        _vP3 = AM_Dot(_v, _p3);
        _wP4 = AM_Dot(_w, _p4);

        m_needUpdate = false;
    }

    CapsuleShape* CapsuleShape::Create(const CapsuleShapeDefinition* definition)
    {
        return new CapsuleShape(definition->radius(), definition->half_height());
    }

    CapsuleShape::CapsuleShape(const AmReal32 radius, const AmReal32 halfHeight)
        : _radius(radius)
        , _halfHeight(halfHeight)
        , _a()
        , _b()
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

    void CapsuleShape::SetRadius(const AmReal32 radius)
    {
        _radius = radius;
        m_needUpdate = true;
    }

    void CapsuleShape::SetHalfHeight(const AmReal32 halfHeight)
    {
        _halfHeight = halfHeight;
        m_needUpdate = true;
    }

    AmReal32 CapsuleShape::GetShortestDistanceToEdge(const AmVec3& location)
    {
        if (m_needUpdate)
            Update();

        const AmVec3 e = _b - _a;
        const AmVec3 m = AM_Cross(_a, _b);
        const AmVec3 u = location - _a;
        const AmVec3 v = location - _b;

        const AmReal32 distanceToAxis = AM_Len(m + AM_Cross(e, location)) / AM_Len(e);
        const AmReal32 distanceToA = AM_Len(u);
        const AmReal32 distanceToB = AM_Len(v);

        // TODO: Check if location is whithin the cylinder part of the capsule

        if (distanceToA <= _radius)
            return _radius - distanceToA;

        if (distanceToB <= _radius)
            return _radius - distanceToB;

        return _radius - distanceToAxis;
    }

    bool CapsuleShape::Contains(const AmVec3& location)
    {
        if (m_needUpdate)
            Update();

        const AmReal32 distanceToOrigin = AM_Len(location - m_location);
        const AmReal32 halfHeight = _halfHeight - _radius;

        const AmVec3 e = _b - _a;
        const AmVec3 m = AM_Cross(_a, _b);

        const AmReal32 distanceToAxis = AM_Len(m + AM_Cross(e, location)) / AM_Len(e);

        // Check if we are in the cylinder part of the capsule
        if (distanceToAxis <= _radius && distanceToOrigin <= halfHeight)
            return true;

        const AmReal32 distanceToA = AM_Len(location - _a);
        const AmReal32 distanceToB = AM_Len(location - _b);

        // Check if we are in one of the spherical parts of the capsule
        if (distanceToA <= _radius || distanceToB <= _radius)
            return true;

        return false;
    }

    void CapsuleShape::Update()
    {
        const AmReal32 halfHeight = _halfHeight - _radius;

        switch (amEngine->GetState()->up_axis)
        {
        default:
        case eGameEngineUpAxis_Y:
            _a = AM_Mul(m_lookAtMatrix, AM_V4(0.0f, halfHeight, 0.0f, 1.0f)).XYZ;
            _b = AM_Mul(m_lookAtMatrix, AM_V4(0.0f, -halfHeight, 0.0f, 1.0f)).XYZ;
            break;

        case eGameEngineUpAxis_Z:
            _a = AM_Mul(m_lookAtMatrix, AM_V4(0.0f, 0.0f, halfHeight, 1.0f)).XYZ;
            _b = AM_Mul(m_lookAtMatrix, AM_V4(0.0f, 0.0f, -halfHeight, 1.0f)).XYZ;
            break;
        }
    }

    ConeShape* ConeShape::Create(const ConeShapeDefinition* definition)
    {
        return new ConeShape(definition->radius(), definition->height());
    }

    ConeShape::ConeShape(const AmReal32 radius, const AmReal32 height)
        : _radius(radius)
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

    void ConeShape::SetRadius(const AmReal32 radius)
    {
        _radius = radius;
        m_needUpdate = true;
    }

    void ConeShape::SetHeight(const AmReal32 height)
    {
        _height = height;
        m_needUpdate = true;
    }

    AmReal32 ConeShape::GetShortestDistanceToEdge(const AmVec3& location)
    {
        if (m_needUpdate)
            Update();

        const AmVec3& shapeToLocation = location - m_location;
        const AmReal32 coneDist = AM_Dot(shapeToLocation, m_direction);

        if (coneDist < 0.0f || coneDist > _height)
            return false;

        const AmReal32 coneRadius = coneDist / _height * _radius;
        const AmReal32 d = AM_Len(shapeToLocation - coneDist * m_direction);

        return coneRadius - d;
    }

    bool ConeShape::Contains(const AmVec3& location)
    {
        if (m_needUpdate)
            Update();

        const AmVec3& shapeToLocation = location - m_location;
        const AmReal32 coneDist = AM_Dot(shapeToLocation, m_direction);

        if (coneDist < 0.0f || coneDist > _height)
            return false;

        const AmReal32 coneRadius = coneDist / _height * _radius;
        const AmReal32 d = AM_Len(shapeToLocation - coneDist * m_direction);

        if (d <= coneRadius)
            return true;

        return false;
    }

    void ConeShape::Update()
    {
        m_needUpdate = false;
    }

    SphereShape* SphereShape::Create(const SphereShapeDefinition* definition)
    {
        return new SphereShape(definition->radius());
    }

    SphereShape::SphereShape(const AmReal32 radius)
        : _radius(radius)
    {}

    AmReal32 SphereShape::GetRadius() const
    {
        return _radius;
    }

    AmReal32 SphereShape::GetDiameter() const
    {
        return _radius * 2.0f;
    }

    void SphereShape::SetRadius(const AmReal32 radius)
    {
        _radius = radius;
        m_needUpdate = true;
    }

    AmReal32 SphereShape::GetShortestDistanceToEdge(const AmVec3& location)
    {
        if (m_needUpdate)
            Update();

        const AmVec3& shapeToLocation = location - m_location;
        const AmReal32 distance = AM_Len(shapeToLocation);

        return _radius - distance;
    }

    bool SphereShape::Contains(const AmVec3& location)
    {
        if (m_needUpdate)
            Update();

        const AmVec3& shapeToLocation = location - m_location;
        const AmReal32 distance = AM_Len(shapeToLocation);

        if (distance > _radius)
            return false;

        return true;
    }

    void SphereShape::Update()
    {
        m_needUpdate = false;
    }

    BoxZone::BoxZone(BoxShape* inner, BoxShape* outer)
        : Zone(inner, outer)
    {}

    AmReal32 BoxZone::GetFactor(const AmVec3& position)
    {
        auto* inner = dynamic_cast<BoxShape*>(m_innerShape);
        auto* outer = dynamic_cast<BoxShape*>(m_outerShape);

        if (inner->m_needUpdate)
            inner->_update();

        if (outer->m_needUpdate)
            outer->_update();

        const eGameEngineUpAxis upAxis = amEngine->GetState()->up_axis;

        const AmVec3& x = position;

        const AmReal32 iUX = AM_Dot(inner->_u, x);
        const AmReal32 iVX = AM_Dot(inner->_v, x);
        const AmReal32 iWX = AM_Dot(inner->_w, x);

        const AmReal32 oUX = AM_Dot(outer->_u, x);
        const AmReal32 oVX = AM_Dot(outer->_v, x);
        const AmReal32 oWX = AM_Dot(outer->_w, x);

        if (AM_BETWEEN(iUX, inner->_uP1, inner->_uP2) && AM_BETWEEN(iVX, inner->_vP1, inner->_vP3) &&
            AM_BETWEEN(iWX, inner->_wP1, inner->_wP4))
            return 1.0f;

        if (!(AM_BETWEEN(oUX, outer->_uP1, outer->_uP2) && AM_BETWEEN(oVX, outer->_vP1, outer->_vP3) &&
              AM_BETWEEN(oWX, outer->_wP1, outer->_wP4)))
            return 0.0f;

        switch (upAxis)
        {
        default:
        case eGameEngineUpAxis_Y:
            {
                const AmReal32 dP1 =
                    AM_ABS(AM_Dot(x - outer->_p1, AM_Norm(outer->_p2 - outer->_p1))) / (outer->GetHalfDepth() - inner->GetHalfDepth());
                const AmReal32 dP2 =
                    AM_ABS(AM_Dot(x - outer->_p2, AM_Norm(outer->_p1 - outer->_p2))) / (outer->GetHalfDepth() - inner->GetHalfDepth());
                const AmReal32 dP3 =
                    AM_ABS(AM_Dot(x - outer->_p3, AM_Norm(outer->_p1 - outer->_p3))) / (outer->GetHalfWidth() - inner->GetHalfWidth());
                const AmReal32 dP4 =
                    AM_ABS(AM_Dot(x - outer->_p4, AM_Norm(outer->_p1 - outer->_p4))) / (outer->GetHalfHeight() - inner->GetHalfHeight());
                const AmReal32 dP5 =
                    AM_ABS(AM_Dot(x - outer->_p1, AM_Norm(outer->_p3 - outer->_p1))) / (outer->GetHalfWidth() - inner->GetHalfWidth());
                const AmReal32 dP6 =
                    AM_ABS(AM_Dot(x - outer->_p1, AM_Norm(outer->_p4 - outer->_p1))) / (outer->GetHalfHeight() - inner->GetHalfHeight());

                const AmReal32 shortestPath = AM_MIN(dP1, AM_MIN(dP2, AM_MIN(dP3, AM_MIN(dP4, AM_MIN(dP5, dP6)))));

                return AM_CLAMP(shortestPath, 0.0f, 1.0f);
            }

        case eGameEngineUpAxis_Z:
            {
                const AmReal32 dP1 =
                    AM_ABS(AM_Dot(x - outer->_p1, AM_Norm(outer->_p2 - outer->_p1))) / (outer->GetHalfHeight() - inner->GetHalfHeight());
                const AmReal32 dP2 =
                    AM_ABS(AM_Dot(x - outer->_p2, AM_Norm(outer->_p1 - outer->_p2))) / (outer->GetHalfHeight() - inner->GetHalfHeight());
                const AmReal32 dP3 =
                    AM_ABS(AM_Dot(x - outer->_p3, AM_Norm(outer->_p1 - outer->_p3))) / (outer->GetHalfWidth() - inner->GetHalfWidth());
                const AmReal32 dP4 =
                    AM_ABS(AM_Dot(x - outer->_p4, AM_Norm(outer->_p1 - outer->_p4))) / (outer->GetHalfDepth() - inner->GetHalfDepth());
                const AmReal32 dP5 =
                    AM_ABS(AM_Dot(x - outer->_p1, AM_Norm(outer->_p3 - outer->_p1))) / (outer->GetHalfWidth() - inner->GetHalfWidth());
                const AmReal32 dP6 =
                    AM_ABS(AM_Dot(x - outer->_p1, AM_Norm(outer->_p4 - outer->_p1))) / (outer->GetHalfDepth() - inner->GetHalfDepth());

                const AmReal32 shortestPath = AM_MIN(dP1, AM_MIN(dP2, AM_MIN(dP3, AM_MIN(dP4, AM_MIN(dP5, dP6)))));

                return AM_CLAMP(shortestPath, 0.0f, 1.0f);
            }
        }

        return 0.0f;
    }

    CapsuleZone::CapsuleZone(CapsuleShape* inner, CapsuleShape* outer)
        : Zone(inner, outer)
    {}

    AmReal32 CapsuleZone::GetFactor(const AmVec3& position)
    {
        auto* inner = dynamic_cast<CapsuleShape*>(m_innerShape);
        auto* outer = dynamic_cast<CapsuleShape*>(m_outerShape);

        if (inner->m_needUpdate)
            inner->Update();

        if (outer->m_needUpdate)
            outer->Update();

        const eGameEngineUpAxis upAxis = amEngine->GetState()->up_axis;

        const AmVec3& x = position;

        const AmReal32 distanceToOrigin = AM_Len(x - inner->GetLocation());

        const AmReal32 innerHalfHeight = inner->GetHalfHeight() - inner->GetRadius();
        const AmReal32 outerHalfHeight = outer->GetHalfHeight() - outer->GetRadius();

        AmVec3 iE = inner->_b - inner->_a;
        AmVec3 iM = AM_Cross(inner->_a, inner->_b);

        AmVec3 oE = outer->_b - outer->_a;
        AmVec3 oM = AM_Cross(outer->_a, outer->_b);

        const AmReal32 iDistanceToAxis = AM_Len(iM + AM_Cross(iE, x)) / AM_Len(iE);
        const AmReal32 oDistanceToAxis = AM_Len(oM + AM_Cross(oE, x)) / AM_Len(oE);

        const AmReal32 iDistanceToA = AM_Len(x - inner->_a);
        const AmReal32 iDistanceToB = AM_Len(x - inner->_b);

        const AmReal32 oDistanceToA = AM_Len(x - outer->_a);
        const AmReal32 oDistanceToB = AM_Len(x - outer->_b);

        if (iDistanceToAxis <= inner->GetRadius() && distanceToOrigin <= innerHalfHeight)
            return 1.0f;

        if (iDistanceToA <= inner->GetRadius() || iDistanceToB <= inner->GetRadius())
            return 1.0f;

        if (oDistanceToAxis >= outer->GetRadius() && distanceToOrigin >= outerHalfHeight)
            return 0.0f;

        const AmReal32 rDelta = 1.0f - (oDistanceToAxis - inner->GetRadius()) / (outer->GetRadius() - inner->GetRadius());
        const AmReal32 hDelta = 1.0f - (distanceToOrigin - inner->GetHalfHeight()) / (outer->GetHalfHeight() - inner->GetHalfHeight());

        const AmReal32 delta = AM_MIN(rDelta, hDelta);

        return AM_CLAMP(delta, 0.0f, 1.0f);
    }

    ConeZone::ConeZone(ConeShape* inner, ConeShape* outer)
        : Zone(inner, outer)
    {}

    AmReal32 ConeZone::GetFactor(const AmVec3& position)
    {
        auto* inner = dynamic_cast<ConeShape*>(m_innerShape);
        auto* outer = dynamic_cast<ConeShape*>(m_outerShape);

        if (inner->m_needUpdate)
            inner->Update();

        if (outer->m_needUpdate)
            outer->Update();

        const eGameEngineUpAxis upAxis = amEngine->GetState()->up_axis;

        const AmVec3& soundToListener = position - inner->GetLocation();
        const AmReal32 distance = AM_Len(soundToListener);

        if (!m_outerShape->Contains(position))
            return 0.0f;

        const AmReal32 coneDist = AM_Dot(soundToListener, inner->GetDirection());

        const AmReal32 innerConeRadius = coneDist / inner->GetHeight() * inner->GetRadius();
        const AmReal32 outerConeRadius = coneDist / outer->GetHeight() * outer->GetRadius();

        const AmReal32 d = AM_Len(soundToListener - coneDist * inner->GetDirection());

        if (d <= innerConeRadius)
            return 1.0f;

        if (d >= outerConeRadius)
            return 0.0f;

        const AmReal32 delta = (distance - innerConeRadius) / (outerConeRadius - innerConeRadius);

        return 1.0f - AM_CLAMP(delta, 0.0f, 1.0f);
    }

    SphereZone::SphereZone(SphereShape* inner, SphereShape* outer)
        : Zone(inner, outer)
    {}

    AmReal32 SphereZone::GetFactor(const AmVec3& position)
    {
        auto* inner = dynamic_cast<SphereShape*>(m_innerShape);
        auto* outer = dynamic_cast<SphereShape*>(m_outerShape);

        if (inner->m_needUpdate)
            inner->Update();

        if (outer->m_needUpdate)
            outer->Update();

        const AmVec3& soundToListener = position - inner->GetLocation();
        const AmReal32 distance = AM_Len(soundToListener);
        if (distance >= outer->GetRadius())
            return 0.0f;

        const AmReal32 delta = (distance - inner->GetRadius()) / (outer->GetRadius() - inner->GetRadius());

        return 1.0f - AM_CLAMP(delta, 0.0f, 1.0f);
    }
} // namespace SparkyStudios::Audio::Amplitude
