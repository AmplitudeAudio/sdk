// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#ifndef _AM_IMPLEMENTATION_AMBISONICS_BINAURAL_DECODING_FILTERS_H
#define _AM_IMPLEMENTATION_AMBISONICS_BINAURAL_DECODING_FILTERS_H

#include <memory>
#include <span>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/HRTF/HRIRSphere.h>

#include <SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h>

#include <DSP/ConvolverKernel.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Immutable binaural decoding filters for one HRIR sphere, ambisonic order and 2D/3D setting.
     *
     * Built once (the engine does it at init) and shared, read-only, by every binauralizer of that order.
     */
    class BinauralDecodingFilters
    {
    public:
        /**
         * @brief Builds the decoding filters: one left and one right kernel per ambisonic channel.
         *
         * @param order The ambisonic order.
         * @param is3D Whether the sound field is 3D.
         * @param sphere The HRIR sphere to sample.
         *
         * @return The filters, or nullptr when the order is invalid or has more than 16 channels, or the sphere has no
         * impulse response.
         */
        static std::shared_ptr<const BinauralDecodingFilters> Build(AmUInt32 order, bool is3D, const HRIRSphere& sphere);

        BinauralDecodingFilters() = default;

        [[nodiscard]] AmUInt32 GetOrder() const;
        [[nodiscard]] bool Is3D() const;
        [[nodiscard]] AmUInt32 GetChannelCount() const;

        /**
         * @brief Gets the kernel of one ear and channel.
         *
         * @param ear 0 for the left ear, 1 for the right.
         * @param channel The ambisonic channel.
         */
        [[nodiscard]] const std::shared_ptr<const ConvolverKernel>& GetKernel(AmUInt32 ear, AmUInt32 channel) const;

        /**
         * @brief Gets the kernels of one ear, in channel order.
         *
         * @param ear 0 for the left ear, 1 for the right.
         */
        [[nodiscard]] std::span<const std::shared_ptr<const ConvolverKernel>> GetKernels(AmUInt32 ear) const;

        /**
         * @brief Gets the length, in samples, of the accumulated HRIRs.
         */
        [[nodiscard]] AmUInt32 GetHRIRLength() const;

        /**
         * @brief Gets the accumulated and normalized time-domain HRIR of one ear and channel.
         *
         * @param ear 0 for the left ear, 1 for the right.
         * @param channel The ambisonic channel.
         *
         * @return A pointer to GetHRIRLength() samples.
         */
        [[nodiscard]] const AmReal32* GetHRIR(AmUInt32 ear, AmUInt32 channel) const;

    private:
        AmUInt32 _hrirLength = 0;
        AudioBuffer _hrirs[2];
        AmUInt32 _order = 0;
        bool _is3D = true;
        AmUInt32 _channelCount = 0;
        std::vector<std::shared_ptr<const ConvolverKernel>> _kernels[2];
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_AMBISONICS_BINAURAL_DECODING_FILTERS_H
