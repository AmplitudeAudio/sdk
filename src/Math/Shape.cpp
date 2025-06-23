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
#include <Math/LinearAlgebra.h>

#include "common_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    std::shared_ptr<Shape> Shape::Create(const ShapeDefinition* definition)
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
        : m_location()
        , m_orientation(Orientation::Zero())
        , m_lookAtMatrix(Identity4())
        , m_needUpdate(true)
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
        const AmVector3& x = entity.GetLocation();
        return Contains(x);
    }

    bool Shape::Contains(const Listener& listener)
    {
        const AmVector3& x = listener.GetLocation();
        return Contains(x);
    }

    void Shape::SetLocation(const AmVector3& location)
    {
        m_location = location;

        m_lookAtMatrix = m_orientation.GetLookAtMatrix(m_location);
        m_needUpdate = true;
        Update();
    }

    void Shape::SetOrientation(const Orientation& orientation)
    {
        m_orientation = orientation;

        m_lookAtMatrix = m_orientation.GetLookAtMatrix(m_location);
        m_needUpdate = true;
        Update();
    }

    const Orientation& Shape::GetOrientation() const
    {
        return m_orientation;
    }

    const AmMatrix4& Shape::GetLookAt() const
    {
        return m_lookAtMatrix;
    }

    const AmVector3& Shape::GetLocation() const
    {
        return m_location;
    }

    AmVector3 Shape::GetDirection() const
    {
        return m_orientation.GetForward();
    }

    AmVector3 Shape::GetUp() const
    {
        return m_orientation.GetUp();
    }

    Zone::Zone(std::shared_ptr<Shape> inner, std::shared_ptr<Shape> outer)
        : m_innerShape(inner)
        , m_outerShape(outer)
    {}

    Zone::~Zone()
    {
        m_innerShape.reset();
        m_outerShape.reset();
    }

    void Zone::SetLocation(const AmVector3& location)
    {
        m_innerShape->SetLocation(location);
        m_outerShape->SetLocation(location);
    }

    const AmVector3& Zone::GetLocation() const
    {
        return m_innerShape->GetLocation();
    }

    AmVector3 Zone::GetDirection() const
    {
        return m_innerShape->GetDirection();
    }

    AmVector3 Zone::GetUp() const
    {
        return m_innerShape->GetUp();
    }

    void Zone::SetOrientation(const Orientation& orientation)
    {
        m_innerShape->SetOrientation(orientation);
        m_outerShape->SetOrientation(orientation);
    }

    const Orientation& Zone::GetOrientation() const
    {
        return m_innerShape->GetOrientation();
    }

    std::shared_ptr<BoxShape> BoxShape::Create(const BoxShapeDefinition* definition)
    {
        return AmSharedPtr<BoxShape>::Make(definition->half_width(), definition->half_height(), definition->half_depth());
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

    BoxShape::BoxShape(const AmVector3& position, const AmVector3& dimensions)
        : _halfWidth(dimensions.x * 0.5f)
        , _halfHeight(dimensions.z * 0.5f)
        , _halfDepth(dimensions.y * 0.5f)
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
    {
        SetLocation(position);
    }

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

    AmReal32 BoxShape::GetShortestDistanceToEdge(const AmVector3& location)
    {
        if (m_needUpdate)
            Update();

        const AmReal32 dP1 = Dot(Sub(location, _p1), Normalize(Sub(_p2, _p1)));
        const AmReal32 dP2 = Dot(Sub(location, _p2), Normalize(Sub(_p1, _p2)));
        const AmReal32 dP3 = Dot(Sub(location, _p3), Normalize(Sub(_p1, _p3)));
        const AmReal32 dP4 = Dot(Sub(location, _p4), Normalize(Sub(_p1, _p4)));
        const AmReal32 dP5 = Dot(Sub(location, _p1), Normalize(Sub(_p3, _p1)));
        const AmReal32 dP6 = Dot(Sub(location, _p1), Normalize(Sub(_p4, _p1)));

        return std::min({ dP1, dP2, dP3, dP4, dP5, dP6 });
    }

    bool BoxShape::Contains(const AmVector3& location)
    {
        if (m_needUpdate)
            Update();

        const AmReal32 ux = Dot(_u, location);
        const AmReal32 vx = Dot(_v, location);
        const AmReal32 wx = Dot(_w, location);

        return (AM_BETWEEN(ux, _uP1, _uP2) && AM_BETWEEN(vx, _vP1, _vP3) && AM_BETWEEN(wx, _wP1, _wP4));
    }

    AmVector3 BoxShape::GetClosestPoint(const AmVector3& location) const
    {
        AmVector3 closestPoint;
        const AmVector3& relativeLocation = GetRelativeDirection(GetLocation(), GetOrientation().GetQuaternion(), location);
        closestPoint.x = std::clamp(relativeLocation.x, -_halfWidth, _halfWidth);
        closestPoint.y = std::clamp(relativeLocation.y, -_halfDepth, _halfDepth);
        closestPoint.z = std::clamp(relativeLocation.z, -_halfHeight, _halfHeight);

        return closestPoint;
    }

    std::array<AmVector3, 8> BoxShape::GetCorners() const
    {
        std::array<AmVector3, 8> corners;
        corners[0] = Transform(m_lookAtMatrix, { -_halfWidth, -_halfDepth, -_halfHeight, 1.0f }).xyz;
        corners[1] = Transform(m_lookAtMatrix, { -_halfWidth, _halfDepth, -_halfHeight, 1.0f }).xyz;
        corners[2] = Transform(m_lookAtMatrix, { _halfWidth, -_halfDepth, -_halfHeight, 1.0f }).xyz;
        corners[3] = Transform(m_lookAtMatrix, { -_halfWidth, -_halfDepth, _halfHeight, 1.0f }).xyz;
        corners[4] = Transform(m_lookAtMatrix, { _halfWidth, _halfDepth, _halfHeight, 1.0f }).xyz;
        corners[5] = Transform(m_lookAtMatrix, { _halfWidth, -_halfDepth, _halfHeight, 1.0f }).xyz;
        corners[6] = Transform(m_lookAtMatrix, { -_halfWidth, _halfDepth, _halfHeight, 1.0f }).xyz;
        corners[7] = Transform(m_lookAtMatrix, { _halfWidth, _halfDepth, -_halfHeight, 1.0f }).xyz;

        return corners;
    }

    bool BoxShape::operator==(const BoxShape& other) const
    {
        return GetCorners() == other.GetCorners();
    }

    bool BoxShape::operator!=(const BoxShape& other) const
    {
        return !(*this == other);
    }

    void BoxShape::Update()
    {
        _p1 = Transform(m_lookAtMatrix, { -_halfWidth, -_halfDepth, -_halfHeight, 1.0f }).xyz;
        _p2 = Transform(m_lookAtMatrix, { -_halfWidth, _halfDepth, -_halfHeight, 1.0f }).xyz;
        _p3 = Transform(m_lookAtMatrix, { _halfWidth, -_halfDepth, -_halfHeight, 1.0f }).xyz;
        _p4 = Transform(m_lookAtMatrix, { -_halfWidth, -_halfDepth, _halfHeight, 1.0f }).xyz;

        _u = Normalize(Sub(_p2, _p1));
        _v = Normalize(Sub(_p3, _p1));
        _w = Normalize(Sub(_p4, _p1));

        _uP1 = Dot(_u, _p1);
        _vP1 = Dot(_v, _p1);
        _wP1 = Dot(_w, _p1);

        _uP2 = Dot(_u, _p2);
        _vP3 = Dot(_v, _p3);
        _wP4 = Dot(_w, _p4);

        m_needUpdate = false;
    }

    std::shared_ptr<CapsuleShape> CapsuleShape::Create(const CapsuleShapeDefinition* definition)
    {
        return AmSharedPtr<CapsuleShape>::Make(definition->radius(), definition->half_height());
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

    AmReal32 CapsuleShape::GetShortestDistanceToEdge(const AmVector3& location)
    {
        if (m_needUpdate)
            Update();

        const AmVector3 e = Sub(_b, _a);
        const AmVector3 m = Cross(_a, _b);
        const AmVector3 u = Sub(location, _a);
        const AmVector3 v = Sub(location, _b);

        const AmReal32 distanceToAxis = Length(Add(m, Cross(e, location))) / Length(e);
        const AmReal32 distanceToA = Length(u);
        const AmReal32 distanceToB = Length(v);

        // TODO: Check if location is within the cylinder part of the capsule

        if (distanceToA <= _radius)
            return _radius - distanceToA;

        if (distanceToB <= _radius)
            return _radius - distanceToB;

        return _radius - distanceToAxis;
    }

    bool CapsuleShape::Contains(const AmVector3& location)
    {
        if (m_needUpdate)
            Update();

        const AmReal32 distanceToOrigin = AM_Len(location - m_location);
        const AmReal32 halfHeight = _halfHeight - _radius;

        const AmVector3 e = _b - _a;
        const AmVector3 m = AM_Cross(_a, _b);

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

    bool CapsuleShape::operator==(const CapsuleShape& other) const
    {
        return _a == other._a && _b == other._b && _radius == other._radius && _halfHeight == other._halfHeight;
    }

    bool CapsuleShape::operator!=(const CapsuleShape& other) const
    {
        return !(*this == other);
    }

    void CapsuleShape::Update()
    {
        const AmReal32 halfHeight = _halfHeight - _radius;

        _a = AM_Mul(m_lookAtMatrix, AM_V4(0.0f, 0.0f, halfHeight, 1.0f)).XYZ;
        _b = AM_Mul(m_lookAtMatrix, AM_V4(0.0f, 0.0f, -halfHeight, 1.0f)).XYZ;
    }

    std::shared_ptr<ConeShape> ConeShape::Create(const ConeShapeDefinition* definition)
    {
        return AmSharedPtr<ConeShape>::Make(definition->radius(), definition->height());
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

    AmReal32 ConeShape::GetShortestDistanceToEdge(const AmVector3& location)
    {
        if (m_needUpdate)
            Update();

        const AmVector3& shapeToLocation = location - m_location;
        const AmReal32 coneDist = Dot(shapeToLocation, m_orientation.GetForward());

        if (coneDist < 0.0f)
            return coneDist;

        if (coneDist >= _height)
            return _height - coneDist;

        const AmReal32 coneRadius = std::min((coneDist / _height) * _radius, _radius);
        const AmReal32 d = AM_Len(shapeToLocation - coneDist * m_orientation.GetForward());

        return coneRadius - d;
    }

    bool ConeShape::Contains(const AmVector3& location)
    {
        if (m_needUpdate)
            Update();

        const AmVector3& shapeToLocation = location - m_location;
        const AmReal32 coneDist = Dot(shapeToLocation, m_orientation.GetForward());

        if (coneDist < 0.0f || coneDist > _height)
            return false;

        const AmReal32 coneRadius = std::min((coneDist / _height) * _radius, _radius);
        const AmReal32 d = AM_Len(shapeToLocation - coneDist * m_orientation.GetForward());

        return d <= coneRadius;
    }

    bool ConeShape::operator==(const ConeShape& other) const
    {
        return _radius == other._radius && _height == other._height && m_location == other.m_location &&
            m_orientation.GetForward() == other.m_orientation.GetForward();
    }

    bool ConeShape::operator!=(const ConeShape& other) const
    {
        return !(*this == other);
    }

    void ConeShape::Update()
    {
        m_needUpdate = false;
    }

    std::shared_ptr<SphereShape> SphereShape::Create(const SphereShapeDefinition* definition)
    {
        return AmSharedPtr<SphereShape>::Make(definition->radius());
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

    AmReal32 SphereShape::GetShortestDistanceToEdge(const AmVector3& location)
    {
        if (m_needUpdate)
            Update();

        const AmVector3& shapeToLocation = location - m_location;
        const AmReal32 distance = AM_Len(shapeToLocation);

        return _radius - distance;
    }

    bool SphereShape::Contains(const AmVector3& location)
    {
        if (m_needUpdate)
            Update();

        const AmVector3& shapeToLocation = location - m_location;
        const AmReal32 distance = AM_Len(shapeToLocation);

        return distance <= _radius;
    }

    bool SphereShape::operator==(const SphereShape& other) const
    {
        return _radius == other._radius && m_location == other.m_location;
    }

    bool SphereShape::operator!=(const SphereShape& other) const
    {
        return !(*this == other);
    }

    void SphereShape::Update()
    {
        m_needUpdate = false;
    }

    BoxZone::BoxZone(std::shared_ptr<BoxShape> inner, std::shared_ptr<BoxShape> outer)
        : Zone(inner, outer)
    {}

    AmReal32 BoxZone::GetFactor(const AmVector3& position)
    {
        if (m_innerShape->Contains(position))
            return 1.0f;

        if (!m_outerShape->Contains(position))
            return 0.0f;

        auto* inner = dynamic_cast<BoxShape*>(m_innerShape.get());
        auto* outer = dynamic_cast<BoxShape*>(m_outerShape.get());

        if (inner->m_needUpdate)
            inner->Update();

        if (outer->m_needUpdate)
            outer->Update();

        const AmVector3& x = position;

        const AmReal32 iUX = Dot(inner->_u, x);
        const AmReal32 iVX = Dot(inner->_v, x);
        const AmReal32 iWX = Dot(inner->_w, x);

        const AmReal32 oUX = Dot(outer->_u, x);
        const AmReal32 oVX = Dot(outer->_v, x);
        const AmReal32 oWX = Dot(outer->_w, x);

        if (AM_BETWEEN(iUX, inner->_uP1, inner->_uP2) && AM_BETWEEN(iVX, inner->_vP1, inner->_vP3) &&
            AM_BETWEEN(iWX, inner->_wP1, inner->_wP4))
            return 1.0f;

        if (!(AM_BETWEEN(oUX, outer->_uP1, outer->_uP2) && AM_BETWEEN(oVX, outer->_vP1, outer->_vP3) &&
              AM_BETWEEN(oWX, outer->_wP1, outer->_wP4)))
            return 0.0f;

        const AmReal32 dP1 =
            std::abs(Dot(x - outer->_p1, AM_Norm(outer->_p2 - outer->_p1))) / (outer->GetHalfHeight() - inner->GetHalfHeight());
        const AmReal32 dP2 =
            std::abs(Dot(x - outer->_p2, AM_Norm(outer->_p1 - outer->_p2))) / (outer->GetHalfHeight() - inner->GetHalfHeight());
        const AmReal32 dP3 =
            std::abs(Dot(x - outer->_p3, AM_Norm(outer->_p1 - outer->_p3))) / (outer->GetHalfWidth() - inner->GetHalfWidth());
        const AmReal32 dP4 =
            std::abs(Dot(x - outer->_p4, AM_Norm(outer->_p1 - outer->_p4))) / (outer->GetHalfDepth() - inner->GetHalfDepth());
        const AmReal32 dP5 =
            std::abs(Dot(x - outer->_p1, AM_Norm(outer->_p3 - outer->_p1))) / (outer->GetHalfWidth() - inner->GetHalfWidth());
        const AmReal32 dP6 =
            std::abs(Dot(x - outer->_p1, AM_Norm(outer->_p4 - outer->_p1))) / (outer->GetHalfDepth() - inner->GetHalfDepth());

        const AmReal32 shortestPath = std::min({ dP1, dP2, dP3, dP4, dP5, dP6 });

        return std::clamp(shortestPath, 0.0f, 1.0f);
    }

    CapsuleZone::CapsuleZone(std::shared_ptr<CapsuleShape> inner, std::shared_ptr<CapsuleShape> outer)
        : Zone(inner, outer)
    {}

    AmReal32 CapsuleZone::GetFactor(const AmVector3& position)
    {
        if (m_innerShape->Contains(position))
            return 1.0f;

        if (!m_outerShape->Contains(position))
            return 0.0f;

        auto* inner = dynamic_cast<CapsuleShape*>(m_innerShape.get());
        auto* outer = dynamic_cast<CapsuleShape*>(m_outerShape.get());

        if (inner->m_needUpdate)
            inner->Update();

        if (outer->m_needUpdate)
            outer->Update();

        const AmVector3& x = position;

        const AmReal32 distanceToOrigin = AM_Len(x - inner->GetLocation());

        const AmReal32 innerHalfHeight = inner->GetHalfHeight() - inner->GetRadius();
        const AmReal32 outerHalfHeight = outer->GetHalfHeight() - outer->GetRadius();

        AmVector3 iE = inner->_b - inner->_a;
        AmVector3 iM = AM_Cross(inner->_a, inner->_b);

        AmVector3 oE = outer->_b - outer->_a;
        AmVector3 oM = AM_Cross(outer->_a, outer->_b);

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

        const AmReal32 delta = std::min(rDelta, hDelta);

        return std::clamp(delta, 0.0f, 1.0f);
    }

    ConeZone::ConeZone(std::shared_ptr<ConeShape> inner, std::shared_ptr<ConeShape> outer)
        : Zone(inner, outer)
    {}

    AmReal32 ConeZone::GetFactor(const AmVector3& position)
    {
        if (m_innerShape->Contains(position))
            return 1.0f;

        if (!m_outerShape->Contains(position))
            return 0.0f;

        auto* inner = dynamic_cast<ConeShape*>(m_innerShape.get());
        auto* outer = dynamic_cast<ConeShape*>(m_outerShape.get());

        if (inner->m_needUpdate)
            inner->Update();

        if (outer->m_needUpdate)
            outer->Update();

        const AmVector3& shapeToPosition = position - inner->GetLocation();
        const AmReal32 distance = AM_Len(shapeToPosition);

        const AmReal32 coneDist = Dot(shapeToPosition, inner->GetDirection());

        if (coneDist < 0.0f || coneDist > outer->GetHeight())
            return 0.0f;

        const AmReal32 innerConeRadius = std::min((coneDist / inner->GetHeight()) * inner->GetRadius(), inner->GetRadius());
        const AmReal32 outerConeRadius = std::min((coneDist / outer->GetHeight()) * outer->GetRadius(), outer->GetRadius());

        const AmReal32 d = AM_Len(shapeToPosition - coneDist * inner->GetDirection());

        // The location is on the direction axis
        if (d == 0.0f)
        {
            const AmReal32 delta = (coneDist - inner->GetHeight()) / (outer->GetHeight() - inner->GetHeight());
            return 1.0f - std::clamp(delta, 0.0f, 1.0f);
        }

        return outer->GetShortestDistanceToEdge(position) / outer->GetRadius();
    }

    SphereZone::SphereZone(std::shared_ptr<SphereShape> inner, std::shared_ptr<SphereShape> outer)
        : Zone(inner, outer)
    {}

    AmReal32 SphereZone::GetFactor(const AmVector3& position)
    {
        if (m_innerShape->Contains(position))
            return 1.0f;

        if (!m_outerShape->Contains(position))
            return 0.0f;

        auto* inner = dynamic_cast<SphereShape*>(m_innerShape.get());
        auto* outer = dynamic_cast<SphereShape*>(m_outerShape.get());

        if (inner->m_needUpdate)
            inner->Update();

        if (outer->m_needUpdate)
            outer->Update();

        const AmVector3& soundToListener = position - inner->GetLocation();
        const AmReal32 distance = AM_Len(soundToListener);
        if (distance >= outer->GetRadius())
            return 0.0f;

        const AmReal32 delta = (distance - inner->GetRadius()) / (outer->GetRadius() - inner->GetRadius());

        return 1.0f - std::clamp(delta, 0.0f, 1.0f);
    }
} // namespace SparkyStudios::Audio::Amplitude
