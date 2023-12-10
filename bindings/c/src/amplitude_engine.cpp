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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <c/include/amplitude_engine.h>

#include "amplitude_internals.h"

extern "C" {
AM_API_PUBLIC am_bool am_engine_initialize(const am_oschar* config_path)
{
    return BOOL_TO_AM_BOOL(amEngine->Initialize(config_path));
}

AM_API_PUBLIC am_bool am_engine_deinitialize()
{
    return BOOL_TO_AM_BOOL(amEngine->Deinitialize());
}

AM_API_PUBLIC am_bool am_engine_is_initialized()
{
    return BOOL_TO_AM_BOOL(amEngine->IsInitialized());
}

AM_API_PUBLIC void am_engine_set_filesystem(am_filesystem* fs)
{
    amEngine->SetFileSystem(reinterpret_cast<FileSystem*>(fs));
}

AM_API_PUBLIC const am_filesystem* am_engine_get_filesystem()
{
    return reinterpret_cast<const am_filesystem*>(amEngine->GetFileSystem());
}

AM_API_PUBLIC void am_engine_advance_frame(const am_time delta_time)
{
    amEngine->AdvanceFrame(delta_time);
}

AM_API_PUBLIC am_time am_engine_get_total_time()
{
    return amEngine->GetTotalTime();
}

AM_API_PUBLIC am_bool am_engine_load_soundbank(const am_oschar* path)
{
    return BOOL_TO_AM_BOOL(amEngine->LoadSoundBank(path));
}

AM_API_PUBLIC am_bool am_engine_load_soundbank_get_id(const am_oschar* path, am_uint64* out_id)
{
    return BOOL_TO_AM_BOOL(amEngine->LoadSoundBank(path, *out_id));
}

AM_API_PUBLIC am_bool am_engine_load_soundbank_from_memory(const char* fileData)
{
    return BOOL_TO_AM_BOOL(amEngine->LoadSoundBankFromMemory(fileData));
}

AM_API_PUBLIC am_bool am_engine_load_soundbank_from_memory_get_id(const char* fileData, am_uint64* out_id)
{
    return BOOL_TO_AM_BOOL(amEngine->LoadSoundBankFromMemory(fileData, *out_id));
}

AM_API_PUBLIC am_bool am_engine_load_soundbank_from_memory_view(am_uint8* ptr, const am_size size)
{
    return BOOL_TO_AM_BOOL(amEngine->LoadSoundBankFromMemoryView(ptr, size));
}

AM_API_PUBLIC am_bool am_engine_load_soundbank_from_memory_view_get_id(am_uint8* ptr, const am_size size, am_uint64* out_id)
{
    return BOOL_TO_AM_BOOL(amEngine->LoadSoundBankFromMemoryView(ptr, size, *out_id));
}

AM_API_PUBLIC void am_engine_unload_soundbank_by_name(const am_oschar* path)
{
    amEngine->UnloadSoundBank(path);
}

AM_API_PUBLIC void am_engine_unload_soundbank_by_id(const am_uint64 id)
{
    amEngine->UnloadSoundBank(id);
}

AM_API_PUBLIC void am_engine_unload_soundbanks()
{
    amEngine->UnloadSoundBanks();
}

AM_API_PUBLIC void am_engine_start_open_filesystem()
{
    amEngine->StartOpenFileSystem();
}

AM_API_PUBLIC am_bool am_engine_try_finalize_open_filesystem()
{
    return BOOL_TO_AM_BOOL(amEngine->TryFinalizeOpenFileSystem());
}

AM_API_PUBLIC void am_engine_start_close_filesystem()
{
    amEngine->StartCloseFileSystem();
}

AM_API_PUBLIC am_bool am_engine_try_finalize_close_filesystem()
{
    return BOOL_TO_AM_BOOL(amEngine->TryFinalizeCloseFileSystem());
}

AM_API_PUBLIC void am_engine_start_load_sound_files()
{
    amEngine->StartLoadSoundFiles();
}

AM_API_PUBLIC am_bool am_engine_try_finalize_load_sound_files()
{
    return BOOL_TO_AM_BOOL(amEngine->TryFinalizeLoadSoundFiles());
}

AM_API_PUBLIC am_sound_object_handle am_engine_get_sound_object_handle_from_name(const char* name)
{
    return reinterpret_cast<am_sound_object_handle>(amEngine->GetSoundObjectHandle(name));
}

AM_API_PUBLIC am_sound_object_handle am_engine_get_sound_object_handle_from_id(const am_uint64 id)
{
    return reinterpret_cast<am_sound_object_handle>(amEngine->GetSoundObjectHandle(id));
}

AM_API_PUBLIC am_sound_object_handle am_engine_get_sound_object_handle_from_file(const am_oschar* filename)
{
    return reinterpret_cast<am_sound_object_handle>(amEngine->GetSoundObjectHandleFromFile(filename));
}

AM_API_PUBLIC am_switch_container_handle am_engine_get_switch_container_handle_from_name(const char* name)
{
    return reinterpret_cast<am_switch_container_handle>(amEngine->GetSwitchContainerHandle(name));
}

AM_API_PUBLIC am_switch_container_handle am_engine_get_switch_container_handle_from_id(const am_uint64 id)
{
    return reinterpret_cast<am_switch_container_handle>(amEngine->GetSwitchContainerHandle(id));
}

AM_API_PUBLIC am_switch_container_handle am_engine_get_switch_container_handle_from_file(const am_oschar* filename)
{
    return reinterpret_cast<am_switch_container_handle>(amEngine->GetSwitchContainerHandleFromFile(filename));
}

AM_API_PUBLIC am_collection_handle am_engine_get_collection_handle_from_name(const char* name)
{
    return reinterpret_cast<am_collection_handle>(amEngine->GetCollectionHandle(name));
}

AM_API_PUBLIC am_collection_handle am_engine_get_collection_handle_from_id(const am_uint64 id)
{
    return reinterpret_cast<am_collection_handle>(amEngine->GetCollectionHandle(id));
}

AM_API_PUBLIC am_collection_handle am_engine_get_collection_handle_from_file(const am_oschar* filename)
{
    return reinterpret_cast<am_collection_handle>(amEngine->GetCollectionHandleFromFile(filename));
}

AM_API_PUBLIC am_sound_handle am_engine_get_sound_handle_from_name(const char* name)
{
    return reinterpret_cast<am_sound_handle>(amEngine->GetSoundHandle(name));
}

AM_API_PUBLIC am_sound_handle am_engine_get_sound_handle_from_id(const am_uint64 id)
{
    return reinterpret_cast<am_sound_handle>(amEngine->GetSoundHandle(id));
}

AM_API_PUBLIC am_sound_handle am_engine_get_sound_handle_from_file(const am_oschar* filename)
{
    return reinterpret_cast<am_sound_handle>(amEngine->GetSoundHandleFromFile(filename));
}

AM_API_PUBLIC am_event_handle am_engine_get_event_handle_from_name(const char* name)
{
    return reinterpret_cast<am_event_handle>(amEngine->GetEventHandle(name));
}

AM_API_PUBLIC am_event_handle am_engine_get_event_handle_from_id(const am_uint64 id)
{
    return reinterpret_cast<am_event_handle>(amEngine->GetEventHandle(id));
}

AM_API_PUBLIC am_event_handle am_engine_get_event_handle_from_file(const am_oschar* filename)
{
    return reinterpret_cast<am_event_handle>(amEngine->GetEventHandleFromFile(filename));
}

AM_API_PUBLIC am_attenuation_handle am_engine_get_attenuation_handle_from_name(const char* name)
{
    return reinterpret_cast<am_attenuation_handle>(amEngine->GetAttenuationHandle(name));
}

AM_API_PUBLIC am_attenuation_handle am_engine_get_attenuation_handle_from_id(const am_uint64 id)
{
    return reinterpret_cast<am_attenuation_handle>(amEngine->GetAttenuationHandle(id));
}

AM_API_PUBLIC am_attenuation_handle am_engine_get_attenuation_handle_from_file(const am_oschar* filename)
{
    return reinterpret_cast<am_attenuation_handle>(amEngine->GetAttenuationHandleFromFile(filename));
}

AM_API_PUBLIC am_switch_handle am_engine_get_switch_handle_from_name(const char* name)
{
    return reinterpret_cast<am_switch_handle>(amEngine->GetSwitchHandle(name));
}

AM_API_PUBLIC am_switch_handle am_engine_get_switch_handle_from_id(const am_uint64 id)
{
    return reinterpret_cast<am_switch_handle>(amEngine->GetSwitchHandle(id));
}

AM_API_PUBLIC am_switch_handle am_engine_get_switch_handle_from_file(const am_oschar* filename)
{
    return reinterpret_cast<am_switch_handle>(amEngine->GetSwitchHandleFromFile(filename));
}

AM_API_PUBLIC am_rtpc_handle am_engine_get_rtpc_handle_from_name(const char* name)
{
    return reinterpret_cast<am_rtpc_handle>(amEngine->GetRtpcHandle(name));
}

AM_API_PUBLIC am_rtpc_handle am_engine_get_rtpc_handle_from_id(const am_uint64 id)
{
    return reinterpret_cast<am_rtpc_handle>(amEngine->GetRtpcHandle(id));
}

AM_API_PUBLIC am_rtpc_handle am_engine_get_rtpc_handle_from_file(const am_oschar* filename)
{
    return reinterpret_cast<am_rtpc_handle>(amEngine->GetRtpcHandleFromFile(filename));
}

AM_API_PUBLIC am_effect_handle am_engine_get_effect_handle_from_name(const char* name)
{
    return reinterpret_cast<am_effect_handle>(amEngine->GetEffectHandle(name));
}

AM_API_PUBLIC am_effect_handle am_engine_get_effect_handle_from_id(const am_uint64 id)
{
    return reinterpret_cast<am_effect_handle>(amEngine->GetEffectHandle(id));
}

AM_API_PUBLIC am_effect_handle am_engine_get_effect_handle_from_file(const am_oschar* filename)
{
    return reinterpret_cast<am_effect_handle>(amEngine->GetEffectHandleFromFile(filename));
}

AM_API_PUBLIC void am_engine_set_master_gain(const am_float32 gain)
{
    amEngine->SetMasterGain(gain);
}

AM_API_PUBLIC am_float32 am_engine_get_master_gain()
{
    return amEngine->GetMasterGain();
}

AM_API_PUBLIC void am_engine_set_mute(const am_bool mute)
{
    amEngine->SetMute(AM_BOOL_TO_BOOL(mute));
}

AM_API_PUBLIC am_bool am_engine_is_muted()
{
    return BOOL_TO_AM_BOOL(amEngine->IsMuted());
}

AM_API_PUBLIC void am_engine_pause(const am_bool pause)
{
    amEngine->Pause(AM_BOOL_TO_BOOL(pause));
}

AM_API_PUBLIC am_bool am_engine_is_paused()
{
    return BOOL_TO_AM_BOOL(amEngine->IsPaused());
}

AM_API_PUBLIC void am_engine_set_default_listener(am_listener_handle listener)
{
    const Listener l(reinterpret_cast<ListenerInternalState*>(listener));
    amEngine->SetDefaultListener(&l);
}

AM_API_PUBLIC void am_engine_set_default_listener_by_id(const am_uint64 id)
{
    amEngine->SetDefaultListener(id);
}

AM_API_PUBLIC am_listener_handle am_engine_get_default_listener()
{
    return reinterpret_cast<am_listener_handle>(amEngine->GetDefaultListener().GetState());
}

AM_API_PUBLIC am_listener_handle am_engine_add_listener(const am_uint64 id)
{
    return reinterpret_cast<am_listener_handle>(amEngine->AddListener(id).GetState());
}

AM_API_PUBLIC am_listener_handle am_engine_get_listener(const am_uint64 id)
{
    return reinterpret_cast<am_listener_handle>(amEngine->GetListener(id).GetState());
}

AM_API_PUBLIC void am_engine_remove_listener_by_id(const am_uint64 id)
{
    amEngine->RemoveListener(id);
}

AM_API_PUBLIC void am_engine_remove_listener(am_listener_handle listener)
{
    const Listener l(reinterpret_cast<ListenerInternalState*>(listener));
    amEngine->RemoveListener(&l);
}

AM_API_PUBLIC am_entity_handle am_engine_add_entity(const am_uint64 id)
{
    return reinterpret_cast<am_entity_handle>(amEngine->AddEntity(id).GetState());
}

AM_API_PUBLIC am_entity_handle am_engine_get_entity(const am_uint64 id)
{
    return reinterpret_cast<am_entity_handle>(amEngine->GetEntity(id).GetState());
}

AM_API_PUBLIC void am_engine_remove_entity_by_id(const am_uint64 id)
{
    amEngine->RemoveEntity(id);
}

AM_API_PUBLIC void am_engine_remove_entity(am_entity_handle entity)
{
    const Entity e(reinterpret_cast<EntityInternalState*>(entity));
    amEngine->RemoveEntity(&e);
}

AM_API_PUBLIC am_environment_handle am_engine_add_environment(const am_uint64 id)
{
    return reinterpret_cast<am_environment_handle>(amEngine->AddEnvironment(id).GetState());
}

AM_API_PUBLIC am_environment_handle am_engine_get_environment(const am_uint64 id)
{
    return reinterpret_cast<am_environment_handle>(amEngine->GetEnvironment(id).GetState());
}

AM_API_PUBLIC void am_engine_remove_environment_by_id(const am_uint64 id)
{
    amEngine->RemoveEnvironment(id);
}

AM_API_PUBLIC void am_engine_remove_environment(am_environment_handle environment)
{
    const Environment e(reinterpret_cast<EnvironmentInternalState*>(environment));
    amEngine->RemoveEnvironment(&e);
}

AM_API_PUBLIC am_bus_handle am_engine_find_bus_by_name(const char* name)
{
    return reinterpret_cast<am_bus_handle>(amEngine->FindBus(name).GetState());
}

AM_API_PUBLIC am_bus_handle am_engine_find_bus_by_id(const am_uint64 id)
{
    return reinterpret_cast<am_bus_handle>(amEngine->FindBus(id).GetState());
}

AM_API_PUBLIC am_channel_handle
am_engine_play_switch_container_world(am_switch_container_handle handle, const am_vec3* location, am_float32 user_gain)
{
    const auto h = reinterpret_cast<SwitchContainerHandle>(handle);

    return reinterpret_cast<am_channel_handle>(amEngine->Play(h, *location, user_gain).GetState());
}

AM_API_PUBLIC am_channel_handle
am_engine_play_switch_container_entity(am_switch_container_handle handle, am_entity_handle entity, am_float32 user_gain)
{
    const Entity e(reinterpret_cast<EntityInternalState*>(entity));
    const auto h = reinterpret_cast<SwitchContainerHandle>(handle);

    return reinterpret_cast<am_channel_handle>(amEngine->Play(h, e, user_gain).GetState());
}

AM_API_PUBLIC am_channel_handle am_engine_play_collection_world(am_collection_handle handle, const am_vec3* location, am_float32 user_gain)
{
    const auto h = reinterpret_cast<CollectionHandle>(handle);

    return reinterpret_cast<am_channel_handle>(amEngine->Play(h, *location, user_gain).GetState());
}

AM_API_PUBLIC am_channel_handle am_engine_play_collection_entity(am_collection_handle handle, am_entity_handle entity, am_float32 user_gain)
{
    const Entity e(reinterpret_cast<EntityInternalState*>(entity));
    const auto h = reinterpret_cast<CollectionHandle>(handle);

    return reinterpret_cast<am_channel_handle>(amEngine->Play(h, e, user_gain).GetState());
}

AM_API_PUBLIC am_channel_handle am_engine_play_sound_world(am_sound_handle handle, const am_vec3* location, am_float32 user_gain)
{
    const auto h = reinterpret_cast<SoundHandle>(handle);

    return reinterpret_cast<am_channel_handle>(amEngine->Play(h, *location, user_gain).GetState());
}

AM_API_PUBLIC am_channel_handle am_engine_play_sound_entity(am_sound_handle handle, am_entity_handle entity, am_float32 user_gain)
{
    const Entity e(reinterpret_cast<EntityInternalState*>(entity));
    const auto h = reinterpret_cast<SoundHandle>(handle);

    return reinterpret_cast<am_channel_handle>(amEngine->Play(h, e, user_gain).GetState());
}

AM_API_PUBLIC am_channel_handle am_engine_play_sound_object_by_name_world(const char* name, const am_vec3* location, am_float32 user_gain)
{
    return reinterpret_cast<am_channel_handle>(amEngine->Play(name, *location, user_gain).GetState());
}

AM_API_PUBLIC am_channel_handle am_engine_play_sound_object_by_name_entity(const char* name, am_entity_handle entity, am_float32 user_gain)
{
    const Entity e(reinterpret_cast<EntityInternalState*>(entity));

    return reinterpret_cast<am_channel_handle>(amEngine->Play(name, e, user_gain).GetState());
}

AM_API_PUBLIC am_channel_handle am_engine_play_sound_object_by_id_world(am_uint64 id, const am_vec3* location, am_float32 user_gain)
{
    return reinterpret_cast<am_channel_handle>(amEngine->Play(id, *location, user_gain).GetState());
}

AM_API_PUBLIC am_channel_handle am_engine_play_sound_object_by_id_entity(am_uint64 id, am_entity_handle entity, am_float32 user_gain)
{
    const Entity e(reinterpret_cast<EntityInternalState*>(entity));

    return reinterpret_cast<am_channel_handle>(amEngine->Play(id, e, user_gain).GetState());
}

AM_API_PUBLIC void am_engine_stop_all()
{
    amEngine->StopAll();
}

AM_API_PUBLIC am_event_canceler_handle am_engine_trigger_event(am_event_handle handle, am_entity_handle entity)
{
    const Entity e(reinterpret_cast<EntityInternalState*>(entity));
    const auto h = reinterpret_cast<EventHandle>(handle);

    return reinterpret_cast<am_event_canceler_handle>(amEngine->Trigger(h, e).GetEvent());
}

AM_API_PUBLIC am_event_canceler_handle am_engine_trigger_event_by_name(const char* name, am_entity_handle entity)
{
    const Entity e(reinterpret_cast<EntityInternalState*>(entity));

    return reinterpret_cast<am_event_canceler_handle>(amEngine->Trigger(name, e).GetEvent());
}

AM_API_PUBLIC void am_engine_destroy_instance()
{
    amEngine->DestroyInstance();
}
}
