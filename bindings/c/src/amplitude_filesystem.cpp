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

#include <c/include/amplitude_filesystem.h>

#include "amplitude_internals.h"

class CFile final : public File
{
public:
    explicit CFile(am_file_vtable* v_table, am_voidptr user_data = nullptr)
        : File()
        , _v_table(v_table)
        , _user_data(user_data)
    {
        if (_v_table->create)
            _v_table->create(_user_data);
    }

    ~CFile() override
    {
        if (_v_table->destroy)
            _v_table->destroy(_user_data);

        _v_table = nullptr;
        _user_data = nullptr;
    }

    [[nodiscard]] AmOsString GetPath() const override
    {
        return _v_table->get_path(_user_data);
    }

    bool Eof() override
    {
        return AM_BOOL_TO_BOOL(_v_table->eof(_user_data));
    }

    AmSize Read(AmUInt8Buffer dst, AmSize bytes) override
    {
        return _v_table->read(_user_data, dst, bytes);
    }

    AmSize Write(AmConstUInt8Buffer src, AmSize bytes) override
    {
        return _v_table->write(_user_data, src, bytes);
    }

    AmSize Length() override
    {
        return _v_table->length(_user_data);
    }

    void Seek(AmSize offset, FileSeekOrigin origin) override
    {
        _v_table->seek(_user_data, offset, static_cast<am_file_seek_origin>(origin));
    }

    AmSize Position() override
    {
        return _v_table->position(_user_data);
    }

    [[nodiscard]] bool IsValid() const override
    {
        return AM_BOOL_TO_BOOL(_v_table->is_valid(_user_data));
    }

private:
    am_file_vtable* _v_table;
    am_voidptr _user_data;
};

class CFileSystem final : public FileSystem
{
public:
    explicit CFileSystem(am_filesystem_vtable* v_table, am_voidptr user_data = nullptr)
        : FileSystem()
        , _v_table(v_table)
        , _user_data(user_data)
    {
        if (_v_table->create)
            _v_table->create(_user_data);
    }

    ~CFileSystem() override
    {
        if (_v_table->destroy)
            _v_table->destroy(_user_data);

        _v_table = nullptr;
        _user_data = nullptr;
    }

    void SetBasePath(const AmOsString& basePath) override
    {
        _v_table->set_base_path(_user_data, basePath.c_str());
    }

    [[nodiscard]] AmOsString ResolvePath(const AmOsString& path) const override
    {
        return _v_table->resolve_path(_user_data, path.c_str());
    }

    [[nodiscard]] bool Exists(const AmOsString& path) const override
    {
        return BOOL_TO_AM_BOOL(_v_table->exists(_user_data, path.c_str()));
    }

    [[nodiscard]] bool IsDirectory(const AmOsString& path) const override
    {
        return BOOL_TO_AM_BOOL(_v_table->is_directory(_user_data, path.c_str()));
    }

    [[nodiscard]] AmOsString Join(const std::vector<AmOsString>& parts) const override
    {
        const auto** c_parts = static_cast<const am_oschar**>(ammalloc(sizeof(am_oschar*) * parts.size()));
        for (size_t i = 0; i < parts.size(); i++)
            c_parts[i] = parts[i].c_str();

        const auto result = _v_table->join(_user_data, c_parts, parts.size());
        amfree(c_parts);

        return result;
    }

    [[nodiscard]] std::shared_ptr<File> OpenFile(const AmOsString& path) const override
    {
        const auto file = _v_table->open_file(_user_data, path.c_str());

        if (file.type == am_file_type_custom)
            return std::shared_ptr<CFile>(static_cast<CFile*>(file.handle));

        if (file.type == am_file_type_disk)
            return std::shared_ptr<DiskFile>(static_cast<DiskFile*>(file.handle));

        if (file.type == am_file_type_memory)
            return std::shared_ptr<MemoryFile>(static_cast<MemoryFile*>(file.handle));

        return nullptr;
    }

    void StartOpenFileSystem() override
    {
        _v_table->start_open_filesystem(_user_data);
    }

    bool TryFinalizeOpenFileSystem() override
    {
        return BOOL_TO_AM_BOOL(_v_table->try_finalize_open_filesystem(_user_data));
    }

    void StartCloseFileSystem() override
    {
        _v_table->start_close_filesystem(_user_data);
    }

    bool TryFinalizeCloseFileSystem() override
    {
        return BOOL_TO_AM_BOOL(_v_table->try_finalize_close_filesystem(_user_data));
    }

private:
    am_filesystem_vtable* _v_table;
    am_voidptr _user_data;
};

static std::map<File*, std::shared_ptr<File>> g_files = {};

#ifdef __cplusplus
extern "C" {
#endif

AM_API_PUBLIC am_file_config am_file_config_init_custom()
{
    return { am_file_type_custom, nullptr, nullptr };
}

AM_API_PUBLIC am_file_config am_file_config_init_disk()
{
    return { am_file_type_disk, nullptr, nullptr };
}

AM_API_PUBLIC am_file_config am_file_config_init_memory()
{
    return { am_file_type_memory, nullptr, nullptr };
}

AM_API_PUBLIC am_file_handle am_file_create(const am_file_config* config)
{
    if (config->type == am_file_type_custom)
        return { am_file_type_custom, ampoolnew(MemoryPoolKind::IO, CFile, config->v_table, config->user_data) };

    if (config->type == am_file_type_disk)
        return { am_file_type_disk, ampoolnew(MemoryPoolKind::IO, DiskFile) };

    if (config->type == am_file_type_memory)
        return { am_file_type_memory, ampoolnew(MemoryPoolKind::IO, MemoryFile) };

    return { am_file_type_unknown, nullptr };
}

AM_API_PUBLIC void am_file_destroy(am_file_handle handle)
{
    if (handle.type == am_file_type_custom)
    {
        ampooldelete(MemoryPoolKind::IO, CFile, static_cast<CFile*>(handle.handle));
    }
    else if (handle.type == am_file_type_disk)
    {
        ampooldelete(MemoryPoolKind::IO, DiskFile, static_cast<DiskFile*>(handle.handle));
    }
    else if (handle.type == am_file_type_memory)
    {
        ampooldelete(MemoryPoolKind::IO, MemoryFile, static_cast<MemoryFile*>(handle.handle));
    }
}

AM_API_PUBLIC const am_oschar* am_file_get_path(am_file_handle handle)
{
    return am_allocate_osstring(static_cast<File*>(handle.handle)->GetPath().c_str());
}

AM_API_PUBLIC am_uint8 am_file_read8(am_file_handle handle)
{
    return static_cast<File*>(handle.handle)->Read8();
}

AM_API_PUBLIC am_uint16 am_file_read16(am_file_handle handle)
{
    return static_cast<File*>(handle.handle)->Read16();
}

AM_API_PUBLIC am_uint32 am_file_read32(am_file_handle handle)
{
    return static_cast<File*>(handle.handle)->Read32();
}

AM_API_PUBLIC am_bool am_file_eof(am_file_handle file)
{
    return BOOL_TO_AM_BOOL(static_cast<File*>(file.handle)->Eof());
}

AM_API_PUBLIC am_size am_file_read(am_file_handle file, am_uint8* dst, am_size bytes)
{
    return static_cast<File*>(file.handle)->Read(dst, bytes);
}

AM_API_PUBLIC am_size am_file_write(am_file_handle file, const am_uint8* src, am_size bytes)
{
    return static_cast<File*>(file.handle)->Write(src, bytes);
}

AM_API_PUBLIC am_size am_file_length(am_file_handle file)
{
    return static_cast<File*>(file.handle)->Length();
}

AM_API_PUBLIC void am_file_seek(am_file_handle file, am_size offset, am_file_seek_origin origin)
{
    static_cast<File*>(file.handle)->Seek(offset, static_cast<FileSeekOrigin>(origin));
}

AM_API_PUBLIC am_size am_file_position(am_file_handle file)
{
    return static_cast<File*>(file.handle)->Position();
}

AM_API_PUBLIC am_bool am_file_is_valid(am_file_handle handle)
{
    return BOOL_TO_AM_BOOL(static_cast<File*>(handle.handle)->IsValid());
}

AM_API_PUBLIC am_filesystem_config am_filesystem_config_init_custom()
{
    return { am_filesystem_type_custom, nullptr, nullptr };
}

AM_API_PUBLIC am_filesystem_config am_filesystem_config_init_disk()
{
    return { am_filesystem_type_disk, nullptr, nullptr };
}

AM_API_PUBLIC am_filesystem_handle am_filesystem_create(const am_filesystem_config* config)
{
    if (config->type == am_filesystem_type_custom)
        return { am_filesystem_type_custom, ampoolnew(MemoryPoolKind::IO, CFileSystem, config->v_table, config->user_data) };

    if (config->type == am_filesystem_type_disk)
        return { am_filesystem_type_disk, ampoolnew(MemoryPoolKind::IO, DiskFileSystem) };

    return { am_filesystem_type_unknown, nullptr };
}

AM_API_PUBLIC void am_filesystem_destroy(am_filesystem_handle filesystem)
{
    if (filesystem.type == am_filesystem_type_custom)
    {
        ampooldelete(MemoryPoolKind::IO, CFileSystem, static_cast<CFileSystem*>(filesystem.handle));
    }
    else if (filesystem.type == am_filesystem_type_disk)
    {
        ampooldelete(MemoryPoolKind::IO, DiskFileSystem, static_cast<DiskFileSystem*>(filesystem.handle));
    }
}

AM_API_PUBLIC void am_filesystem_set_base_path(am_filesystem_handle filesystem, const am_oschar* base_path)
{
    static_cast<FileSystem*>(filesystem.handle)->SetBasePath(base_path);
}

AM_API_PUBLIC const am_oschar* am_filesystem_resolve_path(am_filesystem_handle filesystem, const am_oschar* path)
{
    return am_allocate_osstring(static_cast<FileSystem*>(filesystem.handle)->ResolvePath(path).c_str());
}

AM_API_PUBLIC am_bool am_filesystem_exists(am_filesystem_handle filesystem, const am_oschar* path)
{
    return BOOL_TO_AM_BOOL(static_cast<FileSystem*>(filesystem.handle)->Exists(path));
}

AM_API_PUBLIC am_bool am_filesystem_is_directory(am_filesystem_handle filesystem, const am_oschar* path)
{
    return BOOL_TO_AM_BOOL(static_cast<FileSystem*>(filesystem.handle)->IsDirectory(path));
}

AM_API_PUBLIC const am_oschar* am_filesystem_join(am_filesystem_handle filesystem, const am_oschar** parts, am_size count)
{
    std::vector<AmOsString> cpp_parts(count);
    for (am_size i = 0; i < count; i++)
        cpp_parts[i] = parts[i];

    return am_allocate_osstring(static_cast<FileSystem*>(filesystem.handle)->Join(cpp_parts).c_str());
}

AM_API_PUBLIC am_file_handle am_filesystem_open_file(am_filesystem_handle filesystem, const am_oschar* path)
{
    const auto file = static_cast<FileSystem*>(filesystem.handle)->OpenFile(path);
    g_files.emplace(file.get(), file);

    return { filesystem.type == am_filesystem_type_custom ? am_file_type_custom : am_file_type_disk, file.get() };
}

AM_API_PUBLIC void am_filesystem_start_open(am_filesystem_handle filesystem)
{
    static_cast<FileSystem*>(filesystem.handle)->StartOpenFileSystem();
}

AM_API_PUBLIC am_bool am_filesystem_try_finalize_open(am_filesystem_handle filesystem)
{
    return BOOL_TO_AM_BOOL(static_cast<FileSystem*>(filesystem.handle)->TryFinalizeOpenFileSystem());
}

AM_API_PUBLIC void am_filesystem_start_close(am_filesystem_handle filesystem)
{
    static_cast<FileSystem*>(filesystem.handle)->StartCloseFileSystem();
}

AM_API_PUBLIC am_bool am_filesystem_try_finalize_close(am_filesystem_handle filesystem)
{
    return BOOL_TO_AM_BOOL(static_cast<FileSystem*>(filesystem.handle)->TryFinalizeCloseFileSystem());
}

#ifdef __cplusplus
} // extern "C"
#endif