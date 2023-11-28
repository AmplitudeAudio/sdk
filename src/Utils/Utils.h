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

#pragma once

#ifndef SS_AMPLITUDE_AUDIO_UTILS_H
#define SS_AMPLITUDE_AUDIO_UTILS_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Math/Utils.h>

#if defined(AM_SIMD_INTRINSICS)
#include <xsimd/xsimd.hpp>
#endif // defined(AM_SIMD_INTRINSICS)

namespace SparkyStudios::Audio::Amplitude
{
#if defined(AM_SIMD_INTRINSICS)
    typedef xsimd::batch<AmReal32, xsimd::best_arch> AmAudioFrame;
#else
    typedef AmReal32 AmAudioFrame;
#endif // AM_SIMD_INTRINSICS

    typedef AmAudioFrame* AmAudioFrameBuffer;
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_UTILS_H
