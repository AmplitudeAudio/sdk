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

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct PolarPosition
    {
        PolarPosition() = default;
        PolarPosition(AmReal32 azimuth, AmReal32 elevation, AmReal32 radius);
        PolarPosition(AmVec3 originPosition, AmVec3 originDirection, AmVec3 originUp, AmVec3 position);

        [[nodiscard]] AmVec3 ToCartesian(GameEngineUpAxis upAxis) const;

        AmReal32 m_Azimuth = 0;
        AmReal32 m_Elevation = 0;
        AmReal32 m_Radius = 0;
    };
} // namespace SparkyStudios::Audio::Amplitude
