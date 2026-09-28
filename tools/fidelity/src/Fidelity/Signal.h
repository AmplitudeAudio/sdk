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

#ifndef _AM_FIDELITY_SIGNAL_H
#define _AM_FIDELITY_SIGNAL_H

#include <cstddef>
#include <cstdint>
#include <random>
#include <span>
#include <vector>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    /**
     * @brief A mono signal in double precision.
     */
    using Signal = std::vector<double>;

    /**
     * @brief Smallest amplitude used in dB conversions (-400 dB), so every reported level stays finite.
     */
    constexpr double kMinAmplitude = 1e-20;

    [[nodiscard]] double DbFromAmplitude(double amplitude);
    [[nodiscard]] double DbFromPower(double power);
    [[nodiscard]] double AmplitudeFromDb(double db);

    /**
     * @brief Interval from @p reference to @p frequency in cents.
     */
    [[nodiscard]] double CentsBetween(double frequency, double reference);

    [[nodiscard]] double Rms(std::span<const double> x);
    [[nodiscard]] double Peak(std::span<const double> x);
    [[nodiscard]] Signal ToSignal(std::span<const float> x);
    [[nodiscard]] std::vector<float> ToFloats(std::span<const double> x);

    /**
     * @brief amplitude * sin(2 pi frequency n / sampleRate + phase), with the angle reduced per sample for accuracy.
     */
    [[nodiscard]] Signal MakeSine(std::size_t length, double sampleRate, double frequency, double amplitude, double phase = 0.0);

    /**
     * @brief Gain of a smooth fade-in over the first @p fadeLength samples and fade-out over the last ones (a raised cosine
     * of a raised cosine, continuous up to the third derivative).
     *
     * The gain is 0 at both ends and 0.5 at fadeLength / 2 from either end.
     */
    [[nodiscard]] double FadeGain(std::size_t n, std::size_t total, std::size_t fadeLength);

    /**
     * @brief Multiplies @p x by FadeGain().
     */
    void ApplyFades(Signal& x, std::size_t fadeLength);

    /**
     * @brief Centred moving average; the window shrinks near the edges.
     */
    [[nodiscard]] Signal MovingAverage(std::span<const double> x, std::size_t window);

    /**
     * @brief Seeded generator whose output is identical on every standard library.
     */
    class Random
    {
    public:
        explicit Random(std::uint64_t seed);

        /**
         * @brief Uniform in [0, 1).
         */
        double Uniform();

        /**
         * @brief Uniform in [-1, 1).
         */
        double Symmetric();

    private:
        std::mt19937_64 _engine;
    };
} // namespace SparkyStudios::Audio::Amplitude::Fidelity

#endif // _AM_FIDELITY_SIGNAL_H
