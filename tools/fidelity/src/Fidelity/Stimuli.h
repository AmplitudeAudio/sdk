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

#ifndef _AM_FIDELITY_STIMULI_H
#define _AM_FIDELITY_STIMULI_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <Fidelity/Analysis/FrequencyResponse.h>
#include <Fidelity/Analysis/Pitch.h>
#include <Fidelity/Signal.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    enum class StimulusKind
    {
        Sine,
        LoopSine,
        LogSweep,
        LinearChirp,
        PinkNoise,
    };

    /**
     * @brief A test signal. The catalog is the single source of truth for the WAV files and the analyzers' models.
     */
    struct StimulusSpec
    {
        std::string name;
        StimulusKind kind = StimulusKind::Sine;
        std::uint32_t sampleRate = 48000;
        std::uint16_t channels = 1;
        double durationSeconds = 2.0;
        /// Peak amplitude, or RMS for pink noise.
        double amplitude = 0.5;
        /// Tone frequency, or the start frequency of sweeps and chirps.
        double frequencyHz = 997.0;
        /// End frequency of sweeps and chirps.
        double frequencyEndHz = 0.0;
        /// Raised-cosine fade at both ends.
        double fadeSeconds = 0.0;
        std::uint64_t seed = 0;
        /// Static pitch the sound plays at; the heard frequency is frequencyHz * pitch.
        double pitch = 1.0;
    };

    [[nodiscard]] const std::vector<StimulusSpec>& StimulusCatalog();
    [[nodiscard]] const StimulusSpec* FindStimulus(std::string_view name);
    [[nodiscard]] std::size_t StimulusFrameCount(const StimulusSpec& spec);
    [[nodiscard]] std::size_t FadeFrames(const StimulusSpec& spec);

    /**
     * @brief One channel of the stimulus; every channel of a multi-channel stimulus carries the same signal.
     */
    [[nodiscard]] Signal RenderStimulusChannel(const StimulusSpec& spec);
    [[nodiscard]] std::vector<float> RenderStimulusInterleaved(const StimulusSpec& spec);

    [[nodiscard]] SweepModel SweepModelOf(const StimulusSpec& spec);
    [[nodiscard]] ChirpModel ChirpModelOf(const StimulusSpec& spec);
} // namespace SparkyStudios::Audio::Amplitude::Fidelity

#endif // _AM_FIDELITY_STIMULI_H
