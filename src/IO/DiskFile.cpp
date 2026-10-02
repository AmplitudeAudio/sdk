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
    namespace
    {
        // 64-bit variants of ftell/fseek: long is 32-bit on Windows, which caps plain ftell/fseek at 2 GB.
        AmInt64 TellFile(AmFileHandle file)
        {
#if AM_PLATFORM_WIN
            return _ftelli64(file);
#else
            return ftello(file);
#endif
        }

        int SeekFile(AmFileHandle file, AmInt64 offset, int origin)
        {
#if AM_PLATFORM_WIN
            return _fseeki64(file, offset, origin);
#else
            return fseeko(file, static_cast<off_t>(offset), origin);
#endif
        }
    } // namespace

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

    bool DiskFile::Eof() const
    {
        if (m_fileHandle == nullptr)
            return true;

        if (feof(m_fileHandle) != 0)
            return true;

        const AmInt64 pos = TellFile(m_fileHandle);
        if (pos < 0)
            return true;

        const bool value = fgetc(m_fileHandle) == EOF;
        SeekFile(m_fileHandle, pos, SEEK_SET);
        return value;
    }

    AmSize DiskFile::Read(AmUInt8Buffer dst, AmSize bytes) const
    {
        if (m_fileHandle == nullptr || bytes == 0 || dst == nullptr)
            return 0;

        return fread(dst, 1, bytes, m_fileHandle);
    }

    AmSize DiskFile::Write(AmConstUInt8Buffer src, AmSize bytes)
    {
        if (m_fileHandle == nullptr || bytes == 0 || src == nullptr)
            return 0;

        return fwrite(src, 1, bytes, m_fileHandle);
    }

    AmSize DiskFile::Length() const
    {
        if (m_fileHandle == nullptr)
            return 0;

        const AmInt64 pos = TellFile(m_fileHandle);
        if (pos < 0)
            return 0;

        if (SeekFile(m_fileHandle, 0, SEEK_END) != 0)
            return 0;

        const AmInt64 len = TellFile(m_fileHandle);
        SeekFile(m_fileHandle, pos, SEEK_SET);

        return len < 0 ? 0 : static_cast<AmSize>(len);
    }

    void DiskFile::Seek(AmInt64 offset, eFileSeekOrigin origin)
    {
        if (m_fileHandle == nullptr)
            return;

        SeekFile(m_fileHandle, offset, origin);
    }

    AmSize DiskFile::Position() const
    {
        if (m_fileHandle == nullptr)
            return 0;

        const AmInt64 pos = TellFile(m_fileHandle);
        return pos < 0 ? 0 : static_cast<AmSize>(pos);
    }

    AmVoidPtr DiskFile::GetPtr() const
    {
        return m_fileHandle;
    }

    bool DiskFile::IsValid() const
    {
        return m_fileHandle != nullptr;
    }

    void DiskFile::Close()
    {
        if (m_fileHandle == nullptr)
            return;

        fclose(m_fileHandle);
        m_fileHandle = nullptr;
    }

    AmResult DiskFile::Open(const std::filesystem::path& filePath, eFileOpenMode mode, eFileOpenKind kind)
    {
        if (filePath.empty())
            return eErrorCode_InvalidParameter;

        // fopen() succeeds on a directory on POSIX, but the resulting handle is not a readable file:
        // seeking to its end reports a length of INT64_MAX. Reject directories here so callers never
        // see a "valid" file whose Length() is nonsense.
        std::error_code error;
        if (std::filesystem::is_directory(filePath, error))
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

#if AM_PLATFORM_WIN
        _wfopen_s(&m_fileHandle, filePath.c_str(), op.c_str());
#else
        m_fileHandle = fopen(filePath.c_str(), op.c_str());
#endif

        if (!m_fileHandle)
            return eErrorCode_FileNotFound;

        m_filePath = filePath;

        return eErrorCode_Success;
    }
} // namespace SparkyStudios::Audio::Amplitude
