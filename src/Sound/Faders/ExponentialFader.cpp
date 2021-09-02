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

#include <Sound/Faders/ExponentialFader.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Offset used to avoid NaN results.
     */
    const float kOffset = 0.5f;

    float ExponentialFader::GetFromPercentage(double percentage)
    {
        percentage = AM_CLAMP(percentage, 0.0, 1.0);

        float from = m_from + kOffset;
        float to = m_to + kOffset;
        float factor = percentage == 0.0 ? 1 : AM_PowerF(to / from, (float)percentage);

        float lower = AM_MIN(m_from, m_to);
        float greater = AM_MAX(m_from, m_to);

        return AM_CLAMP(from * factor - kOffset, lower, greater);
    }
} // namespace SparkyStudios::Audio::Amplitude
