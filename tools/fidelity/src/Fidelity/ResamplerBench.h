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

#ifndef _AM_FIDELITY_RESAMPLER_BENCH_H
#define _AM_FIDELITY_RESAMPLER_BENCH_H

#include <string>
#include <vector>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    /**
     * @brief One measured preset/ratio pair.
     */
    struct ResamplerBenchRow
    {
        std::string preset;
        double ratio = 1.0;
        double realtimeVoices = 0.0;
    };

    /**
     * @brief Times each built-in preset at ratios 1, 44.1/48, 2 and 4 on @p seconds of mono audio.
     *
     * Each cell renders @p seconds of output in 512-frame blocks from a fixed input buffer, after a warm-up pass, so the
     * number is the steady state rather than table building. A ratio is input frames per output frame.
     *
     * @return One row per preset and ratio; realtimeVoices is how many such voices one core renders in real time.
     */
    std::vector<ResamplerBenchRow> RunResamplerBench(double seconds);
} // namespace SparkyStudios::Audio::Amplitude::Fidelity

#endif // _AM_FIDELITY_RESAMPLER_BENCH_H