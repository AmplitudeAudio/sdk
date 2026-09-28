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

#ifndef _AM_FIDELITY_ANALYSIS_WINDOW_H
#define _AM_FIDELITY_ANALYSIS_WINDOW_H

#include <cstddef>
#include <vector>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    /**
     * @brief Periodic 7-term Blackman-Harris window (highest sidelobe about -180 dB, main lobe ±7 bins).
     */
    [[nodiscard]] std::vector<double> BlackmanHarris7(std::size_t n);

    /**
     * @brief Mean of the window samples.
     */
    [[nodiscard]] double CoherentGain(const std::vector<double>& window);

    /**
     * @brief Equivalent noise bandwidth in bins: n * sum(w^2) / sum(w)^2.
     */
    [[nodiscard]] double EquivalentNoiseBandwidth(const std::vector<double>& window);
} // namespace SparkyStudios::Audio::Amplitude::Fidelity

#endif // _AM_FIDELITY_ANALYSIS_WINDOW_H
