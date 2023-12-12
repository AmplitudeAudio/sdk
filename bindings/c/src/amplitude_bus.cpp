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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <c/include/amplitude_bus.h>

#include "amplitude_internals.h"

extern "C" {
AM_API_PUBLIC am_bus_id am_bus_get_id(am_bus_handle bus)
{
    const Bus b(reinterpret_cast<BusInternalState*>(bus));
    return b.GetId();
}

AM_API_PUBLIC const char* am_bus_get_name(am_bus_handle bus)
{
    const Bus b(reinterpret_cast<BusInternalState*>(bus));
    return am_allocate_string(b.GetName().c_str());
}

AM_API_PUBLIC void am_bus_set_gain(am_bus_handle bus, am_float32 gain)
{
    const Bus b(reinterpret_cast<BusInternalState*>(bus));
    b.SetGain(gain);
}

AM_API_PUBLIC am_float32 am_bus_get_gain(am_bus_handle bus)
{
    const Bus b(reinterpret_cast<BusInternalState*>(bus));
    return b.GetGain();
}

AM_API_PUBLIC void am_bus_fade_to(am_bus_handle bus, am_float32 target_gain, am_time duration)
{
    const Bus b(reinterpret_cast<BusInternalState*>(bus));
    b.FadeTo(target_gain, duration);
}

AM_API_PUBLIC am_float32 am_bus_get_final_gain(am_bus_handle bus)
{
    const Bus b(reinterpret_cast<BusInternalState*>(bus));
    return b.GetFinalGain();
}

AM_API_PUBLIC void am_bus_set_mute(am_bus_handle bus, am_bool mute)
{
    const Bus b(reinterpret_cast<BusInternalState*>(bus));
    b.SetMute(mute);
}

AM_API_PUBLIC am_bool am_bus_is_muted(am_bus_handle bus)
{
    const Bus b(reinterpret_cast<BusInternalState*>(bus));
    return b.IsMuted();
}
}