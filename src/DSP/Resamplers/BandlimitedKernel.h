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

#ifndef _AM_IMPLEMENTATION_DSP_RESAMPLERS_BANDLIMITED_KERNEL_H
#define _AM_IMPLEMENTATION_DSP_RESAMPLERS_BANDLIMITED_KERNEL_H

#include <vector>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Shape of a band-limited interpolation kernel. Edges are in cycles per input frame (Nyquist is 0.5).
     */
    struct BandlimitedKernelSpec
    {
        AmUInt32 zeroCrossings = 0; ///< Zero crossings on each side of the centre.
        AmUInt32 phases = 0; ///< Table entries per zero crossing.
        AmReal64 beta = 0.0; ///< Kaiser window shape.
        AmReal64 passbandEdge = 0.0;
        AmReal64 stopbandEdge = 0.0;
        AmReal64 stopbandDb = 0.0; ///< Attenuation the table must reach past @c stopbandEdge.
    };

    /// 32 taps, about -80 dB: the balanced preset.
    constexpr BandlimitedKernelSpec kSincKernel{ 16, 256, 7.9, 0.4195, 0.5805, -80.0 };

    /// 80 taps, at least -110 dB, flat to 0.4535 of the lower rate (20 kHz at 44.1 kHz): the default preset.
    constexpr BandlimitedKernelSpec kSincBestKernel{ 40, 1024, 11.2, 0.4535, 0.5465, -110.0 };

    /**
     * @brief Zeroth-order modified Bessel function of the first kind, for the Kaiser window.
     */
    [[nodiscard]] AmReal64 BesselI0(AmReal64 x);

    /**
     * @brief Immutable table of one half of a Kaiser-windowed sinc with its cutoff at Nyquist.
     *
     * Entry i holds the kernel at i / phases input frames from the centre. Entries at whole frames other than the centre
     * are exactly 0, so a kernel read at a whole-frame phase passes the input through unchanged. Built once on the game
     * thread and shared read-only by every instance of a preset.
     */
    class BandlimitedKernel
    {
    public:
        explicit BandlimitedKernel(const BandlimitedKernelSpec& spec);

        [[nodiscard]] AmUInt32 GetZeroCrossings() const
        {
            return _spec.zeroCrossings;
        }

        [[nodiscard]] AmUInt32 GetPhases() const
        {
            return _spec.phases;
        }

        [[nodiscard]] const BandlimitedKernelSpec& GetSpec() const
        {
            return _spec;
        }

        /**
         * @brief Evaluates the kernel at @p distance input frames from the centre, between table entries.
         *
         * @return 0 at or beyond @c GetZeroCrossings() frames.
         */
        [[nodiscard]] AmReal32 Evaluate(AmReal64 distance) const;

        /**
         * @brief The table: @c GetTableSize() entries plus two trailing zeros, so an interpolation never reads past it.
         */
        [[nodiscard]] const AmReal32* GetTable() const
        {
            return _table.data();
        }

        /**
         * @brief Gets the number of meaningful entries, zeroCrossings * phases.
         */
        [[nodiscard]] AmUInt64 GetTableSize() const
        {
            return static_cast<AmUInt64>(_spec.zeroCrossings) * _spec.phases;
        }

    private:
        BandlimitedKernelSpec _spec;
        std::vector<AmReal32> _table;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_DSP_RESAMPLERS_BANDLIMITED_KERNEL_H
