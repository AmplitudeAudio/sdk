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

#include <SparkyStudios/Audio/Amplitude/Math/Curve.h>

#include "common_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    bool CurvePoint::operator==(const CurvePoint& rhs) const
    {
        return x == rhs.x && y == rhs.y;
    }

    bool CurvePoint::operator!=(const CurvePoint& rhs) const
    {
        return !(rhs == *this);
    }

    void CurvePart::Initialize(const CurvePartDefinition* definition)
    {
        if (definition == nullptr)
            return;

        _start = { definition->start().x(), definition->start().y() };
        _end = { definition->end().x(), definition->end().y() };
        _fader = Fader::Create(static_cast<Fader::FADER_ALGORITHM>(definition->fader()));

        _fader->Set(_start.y, _end.y, 0.0);
    }

    const CurvePoint& CurvePart::GetStart() const
    {
        return _start;
    }

    CurvePart::CurvePart()
        : _start()
        , _end()
        , _fader(nullptr)
    {}

    void CurvePart::SetStart(const CurvePoint& start)
    {
        _start = start;

        if (_fader != nullptr)
            _fader->Set(_start.y, _end.y, 0.0);
    }

    const CurvePoint& CurvePart::GetEnd() const
    {
        return _end;
    }

    void CurvePart::SetEnd(const CurvePoint& end)
    {
        _end = end;

        if (_fader != nullptr)
            _fader->Set(_start.y, _end.y, 0.0);
    }

    Fader* CurvePart::GetFader() const
    {
        return _fader;
    }

    void CurvePart::SetFader(Fader::FADER_ALGORITHM fader)
    {
        _fader = Fader::Create(fader);
        _fader->Set(_start.y, _end.y, 0.0);
    }

    float CurvePart::Get(double x) const
    {
        double percentage = (x - _start.x) / (_end.x - _start.x);
        return _fader->GetFromPercentage(percentage);
    }

    Curve::Curve()
        : _parts()
    {}

    void Curve::Initialize(const CurveDefinition* definition)
    {
        const flatbuffers::uoffset_t size = definition->parts() ? definition->parts()->size() : 0;
        _parts.resize(size);
        for (flatbuffers::uoffset_t i = 0; i < size; ++i)
        {
            _parts[i].Initialize(definition->parts()->Get(i));
        }
    }

    float Curve::Get(double x) const
    {
        // Clamp the x value to the possible curve range.
        x = AM_CLAMP(x, _parts.front().GetStart().x, _parts.back().GetEnd().x);
        const CurvePart* part = _findCurvePart(x);

        // If no curve part found
        if (part == nullptr)
            return 0.0f;

        return part->Get(x);
    }

    const CurvePart* Curve::_findCurvePart(double x) const
    {
        for (const auto& item : _parts)
        {
            if (item.GetStart().x <= x && item.GetEnd().x >= x)
                return &item;
        }

        return nullptr;
    }
}; // namespace SparkyStudios::Audio::Amplitude
