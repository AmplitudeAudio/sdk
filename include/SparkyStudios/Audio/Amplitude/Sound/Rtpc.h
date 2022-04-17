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

#ifndef SS_AMPLITUDE_AUDIO_RTPC_H
#define SS_AMPLITUDE_AUDIO_RTPC_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/RefCounter.h>

#include <SparkyStudios/Audio/Amplitude/Math/Curve.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Fader.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct EngineInternalState;

    struct RtpcDefinition;
    struct RtpcCompatibleValue;

    class Rtpc
    {
    public:
        Rtpc();
        ~Rtpc();

        bool LoadRtpcDefinition(const std::string& source);

        bool LoadRtpcDefinitionFromFile(const AmOsString& filename);

        [[nodiscard]] const RtpcDefinition* GetRtpcDefinition() const;

        void Update(AmTime deltaTime);

        /**
         * @brief Returns the unique ID of this RTPC.
         *
         * @return The RTPC unique ID.
         */
        [[nodiscard]] AmRtpcID GetId() const;

        /**
         * @brief Returns the name of this RTPC.
         *
         * @return The RTPC name.
         */
        [[nodiscard]] const std::string& GetName() const;

        /**
         * @brief Get the minimum value of this RTPC.
         *
         * @return The RTPC minimum value.
         */
        [[nodiscard]] AmReal64 GetMinValue() const;

        /**
         * @brief Get the maximum value of this RTPC.
         *
         * @return The RTPC maximum value.
         */
        [[nodiscard]] AmReal64 GetMaxValue() const;

        /**
         * @brief Get the current value of this RTPC.
         *
         * @return The current RTPC value.
         */
        [[nodiscard]] AmReal64 GetValue() const;

        /**
         * @brief Set the current value of this RTPC.
         *
         * @param value The value to set.
         */
        void SetValue(AmReal64 value);

        /**
         * @brief Get the default value of this RTPC.
         * 
         * @return The default RTPC value.
         */
        [[nodiscard]] AmReal64 GetDefaultValue() const;

        /**
         * @brief Resets the current RTPC value to the default value.
         */
        void Reset();

        /**
         * @brief Get the references counter of this instance.
         *
         * @return The references counter.
         */
        RefCounter* GetRefCounter();

    private:
        std::string _source;

        AmRtpcID _id;
        std::string _name;

        AmReal64 _minValue;
        AmReal64 _maxValue;
        AmReal64 _defValue;

        AmReal64 _currentValue;
        AmReal64 _targetValue;

        Fader* _faderAttack;
        Fader* _faderRelease;

        RefCounter _refCounter;
    };

    struct RtpcValue
    {
    public:
        RtpcValue();
        explicit RtpcValue(AmReal32 value);
        explicit RtpcValue(const Rtpc* rtpc, const Curve* curve);
        explicit RtpcValue(const RtpcCompatibleValue* definition);

        float GetValue() const;

    private:
        AmInt8 _valueKind;
        AmReal64 _value;
        const Rtpc* _rtpc;
        const Curve* _curve;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_RTPC_H
