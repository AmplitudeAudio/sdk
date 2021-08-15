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

#include <cstdio>
#include <cstring>

#include <IO/File.h>

namespace SparkyStudios::Audio::Amplitude
{
    unsigned int File::Read8()
    {
        unsigned char d = 0;
        Read((unsigned char*)&d, 1);
        return d;
    }

    unsigned int File::Read16()
    {
        unsigned short d = 0;
        Read((unsigned char*)&d, 2);
        return d;
    }

    unsigned int File::Read32()
    {
        unsigned int d = 0;
        Read((unsigned char*)&d, 4);
        return d;
    }

    DiskFile::DiskFile(FILE* fp)
        : mFileHandle(fp)
    {}

    unsigned int DiskFile::Read(unsigned char* aDst, unsigned int aBytes)
    {
        return (unsigned int)fread(aDst, 1, aBytes, mFileHandle);
    }

    unsigned int DiskFile::Length()
    {
        if (!mFileHandle)
            return 0;

        long pos = ftell(mFileHandle);
        fseek(mFileHandle, 0, SEEK_END);
        auto len = (unsigned int)ftell(mFileHandle);
        fseek(mFileHandle, pos, SEEK_SET);

        return len;
    }

    void DiskFile::Seek(int aOffset)
    {
        fseek(mFileHandle, aOffset, SEEK_SET);
    }

    unsigned int DiskFile::Pos()
    {
        return (unsigned int)ftell(mFileHandle);
    }

    FILE* DiskFile::GetFilePtr()
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

    unsigned int DiskFile::Open(const char* aFilename)
    {
        if (!aFilename)
            return FILE_ERROR_INVALID_PARAMETER;

        fopen_s(&mFileHandle, aFilename, "rb");

        if (!mFileHandle)
            return FILE_ERROR_FILE_NOT_FOUND;

        return FILE_ERROR_NO_ERROR;
    }

    int DiskFile::Eof()
    {
        return feof(mFileHandle);
    }

    unsigned int MemoryFile::Read(unsigned char* aDst, unsigned int aBytes)
    {
        if (mOffset + aBytes >= mDataLength)
            aBytes = mDataLength - mOffset;

        memcpy(aDst, mDataPtr + mOffset, aBytes);
        mOffset += aBytes;

        return aBytes;
    }

    unsigned int MemoryFile::Length()
    {
        return mDataLength;
    }

    void MemoryFile::Seek(int aOffset)
    {
        if (aOffset >= 0)
            mOffset = aOffset;
        else
            mOffset = mDataLength + aOffset;
        if (mOffset > mDataLength - 1)
            mOffset = mDataLength - 1;
    }

    unsigned int MemoryFile::Pos()
    {
        return mOffset;
    }

    const unsigned char* MemoryFile::GetMemPtr()
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

    unsigned int MemoryFile::OpenMem(const unsigned char* aData, unsigned int aDataLength, bool aCopy, bool aTakeOwnership)
    {
        if (aData == nullptr || aDataLength == 0)
            return FILE_ERROR_INVALID_PARAMETER;

        if (mDataOwned)
            delete[] mDataPtr;
        mDataPtr = nullptr;
        mOffset = 0;

        mDataLength = aDataLength;

        if (aCopy)
        {
            mDataOwned = true;
            mDataPtr = new unsigned char[aDataLength];
            if (mDataPtr == nullptr)
                return FILE_ERROR_OUT_OF_MEMORY;
            memcpy((void*)mDataPtr, aData, aDataLength);
            return FILE_ERROR_NO_ERROR;
        }

        mDataPtr = aData;
        mDataOwned = aTakeOwnership;
        return FILE_ERROR_NO_ERROR;
    }

    unsigned int MemoryFile::OpenToMem(const char* aFile)
    {
        if (!aFile)
            return FILE_ERROR_INVALID_PARAMETER;
        if (mDataOwned)
            delete[] mDataPtr;
        mDataPtr = nullptr;
        mOffset = 0;

        DiskFile df;
        unsigned int res = df.Open(aFile);
        if (res != FILE_ERROR_NO_ERROR)
            return res;

        mDataLength = df.Length();
        mDataPtr = new unsigned char[mDataLength];
        if (mDataPtr == nullptr)
            return FILE_ERROR_OUT_OF_MEMORY;
        df.Read((unsigned char*)mDataPtr, mDataLength);
        mDataOwned = true;
        return FILE_ERROR_NO_ERROR;
    }

    unsigned int MemoryFile::OpenFileToMem(File* aFile)
    {
        if (!aFile)
            return FILE_ERROR_INVALID_PARAMETER;
        if (mDataOwned)
            delete[] mDataPtr;
        mDataPtr = nullptr;
        mOffset = 0;

        mDataLength = aFile->Length();
        mDataPtr = new unsigned char[mDataLength];
        if (mDataPtr == nullptr)
            return FILE_ERROR_OUT_OF_MEMORY;
        aFile->Read((unsigned char*)mDataPtr, mDataLength);
        mDataOwned = true;
        return FILE_ERROR_NO_ERROR;
    }

    int MemoryFile::Eof()
    {
        if (mOffset >= mDataLength)
            return 1;
        return 0;
    }
} // namespace SparkyStudios::Audio::Amplitude
