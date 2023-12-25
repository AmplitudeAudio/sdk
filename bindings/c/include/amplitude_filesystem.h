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

#ifndef SS_AMPLITUDE_FILESYSTEM_H
#define SS_AMPLITUDE_FILESYSTEM_H

#include "amplitude_common.h"

typedef enum am_file_type
    : am_uint8
{
    am_file_type_unknown = 0,
    am_file_type_custom = 1,
    am_file_type_disk = 2,
    am_file_type_memory = 3,
} am_file_type;

typedef enum am_filesystem_type
    : am_uint8
{
    am_filesystem_type_unknown = 0,
    am_filesystem_type_custom = 1,
    am_filesystem_type_disk = 2,
} am_filesystem_type;

struct am_file; // Opaque type for the File class.
typedef struct am_file am_file;
typedef struct
{
    am_file_type type;
    am_voidptr handle;
} am_file_handle;

struct am_file_disk; // Opaque type for the DiskFile class.
typedef struct am_file_disk am_file_disk;
typedef am_file_disk* am_file_disk_handle;

struct am_file_memory; // Opaque type for the MemoryFile class.
typedef struct am_file_memory am_file_memory;
typedef am_file_memory* am_file_memory_handle;

typedef enum am_file_open_mode
    : am_uint8
{
    am_file_open_mode_read = 0,
    am_file_open_mode_write = 1,
    am_file_open_mode_append = 2,
    am_file_open_mode_readwrite = 3,
    am_file_open_mode_readappend = 4
} am_file_open_mode;

typedef enum am_file_open_kind
    : am_uint8
{
    am_file_open_kind_binary = 0,
    am_file_open_kind_text = 1
} am_file_open_kind;

typedef enum am_file_seek_origin
    : am_uint8
{
    am_file_seek_origin_start = SEEK_SET,
    am_file_seek_origin_current = SEEK_CUR,
    am_file_seek_origin_end = SEEK_END
} am_file_seek_origin;

typedef struct
{
    void (*create)(am_voidptr user_data);
    void (*destroy)(am_voidptr user_data);
    const am_oschar* (*get_path)(am_voidptr user_data);
    am_bool (*eof)(am_voidptr user_data);
    am_size (*read)(am_voidptr user_data, am_uint8* buffer, am_size bytes);
    am_size (*write)(am_voidptr user_data, const am_uint8* buffer, am_size bytes);
    am_size (*length)(am_voidptr user_data);
    void (*seek)(am_voidptr user_data, am_uint64 offset, am_file_seek_origin origin);
    am_size (*position)(am_voidptr user_data);
    am_bool (*is_valid)(am_voidptr user_data);
} am_file_vtable;

typedef struct
{
    am_file_type type;

    // Only used if type is am_file_type_custom.
    am_voidptr user_data;
    am_file_vtable* v_table;
} am_file_config;

struct am_filesystem; // Opaque type for the FileSystem class.
typedef struct am_filesystem am_filesystem;
typedef struct
{
    am_filesystem_type type;
    am_voidptr handle;
} am_filesystem_handle;

typedef struct
{
    void (*create)(am_voidptr user_data);
    void (*destroy)(am_voidptr user_data);
    void (*set_base_path)(am_voidptr user_data, const am_oschar* base_path);
    const am_oschar* (*resolve_path)(am_voidptr user_data, const am_oschar* path);
    am_bool (*exists)(am_voidptr user_data, const am_oschar* path);
    am_bool (*is_directory)(am_voidptr user_data, const am_oschar* path);
    const am_oschar* (*join)(am_voidptr user_data, const am_oschar** paths, am_uint32 path_count);
    am_file_handle (*open_file)(am_voidptr user_data, const am_oschar* path);
    void (*start_open_filesystem)(am_voidptr user_data);
    am_bool (*try_finalize_open_filesystem)(am_voidptr user_data);
    void (*start_close_filesystem)(am_voidptr user_data);
    am_bool (*try_finalize_close_filesystem)(am_voidptr user_data);
} am_filesystem_vtable;

typedef struct
{
    am_filesystem_type type;

    // Only used if type is am_filesystem_type_custom.
    am_voidptr user_data;
    am_filesystem_vtable* v_table;
} am_filesystem_config;

#ifdef __cplusplus
extern "C" {
#endif

am_file_config am_file_config_init_custom();
am_file_config am_file_config_init_disk();
am_file_config am_file_config_init_memory();

am_file_handle am_file_create(const am_file_config* config);

void am_file_destroy(am_file_handle file);

const am_oschar* am_file_get_path(am_file_handle file);

am_uint8 am_file_read8(am_file_handle file);

am_uint16 am_file_read16(am_file_handle file);

am_uint32 am_file_read32(am_file_handle file);

am_bool am_file_eof(am_file_handle file);

am_size am_file_read(am_file_handle file, am_uint8* buffer, am_size bytes);

am_size am_file_write(am_file_handle file, const am_uint8* buffer, am_size bytes);

am_size am_file_length(am_file_handle file);

void am_file_seek(am_file_handle file, am_size offset, am_file_seek_origin origin);

am_size am_file_position(am_file_handle file);

am_bool am_file_is_valid(am_file_handle file);

am_filesystem_config am_filesystem_config_init_custom();
am_filesystem_config am_filesystem_config_init_disk();

am_filesystem_handle am_filesystem_create(const am_filesystem_config* config);

void am_filesystem_destroy(am_filesystem_handle filesystem);

void am_filesystem_set_base_path(am_filesystem_handle filesystem, const am_oschar* base_path);

const am_oschar* am_filesystem_resolve_path(am_filesystem_handle filesystem, const am_oschar* path);

am_bool am_filesystem_exists(am_filesystem_handle filesystem, const am_oschar* path);

am_bool am_filesystem_is_directory(am_filesystem_handle filesystem, const am_oschar* path);

const am_oschar* am_filesystem_join(am_filesystem_handle filesystem, const am_oschar** parts, am_size count);

am_file_handle am_filesystem_open_file(am_filesystem_handle filesystem, const am_oschar* path);

void am_filesystem_start_open(am_filesystem_handle filesystem);

am_bool am_filesystem_try_finalize_open(am_filesystem_handle filesystem);

void am_filesystem_start_close(am_filesystem_handle filesystem);

am_bool am_filesystem_try_finalize_close(am_filesystem_handle filesystem);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SS_AMPLITUDE_FILESYSTEM_H
