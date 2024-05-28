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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

#include <SparkyStudios/Audio/Amplitude/IO/DiskFile.h>
#include <SparkyStudios/Audio/Amplitude/IO/MemoryFile.h>

namespace SparkyStudios::Audio::Amplitude
{

    MemoryFile::MemoryFile()
        : m_dataPtr(nullptr)
        , m_dataSize(0)
        , m_offset(0)
        , m_dataOwned(false)
    {}

    MemoryFile::MemoryFile(AmUInt8Buffer buffer, AmSize size, bool copy, bool takeOwnership)
        : MemoryFile()
    {
        OpenMem(buffer, size, copy, takeOwnership);
    }

    MemoryFile::~MemoryFile()
    {
        if (m_dataOwned && m_dataPtr != nullptr)
            ampoolfree(MemoryPoolKind::IO, m_dataPtr);

        m_dataPtr = nullptr;
        m_dataSize = 0;
        m_offset = 0;
        m_dataOwned = false;
    }

    AmOsString MemoryFile::GetPath() const
    {
        return AM_OS_STRING("");
    }

    bool MemoryFile::Eof()
    {
        return m_offset >= m_dataSize;
    }

    AmSize MemoryFile::Read(AmUInt8Buffer dst, AmSize bytes)
    {
        if (m_offset + bytes >= m_dataSize)
            bytes = m_dataSize - m_offset;

        std::memcpy(dst, m_dataPtr + m_offset, bytes);
        m_offset += bytes;

        return bytes;
    }

    AmSize MemoryFile::Write(AmConstUInt8Buffer src, AmSize bytes)
    {
        const auto bytesToWrite = std::min(bytes, m_dataSize - m_offset);

        std::memcpy(m_dataPtr + m_offset, src, bytesToWrite);
        m_offset += bytesToWrite;

        return bytesToWrite;
    }

    AmSize MemoryFile::Length()
    {
        return m_dataSize;
    }

    void MemoryFile::Seek(AmSize offset, FileSeekOrigin origin)
    {
        if (origin == eFSO_START)
            m_offset = offset;
        else if (origin == eFSO_CURRENT)
            m_offset += offset;
        else if (origin == eFSO_END)
            m_offset = m_dataSize - offset;

        if (m_offset > m_dataSize - 1)
            m_offset = m_dataSize - 1;
    }

    AmSize MemoryFile::Position()
    {
        return m_offset;
    }

    AmVoidPtr MemoryFile::GetPtr()
    {
        return m_dataPtr;
    }

    bool MemoryFile::IsValid() const
    {
        return m_dataPtr != nullptr;
    }

    AmResult MemoryFile::OpenMem(AmConstUInt8Buffer buffer, AmSize size, bool copy, bool takeOwnership)
    {
        if (buffer == nullptr || size == 0)
            return AM_ERROR_INVALID_PARAMETER;

        if (m_dataOwned && m_dataPtr != nullptr)
            ampoolfree(MemoryPoolKind::IO, m_dataPtr);

        m_dataPtr = nullptr;
        m_offset = 0;

        m_dataSize = size;

        if (copy)
        {
            m_dataOwned = true;
            m_dataPtr = static_cast<AmUInt8Buffer>(ampoolmalloc(MemoryPoolKind::IO, size));

            if (m_dataPtr == nullptr)
                return AM_ERROR_OUT_OF_MEMORY;

            std::memcpy(m_dataPtr, buffer, size);
            return AM_ERROR_NO_ERROR;
        }

        m_dataPtr = const_cast<AmUInt8Buffer>(buffer);
        m_dataOwned = takeOwnership;

        return AM_ERROR_NO_ERROR;
    }

    AmResult MemoryFile::OpenToMem(const std::filesystem::path& fileName)
    {
        if (fileName.empty())
            return AM_ERROR_INVALID_PARAMETER;

        if (m_dataOwned && m_dataPtr != nullptr)
            ampoolfree(MemoryPoolKind::IO, m_dataPtr);

        m_dataPtr = nullptr;
        m_offset = 0;

        DiskFile df;
        if (const AmResult res = df.Open(fileName); res != AM_ERROR_NO_ERROR)
            return res;

        m_dataSize = df.Length();
        m_dataPtr = static_cast<AmUInt8Buffer>(ampoolmalloc(MemoryPoolKind::IO, m_dataSize));

        if (m_dataPtr == nullptr)
            return AM_ERROR_OUT_OF_MEMORY;

        df.Read(m_dataPtr, m_dataSize);
        m_dataOwned = true;

        df.Close();

        return AM_ERROR_NO_ERROR;
    }

    AmResult MemoryFile::OpenFileToMem(File* file)
    {
        if (!file)
            return AM_ERROR_INVALID_PARAMETER;

        if (m_dataOwned && m_dataPtr != nullptr)
            ampoolfree(MemoryPoolKind::IO, m_dataPtr);

        m_dataPtr = nullptr;
        m_offset = file->Position();

        m_dataSize = file->Length();
        m_dataPtr = static_cast<AmUInt8Buffer>(ampoolmalloc(MemoryPoolKind::IO, m_dataSize));

        if (m_dataPtr == nullptr)
            return AM_ERROR_OUT_OF_MEMORY;

        file->Seek(0, eFSO_START);
        file->Read(m_dataPtr, m_dataSize);
        file->Seek(m_offset, eFSO_START);

        m_dataOwned = true;

        return AM_ERROR_NO_ERROR;
    }
}