// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

#include <SparkyStudios/Audio/Amplitude/IO/DiskFile.h>
#include <SparkyStudios/Audio/Amplitude/IO/MappedFile.h>

#if AM_PLATFORM_WIN
// Windows.h already included via Config.h
#elif AM_PLATFORM_LINUX || AM_PLATFORM_APPLE
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace SparkyStudios::Audio::Amplitude
{
    namespace
    {
        constexpr AmSize kMmapThreshold = 1024 * 1024; // 1 MB

        struct MappingResult
        {
            AmUInt8Buffer data;
            AmSize size;
            AmVoidPtr handle;
            bool success;
        };

        MappingResult CreateMapping(const std::filesystem::path& path, AmSize fileSize)
        {
            MappingResult result{};
            result.data = nullptr;
            result.size = fileSize;
            result.handle = nullptr;
            result.success = false;

#if AM_PLATFORM_WIN
            HANDLE fileHandle =
                CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

            if (fileHandle == INVALID_HANDLE_VALUE)
                return result;

            HANDLE mappingHandle = CreateFileMappingW(fileHandle, nullptr, PAGE_READONLY, 0, 0, nullptr);
            if (mappingHandle == nullptr)
            {
                CloseHandle(fileHandle);
                return result;
            }

            result.data = static_cast<AmUInt8Buffer>(MapViewOfFile(mappingHandle, FILE_MAP_READ, 0, 0, 0));
            if (result.data == nullptr)
            {
                CloseHandle(mappingHandle);
                CloseHandle(fileHandle);
                return result;
            }

            // Store both handles so Close() can release them
            auto* handles = static_cast<HANDLE*>(ampoolmalloc(eMemoryPoolKind_IO, sizeof(HANDLE) * 2));
            handles[0] = fileHandle;
            handles[1] = mappingHandle;
            result.handle = handles;
            result.success = true;

#elif AM_PLATFORM_LINUX || AM_PLATFORM_APPLE
            int fd = open(path.c_str(), O_RDONLY);
            if (fd == -1)
                return result;

            result.data = static_cast<AmUInt8Buffer>(mmap(nullptr, fileSize, PROT_READ, MAP_PRIVATE, fd, 0));

            // File descriptor can be closed immediately after mmap; the mapping holds its own reference.
            close(fd);

            if (result.data == MAP_FAILED)
            {
                result.data = nullptr;
                return result;
            }

            result.success = true;
#else
            AM_UNUSED(path);
            AM_UNUSED(fileSize);
#endif

            return result;
        }

        void DestroyMapping(AmUInt8Buffer data, AmSize size, AmVoidPtr handle)
        {
            if (data == nullptr)
                return;

#if AM_PLATFORM_WIN
            UnmapViewOfFile(data);

            if (handle != nullptr)
            {
                auto* handles = static_cast<HANDLE*>(handle);
                CloseHandle(handles[1]); // mapping handle
                CloseHandle(handles[0]); // file handle
                ampoolfree(eMemoryPoolKind_IO, handles);
            }
#elif AM_PLATFORM_LINUX || AM_PLATFORM_APPLE
            munmap(data, size);
            AM_UNUSED(handle);
#else
            AM_UNUSED(data);
            AM_UNUSED(size);
            AM_UNUSED(handle);
#endif
        }
    } // anonymous namespace

    MappedFile::MappedFile()
        : m_dataPtr(nullptr)
        , m_dataSize(0)
        , m_offset(0)
        , m_isMapped(false)
        , m_dataOwned(false)
        , m_mapHandle(nullptr)
    {}

    MappedFile::MappedFile(const std::filesystem::path& fileName)
        : MappedFile()
    {
        Open(fileName);
    }

    MappedFile::~MappedFile()
    {
        Close();
    }

    AmOsString MappedFile::GetPath() const
    {
        return m_filePath.c_str();
    }

    bool MappedFile::Eof() const
    {
        return m_offset >= m_dataSize;
    }

    AmSize MappedFile::Read(AmUInt8Buffer dst, AmSize bytes) const
    {
        if (m_dataPtr == nullptr)
            return 0;

        if (m_offset + bytes >= m_dataSize)
            bytes = m_dataSize - m_offset;

        std::memcpy(dst, m_dataPtr + m_offset, bytes);
        m_offset += bytes;

        return bytes;
    }

    AmSize MappedFile::Write(AmConstUInt8Buffer src, AmSize bytes)
    {
        AM_UNUSED(src);
        AM_UNUSED(bytes);
        return 0;
    }

    AmSize MappedFile::Length() const
    {
        return m_dataSize;
    }

    void MappedFile::Seek(AmInt64 offset, eFileSeekOrigin origin)
    {
        if (origin == eFileSeekOrigin_Start)
            m_offset = offset;
        else if (origin == eFileSeekOrigin_Current)
            m_offset += offset;
        else if (origin == eFileSeekOrigin_End)
            m_offset = m_dataSize + offset;

        if (m_dataSize > 0 && m_offset > m_dataSize - 1)
            m_offset = m_dataSize - 1;
    }

    AmSize MappedFile::Position() const
    {
        return m_offset;
    }

    AmVoidPtr MappedFile::GetPtr() const
    {
        return m_dataPtr;
    }

    bool MappedFile::IsValid() const
    {
        return m_dataPtr != nullptr;
    }

    void MappedFile::Close()
    {
        if (m_dataPtr == nullptr)
            return;

        if (m_isMapped)
            DestroyMapping(m_dataPtr, m_dataSize, m_mapHandle);
        else if (m_dataOwned)
            ampoolfree(eMemoryPoolKind_IO, m_dataPtr);

        m_dataPtr = nullptr;
        m_dataSize = 0;
        m_offset = 0;
        m_isMapped = false;
        m_dataOwned = false;
        m_mapHandle = nullptr;
        m_filePath.clear();
    }

    AmResult MappedFile::Open(const std::filesystem::path& filePath)
    {
        if (filePath.empty())
            return eErrorCode_InvalidParameter;

        Close();

        // Open file to determine size
        DiskFile df;
        if (const AmResult res = df.Open(filePath); res != eErrorCode_Success)
            return res;

        const AmSize fileSize = df.Length();

        m_filePath = filePath;
        m_dataSize = fileSize;

        // Empty files need no mapping or allocation
        if (fileSize == 0)
        {
            df.Close();
            m_dataPtr = nullptr;
            m_isMapped = false;
            m_dataOwned = false;
            return eErrorCode_Success;
        }

        // Try memory mapping for files >= threshold
        if (fileSize >= kMmapThreshold)
        {
            df.Close();

            MappingResult mapping = CreateMapping(filePath, fileSize);
            if (mapping.success)
            {
                m_dataPtr = mapping.data;
                m_dataSize = mapping.size;
                m_mapHandle = mapping.handle;
                m_isMapped = true;
                m_dataOwned = false;
                return eErrorCode_Success;
            }

            // Mapping failed; reopen for heap fallback
            if (const AmResult res = df.Open(filePath); res != eErrorCode_Success)
                return res;
        }

        // Heap fallback
        m_dataPtr = static_cast<AmUInt8Buffer>(ampoolmalloc(eMemoryPoolKind_IO, m_dataSize));
        if (m_dataPtr == nullptr)
        {
            df.Close();
            return eErrorCode_OutOfMemory;
        }

        df.Seek(0, eFileSeekOrigin_Start);
        df.Read(m_dataPtr, m_dataSize);
        df.Close();

        m_isMapped = false;
        m_dataOwned = true;

        return eErrorCode_Success;
    }

    bool MappedFile::IsMapped() const
    {
        return m_isMapped;
    }
} // namespace SparkyStudios::Audio::Amplitude
