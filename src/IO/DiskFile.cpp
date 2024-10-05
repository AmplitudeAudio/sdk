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

    DiskFile::DiskFile(const std::filesystem::path& fileName, eFileOpenMode mode, eFileOpenKind kind)
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
        const long pos = ftell(m_fileHandle);
        const bool value = fgetc(m_fileHandle) == EOF;
        fseek(m_fileHandle, pos, SEEK_SET);
        return value;
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

    void DiskFile::Seek(AmInt64 offset, eFileSeekOrigin origin)
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

    AmResult DiskFile::Open(const std::filesystem::path& filePath, eFileOpenMode mode, eFileOpenKind kind)
    {
        if (filePath.empty())
            return eErrorCode_InvalidParameter;

        AmOsString op{};

        switch (mode)
        {
        case eFileOpenMode_Read:
            op = kind == eFileOpenKind_Text ? AM_OS_STRING("r") : AM_OS_STRING("rb");
            break;
        case eFileOpenMode_Write:
            op = kind == eFileOpenKind_Text ? AM_OS_STRING("w") : AM_OS_STRING("wb");
            break;
        case eFileOpenMode_Append:
            op = kind == eFileOpenKind_Text ? AM_OS_STRING("a") : AM_OS_STRING("ab");
            break;
        case eFileOpenMode_ReadWrite:
            op = kind == eFileOpenKind_Text ? AM_OS_STRING("w+") : AM_OS_STRING("wb+");
            break;
        case eFileOpenMode_ReadAppend:
            op = kind == eFileOpenKind_Text ? AM_OS_STRING("a+") : AM_OS_STRING("ab+");
            break;
        }

#if defined(AM_WINDOWS_VERSION)
        _wfopen_s(&m_fileHandle, filePath.c_str(), op.c_str());
#else
        m_fileHandle = fopen(filePath.c_str(), op.c_str());
#endif

        if (!m_fileHandle)
            return eErrorCode_FileNotFound;

        m_filePath = filePath;

        return eErrorCode_Success;
    }

    void DiskFile::Close()
    {
        if (m_fileHandle == nullptr)
            return;

        fclose(m_fileHandle);
        m_fileHandle = nullptr;
    }
} // namespace SparkyStudios::Audio::Amplitude