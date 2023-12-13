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

#include <c/include/amplitude_codec.h>

#include "amplitude_internals.h"

class CCodec final : public Codec
{
public:
    class CDecoder final : public Decoder
    {
    public:
        CDecoder(const Codec* codec, am_codec_decoder_vtable* v_table, am_voidptr user_data = nullptr)
            : Decoder(codec)
            , _v_table(v_table)
            , _user_data(user_data)
        {
            if (_v_table->onCreate)
                _v_table->onCreate(_user_data);
        }

        ~CDecoder() override
        {
            if (_v_table->onDestroy)
                _v_table->onDestroy(_user_data);

            _v_table = nullptr;
            _user_data = nullptr;
        }

        bool Open(std::shared_ptr<File> file) override
        {
            return AM_BOOL_TO_BOOL(_v_table->onOpen(_user_data, reinterpret_cast<am_filesystem_file_handle>(file.get())));
        }

        bool Close() override
        {
            return AM_BOOL_TO_BOOL(_v_table->onClose(_user_data));
        }

        AmUInt64 Load(AmVoidPtr out) override
        {
            return _v_table->onLoad(_user_data, out);
        }

        AmUInt64 Stream(AmVoidPtr out, AmUInt64 offset, AmUInt64 length) override
        {
            return _v_table->onStream(_user_data, out, offset, length);
        }

        bool Seek(AmUInt64 offset) override
        {
            return AM_BOOL_TO_BOOL(_v_table->onSeek(_user_data, offset));
        }

    private:
        am_codec_decoder_vtable* _v_table;
        am_voidptr _user_data;
    };

    class CEncoder final : public Encoder
    {
    public:
        explicit CEncoder(const Codec* codec, am_codec_encoder_vtable* v_table, am_voidptr user_data = nullptr)
            : Encoder(codec)
            , _v_table(v_table)
            , _user_data(user_data)
        {
            if (_v_table->onCreate)
                _v_table->onCreate(_user_data);
        }

        ~CEncoder() override
        {
            if (_v_table->onDestroy)
                _v_table->onDestroy(_user_data);

            _v_table = nullptr;
            _user_data = nullptr;
        }

        bool Open(std::shared_ptr<File> file) override
        {
            return AM_BOOL_TO_BOOL(_v_table->onOpen(_user_data, reinterpret_cast<am_filesystem_file_handle>(file.get())));
        }

        bool Close() override
        {
            return AM_BOOL_TO_BOOL(_v_table->onClose(_user_data));
        }

        AmUInt64 Write(AmVoidPtr in, AmUInt64 offset, AmUInt64 length) override
        {
            return _v_table->onWrite(_user_data, in, offset, length);
        }

    private:
        am_codec_encoder_vtable* _v_table;
        am_voidptr _user_data;
    };

    explicit CCodec(const am_codec_config& config)
        : Codec(config.name)
        , _config(config)
    {
        if (_config.v_table.onRegister)
            _config.v_table.onRegister(_config.user_data);
    }

    ~CCodec() override
    {
        if (_config.v_table.onUnregister)
            _config.v_table.onUnregister(_config.user_data);
    }

    Decoder* CreateDecoder() override
    {
        return ampoolnew(MemoryPoolKind::Codec, CDecoder, this, &_config.decoder.v_table, _config.decoder.user_data);
    }

    void DestroyDecoder(Decoder* decoder) override
    {
        ampooldelete(MemoryPoolKind::Codec, CDecoder, (CDecoder*)decoder);
    }

    Encoder* CreateEncoder() override
    {
        return ampoolnew(MemoryPoolKind::Codec, CEncoder, this, &_config.encoder.v_table, _config.encoder.user_data);
    }

    void DestroyEncoder(Encoder* encoder) override
    {
        ampooldelete(MemoryPoolKind::Codec, CEncoder, (CEncoder*)encoder);
    }

    [[nodiscard]] bool CanHandleFile(std::shared_ptr<File> file) const override
    {
        return AM_BOOL_TO_BOOL(_config.v_table.onCanHandleFile(_config.user_data, reinterpret_cast<am_filesystem_file_handle>(file.get())));
    }

private:
    am_codec_config _config;
};

static std::map<std::string_view, AmUniquePtr<MemoryPoolKind::Codec, CCodec>> g_registered_codecs = {};

extern "C" {
AM_API_PUBLIC am_codec_config am_codec_config_init(const char* name)
{
    am_codec_config config;

    config.name = name;
    config.user_data = nullptr;
    config.v_table.onRegister = nullptr;
    config.v_table.onUnregister = nullptr;
    config.v_table.onCanHandleFile = nullptr;
    config.decoder.v_table.onCreate = nullptr;
    config.decoder.v_table.onDestroy = nullptr;
    config.decoder.v_table.onOpen = nullptr;
    config.decoder.v_table.onClose = nullptr;
    config.decoder.v_table.onLoad = nullptr;
    config.decoder.v_table.onStream = nullptr;
    config.decoder.v_table.onSeek = nullptr;
    config.decoder.user_data = nullptr;
    config.encoder.v_table.onCreate = nullptr;
    config.encoder.v_table.onDestroy = nullptr;
    config.encoder.v_table.onOpen = nullptr;
    config.encoder.v_table.onClose = nullptr;
    config.encoder.v_table.onWrite = nullptr;
    config.encoder.user_data = nullptr;

    return config;
}

AM_API_PUBLIC void am_codec_register(const am_codec_config* config)
{
    if (g_registered_codecs.contains(config->name))
        return;

    g_registered_codecs.emplace(config->name, ampoolnew(MemoryPoolKind::Codec, CCodec, *config));
}

AM_API_PUBLIC void am_codec_unregister(const char* name)
{
    g_registered_codecs.erase(name);
}

AM_API_PUBLIC am_codec_handle am_codec_find(const char* name)
{
    return reinterpret_cast<am_codec_handle>(Codec::Find(name));
}

AM_API_PUBLIC am_bool am_codec_can_handle_file(am_codec_handle codec, am_filesystem_file_handle file)
{
    if (codec == nullptr || file == nullptr)
        return AM_FALSE;

    return AM_BOOL_TO_BOOL(reinterpret_cast<Codec*>(codec)->CanHandleFile(std::shared_ptr<File>(reinterpret_cast<File*>(file))));
}

AM_API_PUBLIC am_codec_decoder_handle am_codec_decoder_create(const char* name)
{
    auto* codec = Codec::Find(name);
    if (codec == nullptr)
        return nullptr;

    return reinterpret_cast<am_codec_decoder_handle>(codec->CreateDecoder());
}

AM_API_PUBLIC void am_codec_decoder_destroy(const char* name, am_codec_decoder_handle handle)
{
    if (handle == nullptr)
        return;

    auto* codec = Codec::Find(name);
    if (codec == nullptr)
        return;

    codec->DestroyDecoder(reinterpret_cast<Codec::Decoder*>(handle));
}

AM_API_PUBLIC am_bool am_codec_decoder_open(am_codec_decoder_handle handle, am_filesystem_file_handle file)
{
    if (handle == nullptr || file == nullptr)
        return AM_FALSE;

    auto* codec = reinterpret_cast<Codec::Decoder*>(handle);
    return BOOL_TO_AM_BOOL(codec->Open(std::shared_ptr<File>(reinterpret_cast<File*>(file))));
}

AM_API_PUBLIC am_bool am_codec_decoder_close(am_codec_decoder_handle handle)
{
    if (handle == nullptr)
        return AM_FALSE;

    auto* codec = reinterpret_cast<Codec::Decoder*>(handle);
    return BOOL_TO_AM_BOOL(codec->Close());
}

AM_API_PUBLIC am_uint64 am_codec_decoder_load(am_codec_decoder_handle handle, am_voidptr out)
{
    if (handle == nullptr || out == nullptr)
        return 0;

    auto* codec = reinterpret_cast<Codec::Decoder*>(handle);
    return codec->Load(out);
}

AM_API_PUBLIC am_uint64 am_codec_decoder_stream(am_codec_decoder_handle handle, am_voidptr out, am_uint64 offset, am_uint64 length)
{
    if (handle == nullptr || out == nullptr)
        return 0;

    auto* codec = reinterpret_cast<Codec::Decoder*>(handle);
    return codec->Stream(out, offset, length);
}

AM_API_PUBLIC am_uint64 am_codec_decoder_seek(am_codec_decoder_handle handle, am_uint64 offset)
{
    if (handle == nullptr)
        return 0;

    auto* codec = reinterpret_cast<Codec::Decoder*>(handle);
    return codec->Seek(offset);
}
}
