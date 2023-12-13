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

#ifndef SS_AMPLITUDE_CODEC_H
#define SS_AMPLITUDE_CODEC_H

#include "amplitude_common.h"

struct am_filesystem_file;
typedef struct am_filesystem_file am_filesystem_file;
typedef am_filesystem_file* am_filesystem_file_handle;

struct am_codec;
typedef struct am_codec am_codec;
typedef am_codec* am_codec_handle;

struct am_codec_decoder;
typedef struct am_codec_decoder am_codec_decoder;
typedef am_codec_decoder* am_codec_decoder_handle;

struct am_codec_encoder;
typedef struct am_codec_encoder am_codec_encoder;
typedef am_codec_encoder* am_codec_encoder_handle;

typedef struct
{
    void (*onCreate)(am_voidptr user_data);
    void (*onDestroy)(am_voidptr user_data);
    am_bool (*onOpen)(am_voidptr user_data, am_filesystem_file_handle file);
    am_bool (*onClose)(am_voidptr user_data);
    am_uint64 (*onLoad)(am_voidptr user_data, am_voidptr out);
    am_uint64 (*onStream)(am_voidptr user_data, am_voidptr out, am_uint64 offset, am_uint64 length);
    am_bool (*onSeek)(am_voidptr user_data, am_uint64 offset);
} am_codec_decoder_vtable;

typedef struct
{
    void (*onCreate)(am_voidptr user_data);
    void (*onDestroy)(am_voidptr user_data);
    am_bool (*onOpen)(am_voidptr user_data, am_filesystem_file_handle file);
    am_bool (*onClose)(am_voidptr user_data);
    am_uint64 (*onWrite)(am_voidptr user_data, am_voidptr in, am_uint64 offset, am_uint64 length);
} am_codec_encoder_vtable;

typedef struct
{
    void (*onRegister)(am_voidptr user_data);
    void (*onUnregister)(am_voidptr user_data);
    am_bool (*onCanHandleFile)(am_voidptr user_data, am_filesystem_file_handle file);
} am_codec_vtable;

typedef struct
{
    const char* name;

    am_voidptr user_data;

    am_codec_vtable v_table;

    struct
    {
        am_codec_decoder_vtable v_table;
        am_voidptr user_data;
    } decoder;

    struct
    {
        am_codec_encoder_vtable v_table;
        am_voidptr user_data;
    } encoder;
} am_codec_config;

#ifdef __cplusplus
extern "C" {
#endif

am_codec_config am_codec_config_init(const char* name);

void am_codec_register(const am_codec_config* config);

void am_codec_unregister(const char* name);

am_codec_handle am_codec_find(const char* name);

am_bool am_codec_can_handle_file(am_codec_handle codec, am_filesystem_file_handle file);

am_codec_decoder_handle am_codec_decoder_create(const char* name);

void am_codec_decoder_destroy(const char* name, am_codec_decoder_handle handle);

am_bool am_codec_decoder_open(am_codec_decoder_handle handle, am_filesystem_file_handle file);

am_bool am_codec_decoder_close(am_codec_decoder_handle handle);

am_uint64 am_codec_decoder_load(am_codec_decoder_handle handle, am_voidptr out);

am_uint64 am_codec_decoder_stream(am_codec_decoder_handle handle, am_voidptr out, am_uint64 offset, am_uint64 length);

am_uint64 am_codec_decoder_seek(am_codec_decoder_handle handle, am_uint64 offset);

#ifdef __cplusplus
}
#endif

#endif // SS_AMPLITUDE_CODEC_H
