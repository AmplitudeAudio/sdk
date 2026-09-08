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
        , m_lookAtMatrix(kMatrix4Identity)
        , m_needUpdate(true)
    {}

    AmReal32 Shape::GetShortestDistanceToEdge(const Entity& entity) const
    {
        return GetShortestDistanceToEdge(entity.GetLocation());
    }

    AmReal32 Shape::GetShortestDistanceToEdge(const Listener& listener) const
    {
        return GetShortestDistanceToEdge(listener.GetLocation());
    }

    bool Shape::Contains(const Entity& entity) const
    {
        const AmVector3& x = entity.GetLocation();
        return Contains(x);
    }

    bool Shape::Contains(const Listener& listener) const
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
        UpdateIfNeeded();
        return m_orientation;
    }

    const AmMatrix4& Shape::GetLookAt() const
    {
        UpdateIfNeeded();
        return m_lookAtMatrix;
    }

    const AmVector3& Shape::GetLocation() const
    {
        UpdateIfNeeded();
        return m_location;
    }

    AmVector3 Shape::GetDirection() const
    {
        UpdateIfNeeded();
        return m_orientation.GetForward();
    }

    AmVector3 Shape::GetUp() const
    {
        UpdateIfNeeded();
        return m_orientation.GetUp();
    }

    Zone::Zone(std::shared_ptr<Shape> inner, std::shared_ptr<Shape> outer)
        : m_innerShape(std::move(inner))
        , m_outerShape(std::move(outer))
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
        return amshared(BoxShape, definition->half_width(), definition->half_height(), definition->half_depth());
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
        , _uP1(0.0f)
        , _vP1(0.0f)
        , _wP1(0.0f)
        , _uP2(0.0f)
        , _vP3(0.0f)
        , _wP4(0.0f)
        , _corners()
    {
        Update();
    }

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
        , _uP1(0.0f)
        , _vP1(0.0f)
        , _wP1(0.0f)
        , _uP2(0.0f)
        , _vP3(0.0f)
        , _wP4(0.0f)
        , _corners()
    {
        SetLocation(position);
        Update();
    }

    AmReal32 BoxShape::GetHalfWidth() const
    {
        UpdateIfNeeded();
        return _halfWidth;
    }

    AmReal32 BoxShape::GetHalfHeight() const
    {
        UpdateIfNeeded();
        return _halfHeight;
    }

    AmReal32 BoxShape::GetHalfDepth() const
    {
        UpdateIfNeeded();
        return _halfDepth;
    }

    AmReal32 BoxShape::GetWidth() const
    {
        return GetHalfWidth() * 2.0f;
    }

    AmReal32 BoxShape::GetHeight() const
    {
        return GetHalfHeight() * 2.0f;
    }

    AmReal32 BoxShape::GetDepth() const
    {
        return GetHalfDepth() * 2.0f;
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

    AmReal32 BoxShape::GetShortestDistanceToEdge(const AmVector3& location) const
    {
        UpdateIfNeeded();

        // Point in box local space (box centered at origin, axes aligned with orientation)
        const AmVector3 pLocal = GetRelativeDirection(GetLocation(), GetOrientation().GetQuaternion(), location);

        const AmReal32 ax = std::abs(pLocal.x) - _halfWidth;
        const AmReal32 ay = std::abs(pLocal.y) - _halfDepth;
        const AmReal32 az = std::abs(pLocal.z) - _halfHeight;

        if ((ax <= 0.0f) && (ay <= 0.0f) && (az <= 0.0f))
        {
            const AmReal32 dx = _halfWidth - std::abs(pLocal.x);
            const AmReal32 dy = _halfDepth - std::abs(pLocal.y);
            const AmReal32 dz = _halfHeight - std::abs(pLocal.z);

            return std::min({ dx, dy, dz });
        }

        // Outside: negative of the largest overflow beyond any face
        const AmReal32 ox = std::max(ax, 0.0f);
        const AmReal32 oy = std::max(ay, 0.0f);
        const AmReal32 oz = std::max(az, 0.0f);

        return -std::max({ ox, oy, oz });
    }

    bool BoxShape::Contains(const AmVector3& location) const
    {
        UpdateIfNeeded();

        const AmReal32 ux = Dot(_u, location);
        const AmReal32 vx = Dot(_v, location);
        const AmReal32 wx = Dot(_w, location);

        return (AM_BETWEEN(ux, _uP1, _uP2) && AM_BETWEEN(vx, _vP1, _vP3) && AM_BETWEEN(wx, _wP1, _wP4));
    }

    AmVector3 BoxShape::GetClosestPoint(const AmVector3& location) const
    {
        UpdateIfNeeded();

        AmVector3 closestPoint;
        const AmVector3& relativeLocation = GetRelativeDirection(GetLocation(), GetOrientation().GetQuaternion(), location);
        closestPoint.x = std::clamp(relativeLocation.x, -_halfWidth, _halfWidth);
        closestPoint.y = std::clamp(relativeLocation.y, -_halfDepth, _halfDepth);
        closestPoint.z = std::clamp(relativeLocation.z, -_halfHeight, _halfHeight);

        // Transform back to world space
        return Add(GetLocation(), Transform(GetOrientation().GetRotationMatrix(), closestPoint));
    }

    std::span<const AmVector3> BoxShape::GetCorners() const
    {
        UpdateIfNeeded();
        return _corners;
    }

    bool BoxShape::operator==(const BoxShape& other) const
    {
        return _corners == other._corners;
    }

    bool BoxShape::operator!=(const BoxShape& other) const
    {
        return !(*this == other);
    }

    void BoxShape::Update()
    {
        const AmMatrix3 rotation = m_orientation.GetRotationMatrix();

        _corners[0] = Add(m_location, Transform(rotation, AmVector3{ -_halfWidth, -_halfDepth, -_halfHeight }));
        _corners[1] = Add(m_location, Transform(rotation, AmVector3{ -_halfWidth, _halfDepth, -_halfHeight }));
        _corners[2] = Add(m_location, Transform(rotation, AmVector3{ _halfWidth, -_halfDepth, -_halfHeight }));
        _corners[3] = Add(m_location, Transform(rotation, AmVector3{ -_halfWidth, -_halfDepth, _halfHeight }));
        _corners[4] = Add(m_location, Transform(rotation, AmVector3{ _halfWidth, _halfDepth, _halfHeight }));
        _corners[5] = Add(m_location, Transform(rotation, AmVector3{ -_halfWidth, _halfDepth, _halfHeight }));
        _corners[6] = Add(m_location, Transform(rotation, AmVector3{ _halfWidth, _halfDepth, -_halfHeight }));
        _corners[7] = Add(m_location, Transform(rotation, AmVector3{ _halfWidth, -_halfDepth, _halfHeight }));

        _p1 = _corners[0];
        _p2 = _corners[1];
        _p3 = _corners[2];
        _p4 = _corners[3];

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
        return amshared(CapsuleShape, definition->radius(), definition->half_height());
    }

    CapsuleShape::CapsuleShape(const AmReal32 radius, const AmReal32 halfHeight)
        : _radius(radius)
        , _halfHeight(halfHeight)
        , _a()
        , _b()
    {
        Update();
    }

    AmReal32 CapsuleShape::GetRadius() const
    {
        UpdateIfNeeded();
        return _radius;
    }

    AmReal32 CapsuleShape::GetHalfHeight() const
    {
        UpdateIfNeeded();
        return _halfHeight;
    }

    AmReal32 CapsuleShape::GetDiameter() const
    {
        return GetRadius() * 2.0f;
    }

    AmReal32 CapsuleShape::GetHeight() const
    {
        return GetHalfHeight() * 2.0f;
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

    AmReal32 CapsuleShape::GetShortestDistanceToEdge(const AmVector3& location) const
    {
        UpdateIfNeeded();

        const AmVector3 e = Sub(_b, _a);
        const AmVector3 pa = Sub(location, _a);

        const AmReal32 lE = Length(e);
        if (lE <= kEpsilon)
        {
            // Degenerate capsule -> sphere
            const AmReal32 d = Length(pa);
            return _radius - d;
        }

        const AmVector3 eHat = Scale(e, 1.0f / lE);

        const AmReal32 t = Dot(pa, eHat); // projection along the axis from _a

        if (t <= 0.0f)
        {
            // Closest to cap at _a
            return _radius - Length(pa);
        }

        if (t >= lE)
        {
            // Closest to cap at _b
            return _radius - Length(Sub(location, _b));
        }

        // Closest to cylinder part: radial distance to axis
        const AmReal32 distanceToAxis = Length(Cross(e, pa)) / lE;
        return _radius - distanceToAxis;
    }

    bool CapsuleShape::Contains(const AmVector3& location) const
    {
        UpdateIfNeeded();

        const AmVector3 e = Sub(_b, _a);
        const AmVector3 pa = Sub(location, _a);

        const AmReal32 lenE = Length(e);
        if (lenE == 0.0f)
        {
            // Degenerate capsule -> sphere
            return Length(pa) <= _radius;
        }

        const AmVector3 eHat = Scale(e, 1.0f / lenE);
        const AmReal32 t = Dot(pa, eHat);

        if (t <= 0.0f)
            return Length(pa) <= _radius;

        if (t >= lenE)
            return Length(Sub(location, _b)) <= _radius;

        const AmReal32 distanceToAxis = Length(Cross(e, pa)) / lenE;
        return distanceToAxis <= _radius;
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

        const AmMatrix3 rotation = m_orientation.GetRotationMatrix();

        _a = Add(m_location, Transform(rotation, AmVector3{ 0.0f, 0.0f, halfHeight }));
        _b = Add(m_location, Transform(rotation, AmVector3{ 0.0f, 0.0f, -halfHeight }));

        m_needUpdate = false;
    }

    std::shared_ptr<ConeShape> ConeShape::Create(const ConeShapeDefinition* definition)
    {
        return amshared(ConeShape, definition->radius(), definition->height());
    }

    ConeShape::ConeShape(const AmReal32 radius, const AmReal32 height)
        : _radius(radius)
        , _height(height)
    {}

    AmReal32 ConeShape::GetRadius() const
    {
        UpdateIfNeeded();
        return _radius;
    }

    AmReal32 ConeShape::GetDiameter() const
    {
        return GetRadius() * 2.0f;
    }

    AmReal32 ConeShape::GetHeight() const
    {
        UpdateIfNeeded();
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

    AmReal32 ConeShape::GetShortestDistanceToEdge(const AmVector3& location) const
    {
        UpdateIfNeeded();

        const AmVector3& shapeToLocation = Sub(location, m_location);
        const AmReal32 coneDist = Dot(shapeToLocation, m_orientation.GetForward());

        // Behind apex
        if (coneDist < 0.0f)
            return -Length(shapeToLocation); // Distance to apex point

        // Beyond base
        if (coneDist > _height)
        {
            // Distance to base plane
            const AmVector3 baseCenter = Add(m_location, Scale(m_orientation.GetForward(), _height));
            const AmVector3 toBase = Sub(location, baseCenter);
            const AmReal32 axialDist = Dot(toBase, m_orientation.GetForward());
            const AmReal32 radialDist = Length(Sub(toBase, Scale(m_orientation.GetForward(), axialDist)));

            if (radialDist <= _radius)
                return -axialDist; // Above base circle
            else
                return -std::sqrt(axialDist * axialDist + (radialDist - _radius) * (radialDist - _radius));
        }

        // Between apex and base
        const AmReal32 currentRadius = (coneDist / _height) * _radius;
        const AmReal32 radialDist = Length(Sub(shapeToLocation, Scale(m_orientation.GetForward(), coneDist)));

        if (radialDist <= currentRadius)
        {
            // Inside cone - find minimum distance to surface
            // Distance to base
            const AmReal32 distToBase = _height - coneDist;
            // Distance to slanted side (perpendicular to axis)
            const AmReal32 distToSide = (currentRadius - radialDist);

            return std::min(distToBase, distToSide);
        }
        else
        {
            // Outside cone - distance to slanted surface (perpendicular to axis)
            const AmReal32 distToSlant = (radialDist - currentRadius);

            return -distToSlant;
        }
    }

    bool ConeShape::Contains(const AmVector3& location) const
    {
        UpdateIfNeeded();

        const AmVector3& shapeToLocation = Sub(location, m_location);
        const AmReal32 coneDist = Dot(shapeToLocation, m_orientation.GetForward());

        if (coneDist < 0.0f || coneDist > _height)
            return false;

        const AmReal32 coneRadius = std::min((coneDist / _height) * _radius, _radius);
        const AmReal32 d = Length(Sub(shapeToLocation, Scale(m_orientation.GetForward(), coneDist)));

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
        return amshared(SphereShape, definition->radius());
    }

    SphereShape::SphereShape(const AmReal32 radius)
        : _radius(radius)
    {}

    AmReal32 SphereShape::GetRadius() const
    {
        UpdateIfNeeded();
        return _radius;
    }

    AmReal32 SphereShape::GetDiameter() const
    {
        return GetRadius() * 2.0f;
    }

    void SphereShape::SetRadius(const AmReal32 radius)
    {
        _radius = radius;
        m_needUpdate = true;
    }

    AmReal32 SphereShape::GetShortestDistanceToEdge(const AmVector3& location) const
    {
        UpdateIfNeeded();

        const AmVector3& shapeToLocation = Sub(location, m_location);
        const AmReal32 distance = Length(shapeToLocation);

        return _radius - distance;
    }

    bool SphereShape::Contains(const AmVector3& location) const
    {
        UpdateIfNeeded();

        const AmVector3& shapeToLocation = Sub(location, m_location);
        const AmReal32 distance = Length(shapeToLocation);

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
        : Zone(std::move(inner), std::move(outer))
    {}

    AmReal32 BoxZone::GetFactor(const AmVector3& position) const
    {
        if (m_innerShape->Contains(position))
            return 1.0f;

        if (!m_outerShape->Contains(position))
            return 0.0f;

        auto* inner = dynamic_cast<BoxShape*>(m_innerShape.get());
        auto* outer = dynamic_cast<BoxShape*>(m_outerShape.get());

        inner->UpdateIfNeeded();
        outer->UpdateIfNeeded();

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

        const AmReal32 depthDiff = std::max(outer->GetHalfDepth() - inner->GetHalfDepth(), kEpsilon);
        const AmReal32 widthDiff = std::max(outer->GetHalfWidth() - inner->GetHalfWidth(), kEpsilon);
        const AmReal32 heightDiff = std::max(outer->GetHalfHeight() - inner->GetHalfHeight(), kEpsilon);

        const AmReal32 dP1 = std::abs(Dot(Sub(x, outer->_p1), Normalize(Sub(outer->_p2, outer->_p1)))) / depthDiff;
        const AmReal32 dP2 = std::abs(Dot(Sub(x, outer->_p2), Normalize(Sub(outer->_p1, outer->_p2)))) / depthDiff;
        const AmReal32 dP3 = std::abs(Dot(Sub(x, outer->_p3), Normalize(Sub(outer->_p1, outer->_p3)))) / widthDiff;
        const AmReal32 dP4 = std::abs(Dot(Sub(x, outer->_p4), Normalize(Sub(outer->_p1, outer->_p4)))) / heightDiff;
        const AmReal32 dP5 = std::abs(Dot(Sub(x, outer->_p1), Normalize(Sub(outer->_p3, outer->_p1)))) / widthDiff;
        const AmReal32 dP6 = std::abs(Dot(Sub(x, outer->_p1), Normalize(Sub(outer->_p4, outer->_p1)))) / heightDiff;

        const AmReal32 t = std::min({ dP1, dP2, dP3, dP4, dP5, dP6 });
        return std::clamp(t, 0.0f, 1.0f);
    }

    CapsuleZone::CapsuleZone(std::shared_ptr<CapsuleShape> inner, std::shared_ptr<CapsuleShape> outer)
        : Zone(std::move(inner), std::move(outer))
    {}

    AmReal32 CapsuleZone::GetFactor(const AmVector3& position) const
    {
        if (m_innerShape->Contains(position))
            return 1.0f;

        if (!m_outerShape->Contains(position))
            return 0.0f;

        auto* inner = dynamic_cast<CapsuleShape*>(m_innerShape.get());
        auto* outer = dynamic_cast<CapsuleShape*>(m_outerShape.get());

        inner->UpdateIfNeeded();
        outer->UpdateIfNeeded();

        const AmVector3& x = position;

        // Inner axis and distances
        const AmVector3 iE = Sub(inner->_b, inner->_a);
        const AmReal32 iLenE = Length(iE);
        const AmVector3 iEH = (iLenE > 0.0f) ? Scale(iE, 1.0f / iLenE) : kVector3UnitZ;
        const AmVector3 iAX = Sub(x, inner->_a);
        const AmReal32 iT = Dot(iAX, iEH);
        const AmReal32 iRadial = (iLenE > 0.0f) ? (Length(Cross(iE, iAX)) / iLenE) : Length(iAX);

        // Outer axis and distances
        const AmVector3 oE = Sub(outer->_b, outer->_a);
        const AmReal32 oLenE = Length(oE);
        const AmVector3 oEH = (oLenE > 0.0f) ? Scale(oE, 1.0f / oLenE) : kVector3UnitZ;
        const AmVector3 oAX = Sub(x, outer->_a);
        const AmReal32 oT = Dot(oAX, oEH);
        const AmReal32 oRadial = (oLenE > 0.0f) ? (Length(Cross(oE, oAX)) / oLenE) : Length(oAX);

        // Quick contains checks with corrected math
        if (iT <= 0.0f)
        {
            if (Length(iAX) <= inner->GetRadius())
                return 1.0f;
        }
        else if (iT >= iLenE)
        {
            if (Length(Sub(x, inner->_b)) <= inner->GetRadius())
                return 1.0f;
        }
        else if (iRadial <= inner->GetRadius())
        {
            return 1.0f;
        }

        if (oT <= 0.0f)
        {
            if (Length(oAX) > outer->GetRadius())
                return 0.0f;
        }
        else if (oT >= oLenE)
        {
            if (Length(Sub(x, outer->_b)) > outer->GetRadius())
                return 0.0f;
        }
        else if (oRadial >= outer->GetRadius())
        {
            return 0.0f;
        }

        // Blend between inner and outer
        const AmReal32 rDelta = 1.0f - (oRadial - inner->GetRadius()) / std::max(outer->GetRadius() - inner->GetRadius(), kEpsilon);

        // Axial blending based on distance along axis (excluding spherical caps)
        const AmReal32 innerHalf = std::max(inner->GetHalfHeight() - inner->GetRadius(), 0.0f);
        const AmReal32 outerHalf = std::max(outer->GetHalfHeight() - outer->GetRadius(), innerHalf);
        const AmReal32 iAx = std::abs(Dot(Sub(x, inner->GetLocation()), iEH));
        const AmReal32 hDelta = 1.0f - (iAx - innerHalf) / std::max(outerHalf - innerHalf, kEpsilon);

        const AmReal32 t = std::min(rDelta, hDelta);
        return std::clamp(t, 0.0f, 1.0f);
    }

    ConeZone::ConeZone(std::shared_ptr<ConeShape> inner, std::shared_ptr<ConeShape> outer)
        : Zone(std::move(inner), std::move(outer))
    {}

    AmReal32 ConeZone::GetFactor(const AmVector3& position) const
    {
        if (m_innerShape->Contains(position))
            return 1.0f;

        if (!m_outerShape->Contains(position))
            return 0.0f;

        auto* inner = dynamic_cast<ConeShape*>(m_innerShape.get());
        auto* outer = dynamic_cast<ConeShape*>(m_outerShape.get());

        inner->UpdateIfNeeded();
        outer->UpdateIfNeeded();

        const AmReal32 sIn = inner->GetShortestDistanceToEdge(position);
        const AmReal32 sOut = outer->GetShortestDistanceToEdge(position);

        // Total width of the transition zone
        const AmReal32 transitionWidth = std::abs(sIn) + sOut;

        if (transitionWidth <= kEpsilon)
        {
            // Degenerate case: shapes are coincident
            // Return 0.5 to avoid discontinuity
            return 0.5f;
        }

        const AmReal32 t = sOut / transitionWidth;
        return std::clamp(t, 0.0f, 1.0f);
    }

    SphereZone::SphereZone(std::shared_ptr<SphereShape> inner, std::shared_ptr<SphereShape> outer)
        : Zone(std::move(inner), std::move(outer))
    {}

    AmReal32 SphereZone::GetFactor(const AmVector3& position) const
    {
        if (m_innerShape->Contains(position))
            return 1.0f;

        if (!m_outerShape->Contains(position))
            return 0.0f;

        auto* inner = dynamic_cast<SphereShape*>(m_innerShape.get());
        auto* outer = dynamic_cast<SphereShape*>(m_outerShape.get());

        inner->UpdateIfNeeded();
        outer->UpdateIfNeeded();

        const AmVector3& soundToListener = Sub(position, inner->GetLocation());
        const AmReal32 distance = Length(soundToListener);
        if (distance >= outer->GetRadius())
            return 0.0f;

        const AmReal32 t = (distance - inner->GetRadius()) / (outer->GetRadius() - inner->GetRadius());
        return 1.0f - std::clamp(t, 0.0f, 1.0f);
    }
} // namespace SparkyStudios::Audio::Amplitude
