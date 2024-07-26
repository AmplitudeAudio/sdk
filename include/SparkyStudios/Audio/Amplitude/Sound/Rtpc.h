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

#ifndef _AM_SOUND_RTPC_H
#define _AM_SOUND_RTPC_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Core/Asset.h>

#include <SparkyStudios/Audio/Amplitude/Math/Curve.h>

#include <SparkyStudios/Audio/Amplitude/IO/File.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct RtpcCompatibleValue;

    /**
     * @brief Amplitude Real-Time Parameter Control.
     *
     * A RTPC is a value that is updated by the game. Any update to the RTPC is
     * listened by the engine to propagate the changes to other parameters linked to it.
     *
     * A Rtpc object is shared between any objects and values linked to it.
     */
    class AM_API_PUBLIC Rtpc : public Asset<AmRtpcID>
    {
    public:
        /**
         * @brief Updates the value of the RTPC.
         *
         * This method is useful only for RTPCs that are using a curve to update their value.
         *
         * @param deltaTime The time elapsed since the last update.
         */
        virtual void Update(AmTime deltaTime) = 0;

        /**
         * @brief Get the minimum value of this RTPC.
         *
         * @return The RTPC minimum value.
         */
        [[nodiscard]] virtual AmReal64 GetMinValue() const = 0;

        /**
         * @brief Get the maximum value of this RTPC.
         *
         * @return The RTPC maximum value.
         */
        [[nodiscard]] virtual AmReal64 GetMaxValue() const = 0;

        /**
         * @brief Get the current value of this RTPC.
         *
         * @return The current RTPC value.
         */
        [[nodiscard]] virtual AmReal64 GetValue() const = 0;

        /**
         * @brief Set the current value of this RTPC.
         *
         * @param value The value to set.
         */
        virtual void SetValue(AmReal64 value) = 0;

        /**
         * @brief Get the default value of this RTPC.
         *
         * @return The default RTPC value.
         */
        [[nodiscard]] virtual AmReal64 GetDefaultValue() const = 0;

        /**
         * @brief Resets the current RTPC value to the default value.
         */
        virtual void Reset() = 0;
    };

    /**
     * @brief A RTPC compatible value is used as a wrapper to hold propertiy values
     * that can be linked to RTPCs.
     *
     * A property value that can be linked to a RTPC can be either a single static value
     * that never updates, or a curve and an RTPC value that is updated by the game. The
     * curve is used here as a function that takes the current RTPC value and returns the
     * parameter value.
     */
    struct AM_API_PUBLIC RtpcValue
    {
    public:
        static void Init(RtpcValue& value, const RtpcCompatibleValue* definition, AmReal32 staticValue);

        /**
         * @brief Creates an unitialized RtpcValue object.
         *
         * An unitialized RtpcValue object cannot be used to update values.
         */
        RtpcValue();

        RtpcValue(const RtpcValue& other);

        /**
         * @brief Destroys the RtpcValue object.
         */
        ~RtpcValue();

        /**
         * @brief Creates a RtpcValue object with a static value.
         *
         * @param value The static value to set.
         */
        void Init(AmReal32 value);

        /**
         * @brief Creates a RtpcValue object with a curve and an RTPC object.
         *
         * @param rtpc The RTPC to link to.
         * @param curve The curve to use.
         */
        void Init(const Rtpc* rtpc, Curve* curve);

        /**
         * @brief Creates a RtpcValue object from an asset definition.
         *
         * @param definition The RTPC-compatible value asset definition.
         */
        void Init(const RtpcCompatibleValue* definition);

        /**
         * @brief Gets the current RTPC value. For static values, this will always
         * return the value passed to the constructor or set from an asset definition.
         *
         * @return The current RTPC value.
         */
        [[nodiscard]] AmReal32 GetValue() const;

        /**
         * @brief Checks if the RTPC value is static.
         * @return @c true if the RTPC value is static, @c false otherwise.
         */
        [[nodiscard]] bool IsStatic() const;

    private:
        AmInt8 _valueKind;
        AmReal32 _value;
        Curve* _curve;
        bool _ownCurve;
        const Rtpc* _rtpc;

        bool _initialized;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_SOUND_RTPC_H
