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
     *
     * A face is defined by three vertices. This structure is optimized for use
     * with an existing indexed vertex array, so only the indices of each face's
     * vertex need to be provided.
     *
     * @ingroup math
     */
    struct AM_API_PUBLIC Face
    {
        AmSize m_A; ///< Index of the first vertex.
        AmSize m_B; ///< Index of the second vertex.
        AmSize m_C; ///< Index of the third vertex.

        /**
         * @brief Checks if the face is valid.
         *
         * @return `true` if the face is valid, `false` otherwise.
         */
        [[nodiscard]] bool IsValid() const;
    };

    /**
     * @brief Represents an edge.
     *
     * An edge is defined by two vertices. This structure is optimized for use
     * with an existing indexed vertex array, so only the indices of each edge's
     * vertex need to be provided.
     *
     * @ingroup math
     */
    struct AM_API_PUBLIC Edge
    {
        AmSize m_E0; ///< Index of the first vertex.
        AmSize m_E1; ///< Index of the second vertex.

        bool operator<(const Edge& other) const;
        bool operator==(const Edge& other) const;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_MATH_GEOMETRY_H
