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

#ifndef SS_AMPLITUDE_AUDIO_SDK_H
#define SS_AMPLITUDE_AUDIO_SDK_H

#include "amplitude_common.h"

struct am_engine; // Opaque type for the Engine class.
typedef struct am_engine am_engine;

struct am_filesystem; // Opaque type for the FileSystem class.
typedef struct am_filesystem am_filesystem;

struct am_sound_object; // Opaque type for the SoundObject class.
typedef struct am_sound_object am_sound_object;
typedef am_sound_object* am_sound_object_handle;

struct am_switch_container; // Opaque type for the SwitchContainer class.
typedef struct am_switch_container am_switch_container;
typedef am_switch_container* am_switch_container_handle;

struct am_collection; // Opaque type for the Collection class.
typedef struct am_collection am_collection;
typedef am_collection* am_collection_handle;

struct am_sound; // Opaque type for the Sound class.
typedef struct am_sound am_sound;
typedef am_sound* am_sound_handle;

struct am_event; // Opaque type for the Event class.
typedef struct am_event am_event;
typedef am_event* am_event_handle;

struct am_attenuation; // Opaque type for the Attenuation class.
typedef struct am_attenuation am_attenuation;
typedef am_attenuation* am_attenuation_handle;

struct am_switch; // Opaque type for the Switch class.
typedef struct am_switch am_switch;
typedef am_switch* am_switch_handle;

struct am_rtpc; // Opaque type for the Rtpc class.
typedef struct am_rtpc am_rtpc;
typedef am_rtpc* am_rtpc_handle;

struct am_effect; // Opaque type for the Rtpc class.
typedef struct am_effect am_effect;
typedef am_effect* am_effect_handle;

struct am_listener; // Opaque type for the Listener class.
typedef struct am_listener am_listener;
typedef am_listener* am_listener_handle;

struct am_entity; // Opaque type for the Entity class.
typedef struct am_entity am_entity;
typedef am_entity* am_entity_handle;

struct am_environment; // Opaque type for the Environment class.
typedef struct am_environment am_environment;
typedef am_environment* am_environment_handle;

struct am_bus; // Opaque type for the Bus class.
typedef struct am_bus am_bus;
typedef am_bus* am_bus_handle;

struct am_channel; // Opaque type for the Channel class.
typedef struct am_channel am_channel;
typedef am_channel* am_channel_handle;

struct am_event_canceler; // Opaque type for the EventCanceler class.
typedef struct am_event_canceler am_event_canceler;
typedef am_event_canceler* am_event_canceler_handle;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the Amplitude engine.
 *
 * @param config_path The path to the configuration file.
 *
 * @return Whether the engine has been successfully initialized.
 */
am_bool am_engine_initialize(const am_oschar* config_path);

/**
 * @brief Deinitializes the Amplitude engine.
 *
 * @return When the engine has been successfully deinitialized.
 */
am_bool am_engine_deinitialize();

/**
 * @brief Checks if the Amplitude engine has been successfully initialized.
 *
 * @return @c AM_TRUE if the engine has been successfully initialized, @c AM_FALSE otherwise.
 */
am_bool am_engine_is_initialized();

/**
 * @brief Set a file system implementation to be used by the engine.
 *
 * @param fs The file system implementation.
 */
void am_engine_set_filesystem(am_filesystem* fs);

/**
 * @brief Gets the file system implementation used by the engine.
 *
 * @return The file system implementation used by the engine.
 */
const am_filesystem* am_engine_get_filesystem();

/**
 * @brief Updates the engine state for the given number of seconds.
 *
 * @param delta_time The number of seconds since the last frame.
 */
void am_engine_advance_frame(am_time delta_time);

/**
 * @brief Gets the total elapsed time since the start of the engine.
 *
 * @return The total elapsed time since the start of the engine.
 */
am_time am_engine_get_total_time();

/**
 * @brief Loads a sound bank from a file. Queue the sound files in that sound
 *        bank for loading. Call StartLoadSoundFiles() to trigger loading
 *        of the sound files on a separate thread.
 *
 * @param path The path to the sound bank file.
 *
 * @return @c true when the sound bank is successfully loaded, @c false otherwise.
 */
am_bool am_engine_load_soundbank(const am_oschar* path);

/**
 * @brief Loads a sound bank from a file. Queue the sound files in that sound
 *        bank for loading. Call StartLoadSoundFiles() to trigger loading
 *        of the sound files on a separate thread.
 *
 * @param path The path to the sound bank file.
 * @param out_id The ID of the loaded sound bank.
 *
 * @return @c true when the sound bank is successfully loaded, @c false otherwise.
 */
am_bool am_engine_load_soundbank_get_id(const am_oschar* path, am_uint64& out_id);

/**
 * @brief Loads a sound bank from memory. Queue the sound files in that sound
 *        bank for loading. Call StartLoadSoundFiles() to trigger loading
 *        of the sound files on a separate thread.
 *
 * @param fileData The sound bank data to be loaded.
 *
 * @return @c true when the sound bank is successfully loaded, @c false otherwise.
 */
am_bool am_engine_load_soundbank_from_memory(const char* fileData);

/**
 * @brief Loads a sound bank from memory. Queue the sound files in that sound
 *        bank for loading. Call StartLoadSoundFiles() to trigger loading
 *        of the sound files on a separate thread.
 *
 * @param fileData The sound bank data to be loaded.
 * @param out_id The ID of the loaded sound bank.
 *
 * @return @c true when the sound bank is successfully loaded, @c false otherwise.
 */
am_bool am_engine_load_soundbank_from_memory_get_id(const char* fileData, am_uint64& out_id);

/**
 * @brief Loads a sound bank from memory. Queue the sound files in that sound
 *        bank for loading. Call StartLoadSoundFiles() to trigger loading
 *        of the sound files on a separate thread.
 *
 * @param ptr The pointer to the sound bank data to be loaded.
 * @param size The size of the memory to read.
 *
 * @return @c true when the sound bank is successfully loaded, @c false otherwise.
 */
am_bool am_engine_load_soundbank_from_memory_view(am_uint8* ptr, am_size size);

/**
 * @brief Loads a sound bank from memory. Queue the sound files in that sound
 *        bank for loading. Call StartLoadSoundFiles() to trigger loading
 *        of the sound files on a separate thread.
 *
 * @param ptr The pointer to the sound bank data to be loaded.
 * @param size The size of the memory to read.
 * @param out_id The ID of the loaded sound bank.
 *
 * @return @c true when the sound bank is successfully loaded, @c false otherwise.
 */
am_bool am_engine_load_soundbank_from_memory_view_get_id(am_uint8* ptr, am_size size, am_uint64& out_id);

/**
 * @brief Unloades a sound bank.
 *
 * @param path The path to the sound bank to unload.
 */
void am_engine_unload_soundbank_by_name(const am_oschar* path);

/**
 * @brief Unloades a sound bank.
 *
 * @param id The ID to the sound bank to unload.
 */
void am_engine_unload_soundbank_by_id(am_uint64 id);

/**
 * @brief Unloads all the loaded sound banks.
 */
void am_engine_unload_soundbanks();

/**
 * @brief Opens the file system, usually in a separate thread.
 */
void am_engine_start_open_filesystem();

/**
 * @brief Returns @c true if the file system has been fully loaded. Must call
 *        am_engine_start_open_filesystem() first.
 */
am_bool am_engine_try_finalize_open_filesystem();

/**
 * @brief Closes the file system, usually in a separate thread.
 */
void am_engine_start_close_filesystem();

/**
 * @brief Returns @c true if the file system has been fully closed. Must call
 *        am_engine_start_close_filesystem() first.
 */
am_bool am_engine_try_finalize_close_filesystem();

/**
 * @brief Starts the loading of sound files referenced in loaded sound banks.
 *
 * This process will run in another thread. You must call TryFinalizeLoadSoundFiles() to
 * know when the loading has completed, and to release used resources.
 */
void am_engine_start_load_sound_files();

/**
 * @brief Checks if the loading of sound files has been completed, and releases used resources.
 *
 * @return @c true when the sound files have been successfully loaded, @c false otherwise.
 */
am_bool am_engine_try_finalize_load_sound_files();

/**
 * @brief Gets a sound object given its name as defined in its JSON data.
 *
 * Can be a @c am_switch_container_handle, @c am_collection_handle, or @c am_sound_handle.
 *
 * @param name The unique name as defined in the JSON data.
 */
am_sound_object_handle am_engine_get_sound_object_handle_from_name(const char* name);

/**
 * @brief Gets a sound object given its ID as defined in its JSON data.
 *
 * @param id The unique ID as defined in the JSON data.
 */
am_sound_object_handle am_engine_get_sound_object_handle_from_id(am_uint64 id);

/**
 * @brief Gets a sound object given its file name.
 *
 * @param filename The filename containing the flatbuffer binary data.
 */
am_sound_object_handle am_engine_get_sound_object_handle_from_file(const am_oschar* filename);

/**
 * @brief Gets a switch container given its name as defined in its JSON data.
 *
 * @param name The unique name as defined in the JSON data.
 */
am_switch_container_handle am_engine_get_switch_container_handle_from_name(const char* name);

/**
 * @brief Gets a switch container given its ID as defined in its JSON data.
 *
 * @param id The unique ID as defined in the JSON data.
 */
am_switch_container_handle am_engine_get_switch_container_handle_from_id(am_uint64 id);

/**
 * @brief Gets a switch container given its file name.
 *
 * @param filename The filename containing the flatbuffer binary data.
 */
am_switch_container_handle am_engine_get_switch_container_handle_from_file(const am_oschar* filename);

/**
 * @brief Gets a collection given its name as defined in its JSON data.
 *
 * @param name The unique name as defined in the JSON data.
 */
am_collection_handle am_engine_get_collection_handle_from_name(const char* name);

/**
 * @brief Gets a collection given its ID as defined in its JSON data.
 *
 * @param id The unique ID as defined in the JSON data.
 */
am_collection_handle am_engine_get_collection_handle_from_id(am_uint64 id);

/**
 * @brief Gets a collection given its file name.
 *
 * @param filename The filename containing the flatbuffer binary data.
 */
am_collection_handle am_engine_get_collection_handle_from_file(const am_oschar* filename);

/**
 * @brief Gets a sound given its name as defined in its JSON data.
 *
 * @param name The unique name as defined in the JSON data.
 */
am_sound_handle am_engine_get_sound_handle_from_name(const char* name);

/**
 * @brief Gets a sound given its ID as defined in its JSON data.
 *
 * @param id The unique ID as defined in the JSON data.
 */
am_sound_handle am_engine_get_sound_handle_from_id(am_uint64 id);

/**
 * @brief Gets a sound given its file name.
 *
 * @param filename The filename containing the flatbuffer binary data.
 */
am_sound_handle am_engine_get_sound_handle_from_file(const am_oschar* filename);

/**
 * @brief Gets an event given its name as defined in its JSON data.
 *
 * @param name The unique name as defined in the JSON data.
 */
am_event_handle am_engine_get_event_handle_from_name(const char* name);

/**
 * @brief Gets an event given its ID as defined in its JSON data.
 *
 * @param id The unique ID as defined in the JSON data.
 */
am_event_handle am_engine_get_event_handle_from_id(am_uint64 id);

/**
 * @brief Gets an event given its file name.
 *
 * @param filename The filename containing the flatbuffer binary data.
 */
am_event_handle am_engine_get_event_handle_from_file(const am_oschar* filename);

/**
 * @brief Gets an attenuation given its name as defined in its JSON data.
 *
 * @param name The unique name as defined in the JSON data.
 */
am_attenuation_handle am_engine_get_attenuation_handle_from_name(const char* name);

/**
 * @brief Gets an attenuation given its ID as defined in its JSON data.
 *
 * @param id The unique ID as defined in the JSON data.
 */
am_attenuation_handle am_engine_get_attenuation_handle_from_id(am_uint64 id);

/**
 * @brief Gets an attenuation given its file name.
 *
 * @param filename The filename containing the flatbuffer binary data.
 */
am_attenuation_handle am_engine_get_attenuation_handle_from_file(const am_oschar* filename);

/**
 * @brief Gets an switch given its name as defined in its JSON data.
 *
 * @param name The unique name as defined in the JSON data.
 */
am_switch_handle am_engine_get_switch_handle_from_name(const char* name);

/**
 * @brief Gets an switch given its ID as defined in its JSON data.
 *
 * @param id The unique ID as defined in the JSON data.
 */
am_switch_handle am_engine_get_switch_handle_from_id(am_uint64 id);

/**
 * @brief Gets an switch given its file name.
 *
 * @param filename The filename containing the flatbuffer binary data.
 */
am_switch_handle am_engine_get_switch_handle_from_file(const am_oschar* filename);

/**
 * @brief Gets an rtpc given its name as defined in its JSON data.
 *
 * @param name The unique name as defined in the JSON data.
 */
am_rtpc_handle am_engine_get_rtpc_handle_from_name(const char* name);

/**
 * @brief Gets an rtpc given its ID as defined in its JSON data.
 *
 * @param id The unique ID as defined in the JSON data.
 */
am_rtpc_handle am_engine_get_rtpc_handle_from_id(am_uint64 id);

/**
 * @brief Gets an rtpc given its file name.
 *
 * @param filename The filename containing the flatbuffer binary data.
 */
am_rtpc_handle am_engine_get_rtpc_handle_from_file(const am_oschar* filename);

/**
 * @brief Gets an effect given its name as defined in its JSON data.
 *
 * @param name The unique name as defined in the JSON data.
 */
am_effect_handle am_engine_get_effect_handle_from_name(const char* name);

/**
 * @brief Gets an effect given its ID as defined in its JSON data.
 *
 * @param id The unique ID as defined in the JSON data.
 */
am_effect_handle am_engine_get_effect_handle_from_id(am_uint64 id);

/**
 * @brief Gets an effect given its file name.
 *
 * @param filename The filename containing the flatbuffer binary data.
 */
am_effect_handle am_engine_get_effect_handle_from_file(const am_oschar* filename);

/**
 * @brief Adjusts the master gain of the mixer.
 *
 * @param gain The master gain.
 */
void am_engine_set_master_gain(am_float32 gain);

/**
 * @brief Gets the mixer master gain.
 *
 * @return The mixer master gain.
 */
am_float32 am_engine_get_master_gain();

/**
 * @brief Mutes the engine, but keep processing audio.
 *
 * @param mute Whether to mute or unmute the engine.
 */
void am_engine_set_mute(am_bool mute);

/**
 * @brief Whether the engine is currently muted.
 *
 * @return @c true if the engine is muted, @c false otherwise.
 */
am_bool am_engine_is_muted();

/**
 * @brief Pauses or resumes all playing sounds and streams.
 *
 * @param pause Whether to pause or resume the engine.
 */
void am_engine_pause(am_bool pause);

/**
 * @brief Whether the engine is currently paused.
 *
 * @return @c true if the engine is currently paused, @c false otherwise.
 */
am_bool am_engine_is_paused();

/**
 * @brief Sets the default sound listener.
 *
 * @param listener A valid and initialized Listener instance.
 */
void am_engine_set_default_listener(am_listener_handle listener);

/**
 * @brief Sets the default sound listener.
 *
 * @param id A valid Listener ID.
 */
void am_engine_set_default_listener_by_id(am_uint64 id);

/**
 * @brief Returns a Listener object storing the state of the default
 * audio listener.
 *
 * @return An initialized Listener object if a default listener was set,
 * otherwise an unitialized Listener object.
 */
am_listener_handle am_engine_get_default_listener();

/**
 * @brief Initializes and returns a @c Listener.
 *
 * @param id The sound listener ID.
 *
 * @return An initialized Listener.
 */
am_listener_handle am_engine_add_listener(am_uint64 id);

/**
 * @brief Returns the @c Listener with the given ID.
 *
 * @param id The sound listener ID.
 *
 * @return An initialized Listener if that one has been registered before,
 * otherwise an unitialized Listener.
 */
am_listener_handle am_engine_get_listener(am_uint64 id);

/**
 * @brief Removes a @c Listener given its ID.
 *
 * @param id The ID of the Listener to be removed.
 */
void am_engine_remove_listener_by_id(am_uint64 id);

/**
 * @brief Removes a @c Listener given its handle.
 *
 * @param listener The Listener to be removed.
 */
void am_engine_remove_listener(am_listener_handle listener);

/**
 * @brief Initializes and returns an @c am_entity.
 *
 * @param id The game entity ID.
 *
 * @return An initialized am_entity.
 */
am_entity_handle am_engine_add_entity(am_uint64 id);

/**
 * @brief Returns the @c am_entity with the given ID.
 *
 * @param id The game entity ID.
 *
 * @return An initialized entity if that one has been registered before,
 * otherwise an unitialized entity.
 */
am_entity_handle am_engine_get_entity(am_uint64 id);

/**
 * @brief Removes a @c am_entity given its ID.
 *
 * @param id The ID of the game entity to be removed.
 */
void am_engine_remove_entity_by_id(am_uint64 id);

/**
 * @brief Removes an @c am_entity given its handle.
 *
 * @param entity The am_entity to be removed.
 */
void am_engine_remove_entity(am_entity_handle entity);

/**
 * @brief Initializes and returns an @c am_environment.
 *
 * @param id The game environment ID.
 *
 * @return An initialized am_environment.
 */
am_environment_handle am_engine_add_environment(am_uint64 id);

/**
 * @brief Returns the @c am_environment with the given ID.
 *
 * @param id The game environment ID.
 *
 * @return An initialized environment if that one has been registered before,
 * otherwise an unitialized environment.
 */
am_environment_handle am_engine_get_environment(am_uint64 id);

/**
 * @brief Removes a @c am_environment given its ID.
 *
 * @param id The ID of the game environment to be removed.
 */
void am_engine_remove_environment_by_id(am_uint64 id);

/**
 * @brief Removes an @c am_environment given its handle.
 *
 * @param environment The environment to be removed.
 */
void am_engine_remove_environment(am_environment_handle environment);

/**
 * @brief Returns the @c am_bus_handle with the specified name.
 *
 * @param name The name of the bus.
 *
 * @return A valid bus if found, otherwise an invalid bus.
 */
am_bus_handle am_engine_find_bus_by_name(const char* name);

/**
 * @brief Returns the @c am_bus_handle with the given ID.
 *
 * @param id The ID of the bus.
 *
 * @return A valid bus if found, otherwise an invalid bus.
 */
am_bus_handle am_engine_find_bus_by_id(am_uint64 id);

/**
 * @brief Plays a switch container associated with the given handle in the World scope.
 *
 * @param handle A handle to the switch container to play.
 * @param location The location on which play the switch container.
 * @param user_gain The gain of the sound.
 *
 * @return The channel the switch container is played on. If the switch container could not be
 *         played, an invalid Channel is returned.
 */
am_channel_handle am_engine_play_switch_container_world(
    am_switch_container_handle handle, const am_vec3& location = {}, am_float32 user_gain = 1.0f);

/**
 * @brief Plays a switch container associated with the given handle in an Entity scope with the
 * given gain.
 *
 * @param handle A handle to the switch container to play.
 * @param entity The entity which is playing the switch container.
 * @param user_gain The gain of the sound.
 *
 * @return The channel the switch container is played on. If the switch container could not be
 *         played, an invalid Channel is returned.
 */
am_channel_handle am_engine_play_switch_container_entity(
    am_switch_container_handle handle, am_entity_handle entity, am_float32 user_gain = 1.0f);

/**
 * @brief Plays a collection associated with the given handle in the location with
 * the given gain.
 *
 * @param handle A handle to the collection to play.
 * @param location The location on which play the collection.
 * @param user_gain The gain of the sound.
 *
 * @return The channel the collection is played on. If the collection could not be
 * played, an invalid Channel is returned.
 */
am_channel_handle am_engine_play_collection_world(am_collection_handle handle, const am_vec3& location = {}, am_float32 user_gain = 1.0f);

/**
 * @brief Plays a collection associated with the given handle in an Entity scope
 * with the given gain.
 *
 * @param handle A handle to the collection to play.
 * @param entity The entity which is playing the collection.
 * @param user_gain The gain of the sound.
 *
 * @return The channel the collection is played on. If the collection could not be
 * played, an invalid Channel is returned.
 */
am_channel_handle am_engine_play_collection_entity(am_collection_handle handle, am_entity_handle entity, am_float32 user_gain = 1.0f);

/**
 * @brief Plays a sound associated with the given handle in the World scope
 * at the given location with the given gain.
 *
 * @param handle A handle to the sound to play.
 * @param location The location on which play the sound.
 * @param user_gain The gain of the sound.
 *
 * @return The channel the sound is played on. If the sound could not be
 * played, an invalid Channel is returned.
 */
am_channel_handle am_engine_play_sound_world(am_sound_handle handle, const am_vec3& location = {}, am_float32 user_gain = 1.0f);

/**
 * @brief Plays a sound associated with the given handle in an Entity scope
 * with the given gain.
 *
 * @param handle A handle to the sound to play.
 * @param entity The entity which is playing the sound.
 * @param user_gain The gain of the sound.
 *
 * @return The channel the sound is played on. If the sound could not be
 * played, an invalid Channel is returned.
 */
am_channel_handle am_engine_play_sound_entity(am_sound_handle handle, am_entity_handle entity, am_float32 user_gain = 1.0f);

/**
 * @brief Plays a sound object associated with the given name in the World scope.
 *
 * @note Playing a sound object with its handle is faster than using the
 * name as using the name requires a map lookup internally.
 *
 * @param name The name of the sound object to play.
 * @param location The location of the sound.
 * @param user_gain The gain of the sound.
 *
 * @return The channel the sound is played on. If the object could not be
 * played, an invalid Channel is returned.
 */
am_channel_handle am_engine_play_sound_object_by_name_world(const char* name, const am_vec3& location = {}, am_float32 user_gain = 1.0f);

/**
 * @brief Plays a sound object associated with the given name in an Entity scope.
 *
 * @note Playing a sound object with its handle is faster than using the
 * name as using the name requires a map lookup internally.
 *
 * @param name The name of the sound object to play.
 * @param entity The entity which is playing the sound.
 * @param user_gain The gain of the sound.
 *
 * @return The channel the sound is played on. If the object could not be
 * played, an invalid Channel is returned.
 */
am_channel_handle am_engine_play_sound_object_by_name_entity(const char* name, am_entity_handle entity, am_float32 user_gain = 1.0f);

/**
 * @brief Plays a sound object associated with the given ID in the
 * World scope, at the given location, and with the given gain.
 *
 * @note Playing an object with its handle is faster than using the
 * ID as using the ID requires a map lookup internally.
 *
 * @param id The ID of the object to play.
 * @param location The location of the sound.
 * @param user_gain The gain of the sound.
 *
 * @return The channel the sound is played on. If the sound could not be
 * played, an invalid Channel is returned.
 */
am_channel_handle am_engine_play_sound_object_by_id_world(am_uint64 id, const am_vec3& location = {}, am_float32 user_gain = 1.0f);

/**
 * @brief Plays a sound object associated with the given ID in an Entity
 * scope with the given gain.
 *
 * @note Playing an object with its handle is faster than using the
 * ID as using the ID requires a map lookup internally.
 *
 * @param id The ID of the sound object to play.
 * @param entity The entity which is playing the sound.
 * @param user_gain The gain of the sound.
 *
 * @return The channel the sound is played on. If the sound could not be
 * played, an invalid Channel is returned.
 */
am_channel_handle am_engine_play_sound_object_by_id_entity(am_uint64 id, am_entity_handle entity, am_float32 user_gain = 1.0f);

/**
 * @brief Stops all playing sound objects.
 */
void am_engine_stop_all();

/**
 * @brief Triggers the event associated to the given handle at the given location.
 *
 * @param handle The handle of the event to trigger.
 * @param entity The entity which trigger the event.
 *
 * @return An @c EventCanceler object which may be used to cancel the execution of the event..
 */
am_event_canceler_handle am_engine_trigger_event(am_event_handle handle, am_entity_handle entity);

/**
 * @brief Triggers the event associated to the given handle.
 *
 * @note Triggering an event with its EventHandle is faster than using the
 * event name as using the name requires a map lookup internally.
 *
 * @param name The name of event to trigger.
 * @param entity The entity in which trigger the event.
 *
 * @return An @c EventCanceler object which may be used to cancel the execution of the event..
 */
am_event_canceler_handle am_engine_trigger_event_by_name(const char* name, am_entity_handle entity);

/**
 * @brief Destroys the unique Amplitude engine.
 */
void am_engine_destroy_instance();

#ifdef __cplusplus
}
#endif

#endif // SS_AMPLITUDE_AUDIO_SDK_H
