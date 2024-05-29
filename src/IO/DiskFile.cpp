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

#include <SparkyStudios/Audio/Amplitude/IO/DiskFile.h>

namespace SparkyStudios::Audio::Amplitude
{
    DiskFile::DiskFile()
        : DiskFile(nullptr)
    {}

    DiskFile::DiskFile(AmFileHandle fp)
        : m_fileHandle(fp)
    {}

    DiskFile::DiskFile(const std::filesystem::path& fileName, FileOpenMode mode, FileOpenKind kind)
        : DiskFile()
    {
        Open(fileName, mode, kind);
    }

    DiskFile::~DiskFile()
    {
        Close();
    }

    AmOsString DiskFile::GetPath() const
    {
        return m_filePath.c_str();
    }

    bool DiskFile::Eof()
    {
        return feof(m_fileHandle) != 0;
    }

    AmSize DiskFile::Read(AmUInt8Buffer dst, AmSize bytes)
    {
        return fread(dst, 1, bytes, m_fileHandle);
    }

    AmSize DiskFile::Write(AmConstUInt8Buffer src, AmSize bytes)
    {
        return fwrite(src, 1, bytes, m_fileHandle);
    }

    AmSize DiskFile::Length()
    {
        if (!m_fileHandle)
            return 0;

        const long pos = ftell(m_fileHandle);
        fseek(m_fileHandle, 0, SEEK_END);
        const auto len = static_cast<AmUInt32>(ftell(m_fileHandle));
        fseek(m_fileHandle, pos, SEEK_SET);

        return len;
    }

    void DiskFile::Seek(AmSize offset, FileSeekOrigin origin)
    {
        fseek(m_fileHandle, offset, origin);
    }

    AmSize DiskFile::Position()
    {
        return static_cast<AmSize>(ftell(m_fileHandle));
    }

    AmVoidPtr DiskFile::GetPtr()
    {
        return m_fileHandle;
    }

    bool DiskFile::IsValid() const
    {
        return m_fileHandle != nullptr;
    }

    AmResult DiskFile::Open(const std::filesystem::path& filePath, FileOpenMode mode, FileOpenKind kind)
    {
        if (filePath.empty())
            return AM_ERROR_INVALID_PARAMETER;

        AmOsString op{};

        switch (mode)
        {
        case eFOM_READ:
            op = kind == eFOK_TEXT ? AM_OS_STRING("r") : AM_OS_STRING("rb");
            break;
        case eFOM_WRITE:
            op = kind == eFOK_TEXT ? AM_OS_STRING("w") : AM_OS_STRING("wb");
            break;
        case eFOM_APPEND:
            op = kind == eFOK_TEXT ? AM_OS_STRING("a") : AM_OS_STRING("ab");
            break;
        case eFOM_READWRITE:
            op = kind == eFOK_TEXT ? AM_OS_STRING("w+") : AM_OS_STRING("wb+");
            break;
        case eFOM_READAPPEND:
            op = kind == eFOK_TEXT ? AM_OS_STRING("a+") : AM_OS_STRING("ab+");
            break;
        }

#if defined(AM_WINDOWS_VERSION)
        _wfopen_s(&m_fileHandle, filePath.c_str(), op.c_str());
#else
        m_fileHandle = fopen(filePath.c_str(), op.c_str());
#endif

        if (!m_fileHandle)
            return AM_ERROR_FILE_NOT_FOUND;

        m_filePath = filePath;

        return AM_ERROR_NO_ERROR;
    }

    void DiskFile::Close()
    {
        if (m_fileHandle == nullptr)
            return;

        fclose(m_fileHandle);
        m_fileHandle = nullptr;
    }
}