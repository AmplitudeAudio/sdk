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
    AmReal64 LinearFaderInstance::GetFromPercentage(AmReal64 percentage)
    {
        percentage = AM_CLAMP(percentage, 0.0, 1.0);
        return (1.0 - percentage) * m_from + percentage * m_to;
    }
} // namespace SparkyStudios::Audio::Amplitude
