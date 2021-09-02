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

#ifndef SS_AMPLITUDE_AUDIO_CURVE_H
#define SS_AMPLITUDE_AUDIO_CURVE_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Fader.h>

namespace SparkyStudios::Audio::Amplitude
{
    class CurveDefinition;
    class CurvePartDefinition;

    /**
     * @brief A single point in a Curve.
     */
    struct CurvePoint
    {
        /**
         * @brief The coordinates of the point over the X axis.
         */
        double x;

        /**
         * @brief The coordinates of the point over the Y axis.
         */
        float y;

        bool operator==(const CurvePoint& rhs) const;
        bool operator!=(const CurvePoint& rhs) const;
    };

    /**
     * @brief A part of a Curve.
     *
     * CurveParts allows to a curve to have different fading algorithms at the same time.
     * Each CurvePart has a start and end point, and the fading algorithm which moves the value
     * from the start point to the end point.
     */
    class CurvePart
    {
    public:
        /**
         * @brief Creates an empty CurvePart.
         */
        CurvePart();

        /**
         * @brief Initializes this CurvePart from a definition.
         *
         * @param definition The definition of the curve part generated
         *                   from a flatbuffer binary.
         */
        void Initialize(const CurvePartDefinition* definition);

        /**
         * @brief Returns the start point of this CurvePart.
         *
         * @return The start point of this CurvePart.
         */
        [[nodiscard]] const CurvePoint& GetStart() const;

        /**
         * @brief Sets the start point of this CurvePart.
         *
         * @param start The new start point.
         */
        void SetStart(const CurvePoint& start);

        /**
         * @brief Returns the end point of this CurvePart.
         *
         * @return The end point of this CurvePart.
         */
        [[nodiscard]] const CurvePoint& GetEnd() const;

        /**
         * @brief Sets the end point of this CurvePart.
         *
         * @param end The new end point.
         */
        void SetEnd(const CurvePoint& end);

        /**
         * @brief Returns the Fader of this CurvePart.
         *
         * @return The Fader of this CurvePart.
         */
        [[nodiscard]] Fader* GetFader() const;

        /**
         * @brief Sets the Fader of this CurvePart.
         *
         * @param fader The new Fader.
         */
        void SetFader(Fader::FADER_ALGORITHM fader);

        /**
         * @brief Gets the Y coordinates of a point given its coordinates over the X axis.
         *
         * @param x The coordinates of the point over the X axis.
         *
         * @return The Y coordinates of the point.
         */
        [[nodiscard]] float Get(double x) const;

    private:
        CurvePoint _start;
        CurvePoint _end;
        Fader* _fader;
    };

    /**
     * @brief A Curve which describe the variation of a value (on the Y-axis) according to another (on the X-axis).
     */
    class Curve
    {
    public:
        Curve();

        /**
         * @brief Initializes curve parts from the given definition.
         *
         * @param definition The curve definition data.
         */
        void Initialize(const CurveDefinition* definition);

        /**
         * @brief Get the curve value corresponding to the given X value.
         *
         * @param x The X value.
         *
         * @return The curve value.
         */
        [[nodiscard]] float Get(double x) const;

    private:
        // Finds the curve part corresponding to the given X value.
        [[nodiscard]] const CurvePart* _findCurvePart(double x) const;

        // The parts of the curve.
        std::vector<CurvePart> _parts;
    };
} // namespace SparkyStudios::Audio::Amplitude
#endif // SS_AMPLITUDE_AUDIO_CURVE_H
