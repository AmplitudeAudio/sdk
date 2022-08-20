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

#ifndef SPARK_AUDIO_BUS_H
#define SPARK_AUDIO_BUS_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    class BusInternalState;

    /**
     * @brief An object representing one node in the tree of buses.
     * Buses are used to adjust a set of channel gains in tandem.
     *
     * The Bus class is a lightweight reference to a BusInternalState object which
     * is managed by the Engine. There is always at least one bus, the master bus,
     * and any number of additional buses may be defined as well. Each bus can be
     * thought as a node in the tree. The gain on a Bus is applied to all child buses as well.
     */
    class Bus
    {
    public:
        /**
         * @brief Creates an uninitialized Bus.
         *
         * An uninitialized Bus cannot set or get any of it's fields.
         * To initialize the Listener, use <code>Engine::AddListener();</code>.
         */
        Bus()
            : _state(nullptr)
        {}

        explicit Bus(BusInternalState* state)
            : _state(state)
        {}

        /**
         * @brief Uninitializes this Bus.
         *
         * Note that this does not destroy the internal state it references,
         * it just removes this reference to it.
         */
        void Clear();

        /**
         * @brief Checks whether this Bus has been initialized.
         *
         * @return boolean true if this Bus has been initialized.
         */
        [[nodiscard]] bool Valid() const;

        /**
         * @brief Gets the unique ID of this Bus.
         *
         * @return The Bus unique ID.
         */
        [[nodiscard]] AmBusID GetId() const;

        /**
         * @brief Gets the name of this Bus.
         *
         * @return The bus name.
         */
        [[nodiscard]] const std::string& GetName() const;

        /**
         * @brief Sets the gain on this Bus.
         *
         * @param gain The new gain value.
         */
        void SetGain(float gain) const;

        /**
         * @brief Returns the user specified fain on this Bus.
         *
         * @return float The user specified gain.
         */
        [[nodiscard]] float GetGain() const;

        /**
         * @brief Fades to <code>gain</code> over <code>duration</code> seconds.
         *
         * @param gain The gain value to fade to.
         * @param duration The amount of time in seconds to take to reach the desired gain.
         */
        void FadeTo(float gain, AmTime duration) const;

        /**
         * @brief Returns the final calculated gain on this Bus.
         *
         * The final gain of a Bus is the product of the gain specified in the Bus
         * definition file, the gain specified by the use, and the final gain of the
         * parent Bus.
         *
         * @return float The final calculated gain.
         */
        [[nodiscard]] float GetFinalGain() const;

        /**
         * @brief Sets the muted state of this Bus.
         *
         * @param mute The muted state.
         */
        void SetMute(bool mute) const;

        /**
         * @brief Returns whether this Bus is muted.
         *
         * @return true if this Bus is muted, false otherwise.
         */
        [[nodiscard]] bool IsMute() const;

        /**
         * @brief Returns the internal state of this Bus.
         *
         * @return BusInternalState*
         */
        [[nodiscard]] BusInternalState* GetState() const;

    private:
        BusInternalState* _state;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_BUS_H
