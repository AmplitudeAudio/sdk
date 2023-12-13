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

#ifndef SS_AMPLITUDE_CHANNEL_H
#define SS_AMPLITUDE_CHANNEL_H

#include "amplitude_common.h"

struct am_channel;
typedef struct am_channel am_channel;
typedef am_channel* am_channel_handle;

typedef enum am_channel_playback_state
    : am_uint8
{
    am_channel_playback_state_stopped = 0,
    am_channel_playback_state_playing = 1,
    am_channel_playback_state_fading_in = 2,
    am_channel_playback_state_fading_out = 3,
    am_channel_playback_state_switching_state = 4,
    am_channel_playback_state_paused = 5,
} am_channel_playback_state;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Checks if the sound associated to the given channel is playing.
 *
 * @param channel The channel to check.
 *
 * @return Whether the channel is currently playing.
 */
am_bool am_channel_playing(am_channel_handle channel);

/**
 * @brief Stops a channel.
 *
 * Stops the channel from playing. A sound will stop on its own if its not set
 * to loop. Looped audio must be explicitly stopped.
 *
 * @param channel The channel to stop.
 */
void am_channel_stop(am_channel_handle channel);

/**
 * @brief Stops a channel.
 *
 * Stops the channel from playing. A sound will stop on its own if its not set
 * to loop. Looped audio must be explicitly stopped.
 *
 * @param channel The channel to stop.
 * @param duration The fade out duration before to stop the channel.
 */
void am_channel_stop_timeout(am_channel_handle channel, am_time duration);

/**
 * @brief Pauses a channel.
 *
 * Pauses the channel. A paused channel may be resumed where it left off.
 *
 * @param channel The channel to pause.
 */
void am_channel_pause(am_channel_handle channel);

/**
 * @brief Pauses a channel.
 *
 * Pauses the channel. A paused channel may be resumed where it left off.
 *
 * @param channel The channel to pause.
 * @param duration The fade out duration before to pause the channel.
 */
void am_channel_pause_timeout(am_channel_handle channel, am_time duration);

/**
 * @brief Resumes a paused channel.
 *
 * Resumes the channel. If the channel was paused it will continue where it
 * left off.
 *
 * @param channel The channel to resume.
 */
void am_channel_resume(am_channel_handle channel);

/**
 * @brief Resumes a paused channel.
 *
 * Resumes the channel. If the channel was paused it will continue where it
 * left off.
 *
 * @param channel The channel to resume.
 * @param duration The fade in duration after resuming the channel.
 */
void am_channel_resume_timeout(am_channel_handle channel, am_time duration);

/**
 * @brief Gets the location of the given channel.
 *
 * If the audio on the channel is not set to be Positional, this method will
 * return an invalid location.
 *
 * @param channel The channel to get the location of.
 *
 * @return The location of the given channel.
 */
am_vec3 am_channel_get_location(am_channel_handle channel);

/**
 * @brief Sets the location of the given channel.
 *
 * If the audio on the channel is not set to be Positional, this method
 * does nothing.
 *
 * @param channel The channel to set the location of.
 * @param location The new location of the channel.
 */
void am_channel_set_location(am_channel_handle channel, am_vec3 location);

/**
 * @brief Gets the gain of the given channel.
 *
 * @param channel The channel to get the gain of.
 *
 * @return The gain of the channel.
 */
am_float32 am_channel_get_gain(am_channel_handle channel);

/**
 * @brief Sets the gain on the given channel.
 *
 * @param channel The channel to set the gain of.
 * @param gain The new gain value.
 */
void am_channel_set_gain(am_channel_handle channel, am_float32 gain);

/**
 * @brief Gets the playback state of the given channel.
 *
 * @param channel The channel to get the playback state of.
 *
 * @return The playback state of the given channel.
 */
am_channel_playback_state am_channel_get_playback_state(am_channel_handle channel);

#ifdef __cplusplus
}
#endif

#endif // SS_AMPLITUDE_CHANNEL_H
