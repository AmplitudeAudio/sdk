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

#include <SparkyStudios/Audio/Amplitude/Math/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    class Mixer;

    /**
     * @brief A Fader instance. An object of this class will be created each time
     * a Fader is requested.
     */
    class AM_API_PUBLIC FaderInstance
    {
    public:
        /**
         * @brief Create an animation transition function using
         * a one-dimensional cubic bezier curve.
         *
         * This use the exact same algorithm as in CSS. The first and last
         * control points of the cubic bezier curve are fixed to (0,0)
         * and (1,1) respectively.
         */
        struct Transition
        {
        public:
            /**
             * @brief Construct a new Transition curve.
             *
             * @param x1 The x coordinate of the second control point.
             * @param y1 The y coordinate of the second control point.
             * @param x2 The x coordinate of the third control point.
             * @param y2 The y coordinate of the third control point.
             */
            Transition(AmReal32 x1, AmReal32 y1, AmReal32 x2, AmReal32 y2);

            /**
             * @brief Construct a new Transition curve.
             *
             * @param controlPoints The control points of the curve.
             */
            Transition(const BeizerCurveControlPoints& controlPoints);

            /**
             * @brief Given an animation duration percentage (in the range [0, 1]),
             * it calculates the animation progression percentage from the configured curve.
             *
             * @param t The animation duration percentage (in the range [0, 1]).
             *
             * @return The animation progress percentage (in the range [0, 1]).
             */
            [[nodiscard]] AmTime Ease(AmTime t) const;

            /**
             * @brief The control points.
             */
            BeizerCurveControlPoints m_controlPoints;

        private:
            [[nodiscard]] AmTime GetTFromX(AmReal64 x) const;

            AmReal64 _samples[11];
        };

        /**
         * @brief Construct a new FaderInstance object.
         *
         * This will initialize the fader instance state to default values.
         */
        FaderInstance();

        virtual ~FaderInstance() = default;

        /**
         * @brief Set up fader.
         *
         * @param from The start value.
         * @param to The target value.
         * @param duration The duration of transition.
         */
        void Set(AmReal64 from, AmReal64 to, AmTime duration);

        /**
         * @brief Set up fader.
         *
         * @param from The start value.
         * @param to The target value.
         */
        void Set(AmReal64 from, AmReal64 to);

        /**
         * @brief Set the duration of the transition.
         *
         * @param duration The transition duration.
         */
        void SetDuration(AmTime duration);

        /**
         * @brief Get the current fading value.
         *
         * To use this method you firstly need to define the fading start time using
         * <code>Fader::Start()</code>.
         *
         * @param time The time at which the value should be calculated.
         *
         * @return The current value.
         */
        virtual AmReal64 GetFromTime(AmTime time);

        /**
         * @brief Get the current fading value.
         *
         * @param percentage The percentage of time elapsed. This should be in the range [0, 1].
         *
         * @return The current value.
         */
        virtual AmReal64 GetFromPercentage(AmReal64 percentage);

        /**
         * @brief Get the state of this Fader.
         *
         * @return The Fader state.
         */
        [[nodiscard]] AM_INLINE(AM_FADER_STATE) GetState() const
        {
            return m_state;
        }

        /**
         * @brief Set the state of this Fader.
         *
         * @param state The state to set.
         */
        AM_INLINE(void) SetState(AM_FADER_STATE state)
        {
            m_state = state;
        }

        /**
         * @brief Sets the fading start time.
         *
         * @param time The fading start time.
         */
        void Start(AmTime time = 0.0);

    protected:
        // Value to fade from
        AmReal64 m_from;
        // Value to fade to
        AmReal64 m_to;
        // Delta between from and to
        AmReal64 m_delta;
        // Total AmTime to fade
        AmTime m_time;
        // Time fading started
        AmTime m_startTime;
        // Time fading will end
        AmTime m_endTime;
        // Active flag; 0 means disabled, 1 is active, -1 means was active, but stopped
        AM_FADER_STATE m_state;
        // The transition function
        Transition m_curve;
    };

    /**
     * @brief Helper class to process faders.
     *
     * A fader is used to move a value to a specific target value
     * during an amount of time and according to a fading algorithm.
     */
    class AM_API_PUBLIC Fader
    {
        friend class Mixer;

    public:
        /**
         * @brief Create a new Fader instance.
         *
         * @param name The Fader name. eg. "MiniAudioLinear".
         */
        explicit Fader(std::string name);

        /**
         * @brief Default Fader constructor.
         *
         * This will not automatically register the Fader. It's meant for internal Faders only.
         */
        Fader();

        virtual ~Fader();

        /**
         * @brief Creates a new instance of the Fader.
         * @return A new instance of the Fader.
         */
        virtual FaderInstance* CreateInstance() = 0;

        /**
         * @brief Destroys an instance of the Fader. The instance should have
         * been created with CreateInstance().
         * @param instance The Fader instance to be destroyed.
         */
        virtual void DestroyInstance(FaderInstance* instance) = 0;

        /**
         * @brief Gets the name of this Fader.
         *
         * @return The name of this Fader.
         */
        [[nodiscard]] const std::string& GetName() const;

        /**
         * @brief Gets the control points of the transition curve used by this Fader.
         *
         * @return The control points of the transition curve used by this Fader.
         */
        [[nodiscard]] virtual BeizerCurveControlPoints GetControlPoints() const = 0;

        /**
         * @brief Registers a new fader.
         *
         * @param fader The Fader to add in the registry.
         */
        static void Register(Fader* fader);

        /**
         * @brief Unregister a fader.
         *
         * @param fader The Fader to remove from the registry.
         */
        static void Unregister(const Fader* fader);

        /**
         * @brief Creates a new instance of the the Fader with the given name
         * and returns its pointer. The returned pointer should be deleted using Fader::Destruct().
         *
         * @param name The name of the Fader.
         *
         * @return The Fader with the given name, or NULL if none.
         */
        static FaderInstance* Construct(const std::string& name);

        /**
         * @brief Destroys the given Fader instance.
         *
         * @param name The name of the Fader.
         * @param instance The Fader instance to destroy.
         */
        static void Destruct(const std::string& name, FaderInstance* instance);

        /**
         * @brief Locks the faders registry.
         *
         * This function is mainly used for internal purposes. Its
         * called before the Engine initialization, to discard the
         * registration of new Faders after the engine is fully loaded.
         */
        static void LockRegistry();

        /**
         * @brief Unlocks the faders registry.
         *
         * This function is mainly used for internal purposes. Its
         * called after the Engine deinitialization, to allow the
         * registration of new divers after the engine is fully unloaded.
         */
        static void UnlockRegistry();

        /**
         * @brief Gets the list of registered Faders.
         *
         * @return The registry of Faders.
         */
        static const std::map<std::string, Fader*>& GetRegistry();

        /**
         * @brief Look up a Fader by name.
         *
         * @return The Fader with the given name, or NULL if none.
         */
        static Fader* Find(const std::string& name);

    protected:
        /**
         * @brief The name of this Fader.
         */
        std::string m_name;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_FADER_H
