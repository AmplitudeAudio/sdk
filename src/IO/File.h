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

#ifndef SPARK_AUDIO_FILE_H
#define SPARK_AUDIO_FILE_H

#include <cstdio>

namespace SparkyStudios::Audio::Amplitude
{
    enum FILE_ERROR
    {
        FILE_ERROR_NO_ERROR = 0, // No error
        FILE_ERROR_INVALID_PARAMETER = 1, // Some parameter is invalid
        FILE_ERROR_FILE_NOT_FOUND = 2, // File not found
        FILE_ERROR_FILE_LOAD_FAILED = 3, // File found, but could not be loaded
        FILE_ERROR_DLL_NOT_FOUND = 4, // DLL not found, or wrong DLL
        FILE_ERROR_OUT_OF_MEMORY = 5, // Out of memory
        FILE_ERROR_UNKNOWN = 7 // Other error
    };

    class File
    {
    public:
        virtual ~File()
        {}
        unsigned int Read8();
        unsigned int Read16();
        unsigned int Read32();
        virtual int Eof() = 0;
        virtual unsigned int Read(unsigned char* aDst, unsigned int aBytes) = 0;
        virtual unsigned int Length() = 0;
        virtual void Seek(int aOffset) = 0;
        virtual unsigned int Pos() = 0;
        virtual FILE* GetFilePtr()
        {
            return nullptr;
        }
        virtual const unsigned char* GetMemPtr()
        {
            return nullptr;
        }
    };

    class DiskFile : public File
    {
    public:
        FILE* mFileHandle;

        virtual int Eof();
        virtual unsigned int Read(unsigned char* aDst, unsigned int aBytes);
        virtual unsigned int Length();
        virtual void Seek(int aOffset);
        virtual unsigned int Pos();
        virtual ~DiskFile();
        DiskFile();
        DiskFile(FILE* fp);
        unsigned int Open(const char* aFilename);
        virtual FILE* GetFilePtr();
    };

    class MemoryFile : public File
    {
    public:
        const unsigned char* mDataPtr;
        unsigned int mDataLength;
        unsigned int mOffset;
        bool mDataOwned;

        virtual int Eof();
        virtual unsigned int Read(unsigned char* aDst, unsigned int aBytes);
        virtual unsigned int Length();
        virtual void Seek(int aOffset);
        virtual unsigned int Pos();
        virtual const unsigned char* GetMemPtr();
        virtual ~MemoryFile();
        MemoryFile();
        unsigned int OpenMem(const unsigned char* aData, unsigned int aDataLength, bool aCopy = false, bool aTakeOwnership = true);
        unsigned int OpenToMem(const char* aFilename);
        unsigned int OpenFileToMem(File* aFile);
    };
}; // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_FILE_H
