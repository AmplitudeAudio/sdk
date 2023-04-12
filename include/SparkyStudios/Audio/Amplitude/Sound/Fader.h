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

#ifndef SS_AMPLITUDE_AUDIO_FADER_H
#define SS_AMPLITUDE_AUDIO_FADER_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    class Mixer;

    /**
     * @brief Helper class to process faders.
     *
     * A fader is used to move a value to a specific target value
     * during an amount of time and according to a fading algorithm.
     */
    class Fader
    {
        friend class Mixer;

    public:
        enum FADER_ALGORITHM : AmInt8
        {
            /**
             * @brief Fades linearly from the initial value to the final value.
             */
            ALGORITHM_LINEAR = 0,

            /**
             * @brief Keeps returning the initial value until the transition is complete.
             */
            ALGORITHM_CONSTANT = 1,

            /**
             * @brief Returns values using an S-shaped curve from the initial value to the final value.
             */
            ALGORITHM_S_CURVE_SMOOTH = 2,

            /**
             * @brief Returns values using an exponential growth algorithm from the initial value to the final value.
             */
            ALGORITHM_EXPONENTIAL_SMOOTH = 3,

            /**
             * @brief Returns values using an S-shaped curve from the initial value to the final value with
             * sharper transitions.
             */
            ALGORITHM_S_CURVE_SHARP = 4,

            /**
             * @brief Returns values using an exponential growth algorithm from the initial value to the final value
             * with sharper transitions.
             */
            ALGORITHM_EXPONENTIAL_SHARP = 5,
        };

        Fader();
        virtual ~Fader() = default;

        /**
         * @brief Set up fader.
         *
         * @param from The start value.
         * @param to The target value.
         * @param duration The duration of transition.
         */
        void Set(float from, float to, AmTime duration);

        /**
         * @brief Set up fader.
         *
         * @param from The start value.
         * @param to The target value.
         */
        void Set(float from, float to);

        /**
         * @brief Set the duration of the transition.
         *
         * @param duration The transition duration.
         */
        void SetDuration(AmTime duration);

        /**
         * @brief Get the current fading value.
         *
         * To use this method you need to define the fading stat time using
         * <code>Fader::Start()</code> firstly.
         *
         * @param time The time at which the value should be calculated.
         *
         * @return The current value.
         */
        virtual float GetFromTime(AmTime time);

        /**
         * @brief Get the current fading value.
         *
         * @param percentage The percentage of time elapsed. This should be in the range [0, 1].
         *
         * @return The current value.
         */
        virtual float GetFromPercentage(double percentage) = 0;

        /**
         * @brief Get the state of this Fader.
         *
         * @return The Fader state.
         */
        [[nodiscard]] AM_FADER_STATE GetState() const
        {
            return m_state;
        }

        /**
         * @brief Set the state of this Fader.
         *
         * @param state The state to set.
         */
        void SetState(AM_FADER_STATE state)
        {
            m_state = state;
        }

        /**
         * @brief Sets the fading start time.
         *
         * @param time The fading start time.
         */
        void Start(AmTime time);

        /**
         * @brief Creates a fader from the provided algorithm.
         *
         * @param algorithm The fader algorithm.
         *
         * @return A fader instance which matches the provided algorithm.
         */
        static Fader* Create(FADER_ALGORITHM algorithm);

    protected:
        // Value to fade from
        float m_from;
        // Value to fade to
        float m_to;
        // Delta between from and to
        float m_delta;
        // Total AmTime to fade
        AmTime m_time;
        // Time fading started
        AmTime m_startTime;
        // Time fading will end
        AmTime m_endTime;
        // Active flag; 0 means disabled, 1 is active, -1 means was active, but stopped
        AM_FADER_STATE m_state;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_FADER_H
