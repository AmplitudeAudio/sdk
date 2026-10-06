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

#ifndef _AM_FIDELITY_ANALYSIS_SIDEBAND_H
#define _AM_FIDELITY_ANALYSIS_SIDEBAND_H

#include <cstddef>
#include <span>

#include <Fidelity/Signal.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    /**
     * @brief Options for AnalyzeGlideSidebands().
     */
    struct GlideSidebandOptions
    {
        /// Analysed region [begin, end) of the render; end 0 means the whole signal.
        std::size_t begin = 0;
        std::size_t end = 0;
        double sampleRate = 48000.0;
        /// The modulation the render is checked for: the audio block rate, sampleRate / blockSize.
        double blockRateHz = 46.875;
        /// How many multiples of blockRateHz the sideband total sums over.
        std::size_t orders = 3;
        /// The carrier's own main lobe, this many block rates wide, is excluded from the sideband total.
        double innerBlockRates = 0.25;
        /// Samples the analysis covers; falls back to the whole region when it is longer.
        std::size_t frames = 65536;
        /// The tracked pitch is slid against the render over [-fitSpanSeconds, fitSpanSeconds], in this many
        /// steps, and the slide that leaves the most carrier behind is the one measured.
        double fitSpanSeconds = 0.2;
        std::size_t fitSteps = 256;
    };

    /**
     * @brief Result of AnalyzeGlideSidebands().
     */
    struct GlideSidebandResult
    {
        /// Energy within +/- orders * blockRateHz of the carrier, outside its main lobe, against the carrier's own
        /// energy, in dBc. -400 when nothing could be measured.
        double sidebandDbc = -400.0;
        /// The demodulated carrier against a full-scale sine, in dBFS.
        double carrierDbfs = -400.0;
        /// The slide that aligned the tracked pitch with the render, in samples. Positive means the render lags.
        double fitOffsetSamples = 0.0;
    };

    /**
     * @brief Energy at the block-rate sidebands of a gliding tone.
     *
     * The tone is demodulated by @p trackedHz, its expected instantaneous frequency, so what is left is the phase
     * error the render actually commits. A spectrum taken straight off a glide would smear the carrier over hundreds
     * of hertz -- the pitch moves faster than a window can resolve -- which is why the sweep is unwrapped first and
     * the block rate measured in the baseband that comes out.
     *
     * @p trackedHz is the render's tracked pitch, not the command: the engine eases toward the command and lags it,
     * which would walk the demodulator through several cycles of phase. The tracker's millisecond of smoothing
     * leaves the block rate untouched. The constant slide that best aligns the two is searched first.
     *
     * @param[in] x          Rendered samples.
     * @param[in] trackedHz  Expected instantaneous frequency per sample, indexed like @p x.
     * @param[in] options    Analysis options.
     *
     * @return The sideband level against the carrier, the carrier itself, and the slide it was measured at.
     */
    [[nodiscard]] GlideSidebandResult AnalyzeGlideSidebands(
        std::span<const double> x, const Signal& trackedHz, const GlideSidebandOptions& options);
} // namespace SparkyStudios::Audio::Amplitude::Fidelity

#endif // _AM_FIDELITY_ANALYSIS_SIDEBAND_H