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

#ifndef _AM_SOUND_FADER_H
#define _AM_SOUND_FADER_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Math/Utils.h>

#include <map>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Lists the possible states a fader can have.
     *
     * @ingroup engine
     */
    enum eFaderState : AmInt8
    {
        /**
         * @brief The fader is stopped.
         */
        eFaderState_Stopped = -1,

        /**
         * @brief The fader is disabled.
         */
        eFaderState_Disabled = 0,

        /**
         * @brief The fader is active and fading.
         */
        eFaderState_Active = 1,
    };

    /**
     * @brief A Fader instance.
     *
     * The @c FaderInstance class uses transition curves similar to CSS transition functions, through an implementation of a fast
     * one-dimensional cubic Bézier curve evaluator. The @ref Transition struct is used to define the coordinates of the curve's
     * control points.
     *
     * An object of this class will be created each time a @c Fader is requested.
     *
     * @ingroup engine
     */
    class AM_API_PUBLIC FaderInstance
    {
    public:
        /**
         * @brief Create an animation transition function using a one-dimensional cubic Bézier curve.
         *
         * This uses the exact same algorithm as in CSS. The first and last control points of the
         * cubic Bézier curve are fixed to (0,0) and (1,1) respectively.
         */
        struct Transition
        {
        public:
            /**
             * @brief Constructs a new transition curve.
             *
             * @param[in] x1 The x coordinate of the second control point.
             * @param[in] y1 The y coordinate of the second control point.
             * @param[in] x2 The x coordinate of the third control point.
             * @param[in] y2 The y coordinate of the third control point.
             */
            Transition(AmReal32 x1, AmReal32 y1, AmReal32 x2, AmReal32 y2);

            /**
             * @brief Constructs a new transition curve.
             *
             * @param[in] controlPoints The control points of the curve.
             */
            explicit Transition(const BezierCurveControlPoints& controlPoints);

            /**
             * @brief Calculates animation progression.
             *
             * Given an animation duration percentage (in the range [0, 1]), it calculates the animation
             * progression percentage from the configured curve.
             *
             * @param[in] t The animation duration percentage (in the range [0, 1]).
             *
             * @return The animation progress percentage (in the range [0, 1]).
             */
            [[nodiscard]] AmTime Ease(AmTime t) const;

            /**
             * @brief The control points.
             */
            BezierCurveControlPoints m_controlPoints;

        private:
            [[nodiscard]] AmTime GetTFromX(AmReal64 x) const;

            AmReal64 _samples[11];
        };

        /**
         * @brief Constructs a new @c FaderInstance object.
         *
         * This will initialize the fader instance state to default values.
         */
        FaderInstance();

        /**
         * @brief Default destructor.
         */
        virtual ~FaderInstance() = default;

        /**
         * @brief Resets the fading state with the given values.
         *
         * @param[in] from The start value.
         * @param[in] to The target value.
         * @param[in] duration The duration of transition.
         */
        void Set(AmReal64 from, AmReal64 to, AmTime duration);

        /**
         * @brief Resets the fading state with the given values.
         *
         * @param[in] from The start value.
         * @param[in] to The target value.
         */
        void Set(AmReal64 from, AmReal64 to);

        /**
         * @brief Sets the duration of the transition.
         *
         * @param[in] duration The transition duration.
         */
        void SetDuration(AmTime duration);

        /**
         * @brief Gets the current fading value.
         *
         * To use this method, you first need to define the fading start time using
         * @ref Start "`Start()`".
         *
         * @param[in] time The time at which the value should be calculated.
         *
         * @return The current value.
         */
        virtual AmReal64 GetFromTime(AmTime time);

        /**
         * @brief Gets the current fading value.
         *
         * @param[in] percentage The percentage of time elapsed. This should be in the range [0, 1].
         *
         * @return The current value.
         */
        virtual AmReal64 GetFromPercentage(AmReal64 percentage);

        /**
         * @brief Gets the state of this @c FaderInstance.
         *
         * @return The current @c FaderInstance state.
         */
        [[nodiscard]] AM_INLINE eFaderState GetState() const
        {
            return m_state;
        }

        /**
         * @brief Sets the state of this @c FaderInstance.
         *
         * @warning Unless you know what you are doing, it's preferred to leave this state managed by the engine.
         *
         * @param[in] state The state to set.
         */
        AM_INLINE void SetState(eFaderState state)
        {
            m_state = state;
        }

        /**
         * @brief Sets the fading start time.
         *
         * @param[in] time The fading start time.
         */
        void Start(AmTime time = 0.0);

    protected:
        /**
         * @brief Value to fade from (origin value).
         */
        AmReal64 m_from;

        /**
         * @brief Value to fade to (target value).
         */
        AmReal64 m_to;

        /**
         * @brief Delta between the origin value and the target value.
         */
        AmReal64 m_delta;

        /**
         * @brief Total fading duration (in milliseconds).
         */
        AmTime m_time;

        /**
         * @brief Time at which the fading has started.
         */
        AmTime m_startTime;

        /**
         * @brief Time at which the fading will end.
         */
        AmTime m_endTime;

        /**
         * @brief Current state of the fading operation.
         */
        eFaderState m_state;

        /**
         * @brief The transition function to use while fading.
         */
        Transition m_curve;
    };

    /**
     * @brief Base class used to create faders.
     *
     * A fader is used to move a value to a target value during a specific amount of time and according
     * to a fading algorithm. The @c Fader class implements factory methods to create instances of @c FaderInstance
     * objects, which are where the value is being transitioned to its target.
     *
     * The @c Fader class follows the [plugin architecture](/plugins/anatomy), and thus, you are able to create
     * your own faders and register them to the Engine by inheriting from this class and by implementing the necessary dependencies.
     *
     * @ingroup engine
     */
    class AM_API_PUBLIC Fader
    {
    public:
        /**
         * @brief a new fader instance.
         *
         * @param name The fader name, e.g., "MiniAudioLinear".
         */
        explicit Fader(AmString name);

        /**
         * @brief Default fader constructor.
         *
         * @warning This constructor is meant for internal faders only.
         */
        Fader();

        /**
         * @brief Default destructor.
         */
        virtual ~Fader();

        /**
         * @brief Creates a new instance of the fader.
         *
         * @return A new instance of the fader.
         */
        virtual std::shared_ptr<FaderInstance> CreateInstance() = 0;

        /**
         * @brief Gets the name of this fader.
         *
         * @return The name of this fader.
         */
        [[nodiscard]] const AmString& GetName() const;

        /**
         * @brief Gets the control points of the transition curve used by this fader.
         *
         * @return The control points of the transition curve used by this fader.
         */
        [[nodiscard]] virtual BezierCurveControlPoints GetControlPoints() const = 0;

        /**
         * @brief Registers a new fader.
         *
         * @note This method does nothing if the registry is locked.
         *
         * @param[in] fader The fader to add in the registry.
         *
         * @see LockRegistry, UnlockRegistry
         */
        static void Register(std::shared_ptr<Fader> fader);

        /**
         * @brief Unregisters a fader.
         *
         * @note This method does nothing if the registry is locked.
         *
         * @param[in] fader The Fader to remove from the registry.
         *
         * @see LockRegistry, UnlockRegistry
         */
        static void Unregister(std::shared_ptr<const Fader> fader);

        /**
         * @brief Look up a fader by name.
         *
         * @param[in] name The name of the fader to find.
         *
         * @return The fader with the given name, or @c nullptr if not found.
         */
        static std::shared_ptr<Fader> Find(const AmString& name);

        /**
         * @brief Creates a new instance of the fader with the given name and returns its pointer.
         *
         * @param[in] name The name of the fader.
         *
         * @return The fader with the given name, or @c nullptr if not found.
         */
        static std::shared_ptr<FaderInstance> Construct(const AmString& name);

        /**
         * @brief Locks the faders' registry.
         *
         * @warning This function is mainly used for internal purposes. It's
         * called before the @c Engine initialization, to discard the registration
         * of new faders after the engine is fully loaded.
         */
        static void LockRegistry();

        /**
         * @brief Unlocks the faders' registry.
         *
         * @warning This function is mainly used for internal purposes. It's
         * called after the @c Engine deinitialization, to allow the registration
         * of new faders after the engine is fully unloaded.
         */
        static void UnlockRegistry();

        /**
         * @brief Gets the list of registered faders.
         *
         * @return The registry of faders.
         */
        static const std::map<AmString, std::shared_ptr<Fader>>& GetRegistry();

    protected:
        /**
         * @brief The name of this fader.
         */
        AmString m_name;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_SOUND_FADER_H
