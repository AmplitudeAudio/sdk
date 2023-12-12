// Copyright (c) 2023-present Sparky Studios. All rights reserved.
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

#ifndef SS_AMPLITUDE_BUS_H
#define SS_AMPLITUDE_BUS_H

#include "amplitude_common.h"

struct am_bus;
typedef struct am_bus am_bus;
typedef am_bus* am_bus_handle;

typedef am_uint64 am_bus_id;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Gets the unique ID of the given bus.
 *
 * @param bus The bus.
 *
 * @return The Bus unique ID.
 */
am_bus_id am_bus_get_id(am_bus_handle bus);

/**
 * @brief Gets the name of the given bus.
 *
 * @param bus The bus.
 *
 * @return The bus name.
 */
const char* am_bus_get_name(am_bus_handle bus);

/**
 * @brief Sets the gain of the given bus.
 *
 * @param bus The bus.
 * @param gain The new gain value.
 */
void am_bus_set_gain(am_bus_handle bus, am_float32 gain);

/**
 * @brief Returns the user specified gain on the given bus.
 *
 * @param bus The bus.
 *
 * @return The user specified gain.
 */
am_float32 am_bus_get_gain(am_bus_handle bus);

/**
 * @brief Fades to <code>gain</code> over <code>duration</code> seconds.
 *
 * @param bus The bus.
 * @param target_gain The gain value to fade to.
 * @param duration The amount of time in seconds to take to reach the desired gain.
 */
void am_bus_fade_to(am_bus_handle bus, am_float32 target_gain, am_time duration);

/**
 * @brief Returns the final calculated gain of the given bus.
 *
 * @note The final gain of a bus is the product of the gain specified in the bus
 * definition file, the gain specified by the user, and the final gain of the
 * parent Bus.
 *
 * @param bus The bus.
 *
 * @return AmReal32 The final calculated gain.
 */
am_float32 am_bus_get_final_gain(am_bus_handle bus);

/**
 * @brief Sets the muted state of the given bus.
 *
 * @param bus The bus to mute/unmute.
 * @param mute The muted state.
 */
void am_bus_set_mute(am_bus_handle bus, am_bool mute);

/**
 * @brief Returns whether the given bus is muted.
 *
 * @param bus The bus to check.
 *
 * @return @c true if the bus is muted, @c false otherwise.
 */
am_bool am_bus_is_muted(am_bus_handle bus);

#ifdef __cplusplus
}
#endif

#endif // SS_AMPLITUDE_BUS_H
