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

#include <SparkyStudios/Audio/Amplitude/Math/Geometry.h>

namespace SparkyStudios::Audio::Amplitude
{
    bool Face::IsValid() const
    {
        return m_A != m_B && m_B != m_C && m_C != m_A;
    }

    bool Edge::operator<(const Edge& other) const
    {
        return m_E0 < other.m_E0 || (m_E0 == other.m_E0 && m_E1 < other.m_E1);
    }

    bool Edge::operator==(const Edge& other) const
    {
        return m_E0 == other.m_E0 && m_E1 == other.m_E1;
    }
}