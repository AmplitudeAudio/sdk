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

#include <IO/File.h>

namespace SparkyStudios::Audio::Amplitude
{
    AmUInt32 File::Read8()
    {
        AmUInt8 d = 0;
        Read((AmUInt8Buffer)&d, 1);
        return d;
    }

    AmUInt32 File::Read16()
    {
        AmUInt8 d = 0;
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

    AmUInt32 DiskFile::Read(AmUInt8Buffer dst, AmUInt32 bytes)
    {
        return (AmUInt32)fread(dst, 1, bytes, mFileHandle);
    }

    AmUInt32 DiskFile::Length()
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

    AmUInt32 DiskFile::Pos()
    {
        return (AmUInt32)ftell(mFileHandle);
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

    AmResult DiskFile::Open(AmOsString fileName)
    {
        if (!fileName)
            return AM_ERROR_INVALID_PARAMETER;

#if defined(AM_WINDOWS_VERSION)
        _wfopen_s(&mFileHandle, fileName, AM_OS_STRING("rb"));
#else
        fopen_s(&mFileHandle, aFilename, AM_OS_STRING("rb"));
#endif

        if (!mFileHandle)
            return AM_ERROR_FILE_NOT_FOUND;

        return AM_ERROR_NO_ERROR;
    }

    bool DiskFile::Eof()
    {
        return feof(mFileHandle) != 0;
    }

    AmUInt32 MemoryFile::Read(AmUInt8Buffer dst, AmUInt32 bytes)
    {
        if (mOffset + bytes >= mDataLength)
            bytes = mDataLength - mOffset;

        memcpy(dst, mDataPtr + mOffset, bytes);
        mOffset += bytes;

        return bytes;
    }

    AmUInt32 MemoryFile::Length()
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

    AmUInt32 MemoryFile::Pos()
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

    AmResult MemoryFile::OpenMem(AmConstUInt8Buffer data, AmUInt32 dataLength, bool copy, bool takeOwnership)
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

    AmResult MemoryFile::OpenToMem(AmOsString fileName)
    {
        if (!fileName)
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
