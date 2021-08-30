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

#include <Sound/Faders/LinearFader.h>

namespace SparkyStudios::Audio::Amplitude
{
    float LinearFader::GetFromPercentage(double percentage)
    {
        return AM_Lerp(m_from, AM_CLAMP(percentage, 0.0, 1.0), m_to);
    }
} // namespace SparkyStudios::Audio::Amplitude