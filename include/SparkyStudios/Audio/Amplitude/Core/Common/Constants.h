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

#ifndef SS_AMPLITUDE_AUDIO_CONSTANTS_H
#define SS_AMPLITUDE_AUDIO_CONSTANTS_H

#include <SparkyStudios/Audio/Amplitude/Core/Common/Types.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Invalid Amplitude object ID.
     */
    constexpr AmObjectID kAmInvalidObjectId = 0;

    /**
     * @brief Specifies the value of the "master" bus ID.
     */
    constexpr AmBusID kAmMasterBusId = 1;

    /**
     * @brief The number of milliseconds in one second.
     */
    constexpr AmTime kAmSecond = 1000.0;

    /**
     * @brief Minimum value where values lower than this are considered to be 0.
     */
    constexpr AmReal32 kEpsilon = 0.0001f;

    /**
     * @brief The minimum fade duration in milliseconds.
     */
    constexpr AmTime kMinFadeDuration = 10.0;

    /**
     * @brief The number of bits to shift when processing audio data with floating point values.
     */
    constexpr AmInt32 kAmFixedPointBits = 15;

    constexpr AmInt32 kAmFixedPointUnit = (1 << kAmFixedPointBits);

    constexpr AmInt32 kAmFixedPointMask = (kAmFixedPointBits - 1);
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_CONSTANTS_H
