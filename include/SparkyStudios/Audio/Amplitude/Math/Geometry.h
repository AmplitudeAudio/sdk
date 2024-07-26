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

#pragma once

#ifndef _AM_MATH_GEOMETRY_H
#define _AM_MATH_GEOMETRY_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Represents a triangulated face.
     */
    struct AM_API_PUBLIC Face
    {
        AmSize m_A;
        AmSize m_B;
        AmSize m_C;

        [[nodiscard]] bool IsValid() const;
    };

    /**
     * @brief Represents a triangulated edge.
     */
    struct AM_API_PUBLIC Edge
    {
        AmSize m_E0;
        AmSize m_E1;

        bool operator<(const Edge& other) const;
        bool operator==(const Edge& other) const;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_MATH_GEOMETRY_H
