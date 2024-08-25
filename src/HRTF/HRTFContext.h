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

#ifndef _AM_IMPLEMENTATION_HRTF_HRTF_CONTEXT_H
#define _AM_IMPLEMENTATION_HRTF_HRTF_CONTEXT_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct HRTFContext
    {
        AmVec3 m_CurrentDirection = { 0.0f, 0.0f, 0.0f };
        AmVec3 m_PreviousDirection = { 0.0f, 0.0f, 0.0f };

        AmAlignedReal32Buffer m_PreviousSamplesL;
        AmAlignedReal32Buffer m_PreviousSamplesR;

        AmReal32 m_PreviousGain;
        AmReal32 m_CurrentGain;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_HRTF_HRTF_CONTEXT_H