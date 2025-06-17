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

#ifndef _AM_CORE_PLAYBACK_BUS_H
#define _AM_CORE_PLAYBACK_BUS_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    class BusInternalState;

    /**
     * @brief An object representing one node in the tree of buses. Buses are used to adjust a set of channel gains in tandem.
     *
     * The @c Bus class is a lightweight reference to a @c BusInternalState object which is managed internally by the Engine. There
     * is always at least one bus, the "master" bus, and any number of additional buses may be defined as well. Each bus can be
     * thought as a node in the tree. The gain on a @c Bus is applied to all child buses as well.
     *
     * @ingroup engine
     */
    class AM_API_PUBLIC Bus
    {
    public:
        /**
         * @brief Creates an uninitialized bus.
         *
         * @note An uninitialized bus cannot set or get any of its fields.
         */
        Bus();

        /**
         * @brief Creates a wrapper instance over the provided state.
         *
         * @warning This constructor is for internal usage only.
         *
         * @param[in] state The internal state to wrap.
         */
        explicit Bus(BusInternalState* state);

        /**
         * @brief Uninitializes this bus.
         *
         * @note This does not destroy the internal state it references, it just removes this reference to it.
         */
        void Clear();

        /**
         * @brief Checks whether this bus has been initialized.
         *
         * @return @c true if this bus has been initialized, @c false otherwise.
         */
        [[nodiscard]] bool Valid() const;

        /**
         * @brief Gets the unique ID of this bus.
         *
         * @return The bus unique ID.
         */
        [[nodiscard]] AmBusID GetId() const;

        /**
         * @brief Gets the name of this bus.
         *
         * @return The bus name.
         */
        [[nodiscard]] const AmString& GetName() const;

        /**
         * @brief Sets the gain of this bus.
         *
         * @param[in] gain The new gain value.
         */
        void SetGain(AmReal32 gain) const;

        /**
         * @brief Returns the user specified gain on this bus.
         *
         * @return The user specified gain.
         */
        [[nodiscard]] AmReal32 GetGain() const;

        /**
         * @brief Fades to @c gain over @c duration milliseconds.
         *
         * @param[in] gain The gain value to fade to.
         * @param[in] duration The amount of time in milliseconds to take to reach the desired gain.
         */
        void FadeTo(AmReal32 gain, AmTime duration) const;

        /**
         * @brief Returns the final calculated gain on this bus.
         *
         * @note The final gain of a bus is the product of the gain specified in the bus
         * definition file, with the gain specified by the user, and with the final gain
         * of the parent bus.
         *
         * @return The final calculated gain.
         */
        [[nodiscard]] AmReal32 GetFinalGain() const;

        /**
         * @brief Sets the muted state of this bus.
         *
         * @param[in] mute The muted state.
         */
        void SetMute(bool mute) const;

        /**
         * @brief Returns whether this bus is muted.
         *
         * @return @c true if this bus is muted, @c false otherwise.
         */
        [[nodiscard]] bool IsMuted() const;

        /**
         * @brief Returns the internal state of this bus.
         *
         * @warning This method is only for internal usage only.
         *
         * @return The bus internal state.
         */
        [[nodiscard]] BusInternalState* GetState() const;

    private:
        /**
         * @brief Internal state of the bus.
         *
         * @internal This field is not exposed to the public API.
         */
        BusInternalState* _state;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_PLAYBACK_BUS_H
