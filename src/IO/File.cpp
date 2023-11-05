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

#include <SparkyStudios/Audio/Amplitude/IO/File.h>

namespace SparkyStudios::Audio::Amplitude
{
    AmUInt8 File::Read8()
    {
        AmUInt8 d = 0;
        Read((AmUInt8Buffer)&d, 1);
        return d;
    }

    AmUInt16 File::Read16()
    {
        AmUInt16 d = 0;
        Read((AmUInt8Buffer)&d, 2);
        return d;
    }

    AmUInt32 File::Read32()
    {
        AmUInt32 d = 0;
        Read((AmUInt8Buffer)&d, 4);
        return d;
    }

    DiskFile::DiskFile(AmFileHandle fp)
        : mFileHandle(fp)
    {}

    AmSize DiskFile::Read(AmUInt8Buffer dst, AmSize bytes)
    {
        return (AmSize)fread(dst, 1, bytes, mFileHandle);
    }

    AmSize DiskFile::Write(AmConstUInt8Buffer src, AmSize bytes)
    {
        return (AmSize)fwrite(src, 1, bytes, mFileHandle);
    }

    AmSize DiskFile::Length()
    {
        if (!mFileHandle)
            return 0;

        long pos = ftell(mFileHandle);
        fseek(mFileHandle, 0, SEEK_END);
        auto len = (AmUInt32)ftell(mFileHandle);
        fseek(mFileHandle, pos, SEEK_SET);

        return len;
    }

    void DiskFile::Seek(AmInt32 offset)
    {
        fseek(mFileHandle, offset, SEEK_SET);
    }

    AmSize DiskFile::Pos()
    {
        return (AmSize)ftell(mFileHandle);
    }

    AmFileHandle DiskFile::GetFilePtr()
    {
        return mFileHandle;
    }

    DiskFile::~DiskFile()
    {
        if (mFileHandle)
            fclose(mFileHandle);
    }

    DiskFile::DiskFile()
    {
        mFileHandle = nullptr;
    }

    AmResult DiskFile::Open(const AmOsString& fileName, FileOpenMode mode, FileOpenKind kind)
    {
        if (fileName.empty())
            return AM_ERROR_INVALID_PARAMETER;

        AmOsString op{};

        switch (mode)
        {
            case eFOM_READ:
                op = kind == eFOK_TEXT ? AM_OS_STRING("r") : AM_OS_STRING("rb");
                break;
            case eFOM_WRITE:
                op = kind == eFOK_TEXT ? AM_OS_STRING("w") : AM_OS_STRING("wb");
            case eFOM_APPEND:
                op = kind == eFOK_TEXT ? AM_OS_STRING("a") : AM_OS_STRING("ab");
            case eFOM_READWRITE:
                op = kind == eFOK_TEXT ? AM_OS_STRING("w+") : AM_OS_STRING("wb+");
                break;
            case eFOM_READAPPEND:
                op = kind == eFOK_TEXT ? AM_OS_STRING("a+") : AM_OS_STRING("ab+");
                break;
        }

#if defined(AM_WINDOWS_VERSION)
        _wfopen_s(&mFileHandle, fileName.c_str(), op.c_str());
#else
        mFileHandle = fopen(fileName.c_str(), op.c_str());
#endif

        if (!mFileHandle)
            return AM_ERROR_FILE_NOT_FOUND;

        return AM_ERROR_NO_ERROR;
    }

    bool DiskFile::Eof()
    {
        return feof(mFileHandle) != 0;
    }

    AmSize MemoryFile::Read(AmUInt8Buffer dst, AmSize bytes)
    {
        if (mOffset + bytes >= mDataLength)
            bytes = mDataLength - mOffset;

        memcpy(dst, mDataPtr + mOffset, bytes);
        mOffset += bytes;

        return bytes;
    }

    AmSize MemoryFile::Write(AmConstUInt8Buffer src, AmSize bytes)
    {
        // Not available on memory files
        return 0;
    }

    AmSize MemoryFile::Length()
    {
        return mDataLength;
    }

    void MemoryFile::Seek(AmInt32 offset)
    {
        if (offset >= 0)
            mOffset = offset;
        else
            mOffset = mDataLength + offset;

        if (mOffset > mDataLength - 1)
            mOffset = mDataLength - 1;
    }

    AmSize MemoryFile::Pos()
    {
        return mOffset;
    }

    AmConstUInt8Buffer MemoryFile::GetMemPtr()
    {
        return mDataPtr;
    }

    MemoryFile::~MemoryFile()
    {
        if (mDataOwned)
            delete[] mDataPtr;
    }

    MemoryFile::MemoryFile()
    {
        mDataPtr = nullptr;
        mDataLength = 0;
        mOffset = 0;
        mDataOwned = false;
    }

    AmResult MemoryFile::OpenMem(AmConstUInt8Buffer data, AmSize dataLength, bool copy, bool takeOwnership)
    {
        if (data == nullptr || dataLength == 0)
            return AM_ERROR_INVALID_PARAMETER;

        if (mDataOwned)
            delete[] mDataPtr;
        mDataPtr = nullptr;
        mOffset = 0;

        mDataLength = dataLength;

        if (copy)
        {
            mDataOwned = true;
            mDataPtr = new unsigned char[dataLength];
            if (mDataPtr == nullptr)
                return AM_ERROR_OUT_OF_MEMORY;

            memcpy((AmVoidPtr)mDataPtr, data, dataLength);
            return AM_ERROR_NO_ERROR;
        }

        mDataPtr = data;
        mDataOwned = takeOwnership;

        return AM_ERROR_NO_ERROR;
    }

    AmResult MemoryFile::OpenToMem(const AmOsString& fileName)
    {
        if (fileName.empty())
            return AM_ERROR_INVALID_PARAMETER;
        if (mDataOwned)
            delete[] mDataPtr;

        mDataPtr = nullptr;
        mOffset = 0;

        DiskFile df;
        AmResult res = df.Open(fileName);
        if (res != AM_ERROR_NO_ERROR)
            return res;

        mDataLength = df.Length();
        mDataPtr = new AmUInt8[mDataLength];

        if (mDataPtr == nullptr)
            return AM_ERROR_OUT_OF_MEMORY;

        df.Read((AmUInt8Buffer)mDataPtr, mDataLength);
        mDataOwned = true;

        return AM_ERROR_NO_ERROR;
    }

    AmResult MemoryFile::OpenFileToMem(File* aFile)
    {
        if (!aFile)
            return AM_ERROR_INVALID_PARAMETER;
        if (mDataOwned)
            delete[] mDataPtr;

        mDataPtr = nullptr;
        mOffset = 0;

        mDataLength = aFile->Length();
        mDataPtr = new AmUInt8[mDataLength];
        if (mDataPtr == nullptr)
            return AM_ERROR_OUT_OF_MEMORY;

        aFile->Read((AmUInt8Buffer)mDataPtr, mDataLength);
        mDataOwned = true;

        return AM_ERROR_NO_ERROR;
    }

    bool MemoryFile::Eof()
    {
        if (mOffset >= mDataLength)
            return true;

        return false;
    }
} // namespace SparkyStudios::Audio::Amplitude
