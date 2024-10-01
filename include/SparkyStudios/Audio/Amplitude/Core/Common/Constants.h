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

#ifndef _AM_CORE_COMMON_CONSTANTS_H
#define _AM_CORE_COMMON_CONSTANTS_H

#include <SparkyStudios/Audio/Amplitude/Core/Common/Types.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Invalid Amplitude object ID.
     *
     * @ingroup core
     */
    constexpr AmObjectID kAmInvalidObjectId = 0;

    /**
     * @brief Specifies the value of the "master" bus ID.
     *
     * @ingroup core
     */
    constexpr AmBusID kAmMasterBusId = 1;

    /**
     * @brief The number of milliseconds in one second.
     *
     * @ingroup core
     */
    constexpr AmTime kAmSecond = 1000.0;

    /**
     * @brief Minimum value where values lower than this are considered to be 0.
     *
     * @ingroup core
     */
    constexpr AmReal32 kEpsilon = 1e-6f;

    /**
     * @brief The minimum fade duration in milliseconds.
     *
     * @ingroup core
     */
    constexpr AmTime kMinFadeDuration = 10.0;

    /**
     * @brief The number of bits to shift when processing audio data with floating point values.
     *
     * @ingroup core
     */
    constexpr AmInt32 kAmFixedPointBits = 15;

    /**
     * @brief The unit value for a 32-bit fixed-point audio sample..
     *
     * @ingroup core
     */
    constexpr AmInt32 kAmFixedPointUnit = (1 << kAmFixedPointBits);

    /**
     * @brief Used to mask the bits when processing audio data with fixed-point values.
     *
     * @ingroup core
     */
    constexpr AmInt32 kAmFixedPointMask = (kAmFixedPointBits - 1);

    /**
     * @brief The maximum number of frames that can be processed at once.
     *
     * @ingroup core
     */
    constexpr AmUInt64 kAmMaxSupportedFrameCount = 16384;

    /**
     * @brief The maximum supported ambisonic order.
     *
     * @ingroup core
     */
    constexpr AmUInt32 kAmMaxSupportedAmbisonicOrder = 3;

    /**
     * @brief The number of surfaces in a room.
     *
     * @warning Only cube-shaped rooms are supported.
     *
     * @ingroup core
     */
    constexpr AmSize kAmRoomSurfaceCount = 6;

    /**
     * @brief The number of channels in a mono audio source.
     *
     * @ingroup core
     */
    constexpr AmSize kAmMonoChannelCount = 1;

    /**
     * @brief The number of channels in a stereo audio source.
     *
     * @ingroup core
     */
    constexpr AmSize kAmStereoChannelCount = 2;

    /**
     * @brief The number of channels in a 5.1 surround audio source.
     *
     * @ingroup core
     */
    constexpr AmSize kAm51SurroundChannelCount = 6;

    /**
     * @brief The number of channels in a 7.1 surround audio source.
     *
     * @ingroup core
     */
    constexpr AmSize kAm71SurroundChannelCount = 8;

    /**
     * @brief The number of channels in first-order ambisonic source.
     *
     * @ingroup core
     */
    constexpr AmSize kAmFirstOrderAmbisonicChannelCount = 4;

    /**
     * @brief The number of channels in second-order ambisonic source.
     *
     * @ingroup core
     */
    constexpr AmSize kAmSecondOrderAmbisonicChannelCount = 9;

    /**
     * @brief The number of channels in third-order ambisonic source.
     *
     * @ingroup core
     */
    constexpr AmSize kAmThirdOrderAmbisonicChannelCount = 16;

    /**
     * @brief The maximum supported channel count for an ambisonic source.
     *
     * @ingroup core
     */
    constexpr AmUInt32 kAmMaxSupportedChannelCount = (kAmMaxSupportedAmbisonicOrder + 1) * (kAmMaxSupportedAmbisonicOrder + 1);

    /**
     * @brief The number of air absorption bands for attenuation models.
     *
     * @ingroup core
     */
    constexpr AmUInt32 kAmAirAbsorptionBandCount = 3;

    /**
     * @brief The low cutoff frequencies for air absorption models.
     *
     * @ingroup core
     */
    constexpr AmReal32 kLowCutoffFrequencies[kAmAirAbsorptionBandCount] = { 0.0f, 800.0f, 8000.0f };

    /**
     * @brief The high cutoff frequencies for air absorption models.
     *
     * @ingroup core
     */
    constexpr AmReal32 kHighCutoffFrequencies[kAmAirAbsorptionBandCount] = { 800.0f, 8000.0f, 22000.0f };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_COMMON_CONSTANTS_H
