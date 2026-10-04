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

#ifndef _AM_FIDELITY_RENDER_SESSION_H
#define _AM_FIDELITY_RENDER_SESSION_H

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

#include <Fidelity/Signal.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    /**
     * @brief Fidelity assets relative to the build directory (where amplitude_tests and amplitude_fidelity run).
     */
    constexpr const char* kDefaultAssetsPath = "fidelity/assets";

    enum class MarkerKind
    {
        Action,
        Frame,
        Block,
    };

    /**
     * @brief A timeline event recorded during a render, at an output sample index.
     */
    struct Marker
    {
        std::uint64_t sample = 0;
        MarkerKind kind = MarkerKind::Frame;
        std::string label;
    };

    /**
     * @brief Game-side action, run on the first game frame at or after @c sample.
     */
    struct TimedAction
    {
        std::uint64_t sample = 0;
        std::string label;
        std::function<void()> run;
    };

    /**
     * @brief Parameters of one offline render.
     */
    struct RenderSettings
    {
        /// Compiled config name, e.g. "fidelity.isolated.b1024.config.amconfig".
        std::string configFile;
        /// Frames per Mix call; must equal the config's buffer_size / 2.
        std::uint32_t blockSize = 1024;
        /// When not empty, Mix is called with these frame counts in turn instead of blockSize.
        std::vector<std::uint32_t> blockSequence;
        double fps = 60.0;
        bool jitter = false;
        double jitterAmount = 0.25;
        std::uint64_t jitterSeed = 1;
        std::uint64_t durationSamples = 0;
    };

    /**
     * @brief The rendered output, one vector per channel, with its timeline markers (in time order).
     */
    struct Capture
    {
        std::uint32_t sampleRate = 0;
        std::vector<std::vector<float>> channels;
        std::vector<Marker> markers;
    };

    /**
     * @brief Result of Render(); @c error is empty on success.
     */
    struct RenderOutcome
    {
        Capture capture;
        std::string error;
    };

    /**
     * @brief One step of the lock-step loop.
     */
    struct ScheduleEvent
    {
        MarkerKind kind = MarkerKind::Frame;
        double sample = 0.0;
        /// Mix frame count (blocks only).
        std::uint32_t frames = 0;
        /// Time since the previous frame in milliseconds (frames only).
        double deltaMs = 0.0;
    };

    /**
     * @brief Interleaves game frames and audio blocks in output-sample time. A frame wins a tie.
     */
    class LockStepSchedule
    {
    public:
        LockStepSchedule(const RenderSettings& settings, std::uint32_t sampleRate);

        /**
         * @brief Returns the next event and advances the schedule.
         */
        ScheduleEvent Next();

    private:
        double NextPeriod();

        double _sampleRate;
        std::uint32_t _blockSize;
        std::vector<std::uint32_t> _sequence;
        std::size_t _sequenceIndex = 0;
        double _period;
        bool _jitter;
        double _jitterAmount;
        Random _random;
        bool _hasPending = false;
        double _pending = 0.0;
        double _nextFrame = 0.0;
        double _lastFrame;
        std::uint64_t _nextBlock = 0;
    };

    /**
     * @brief Brings up a fresh engine on the offline driver with @p settings.configFile, runs @p actions on the
     * lock-step loop until @p settings.durationSamples output samples are captured, and tears the engine down.
     */
    [[nodiscard]] RenderOutcome Render(
        const std::filesystem::path& assets, const RenderSettings& settings, std::vector<TimedAction> actions);
} // namespace SparkyStudios::Audio::Amplitude::Fidelity

#endif // _AM_FIDELITY_RENDER_SESSION_H
