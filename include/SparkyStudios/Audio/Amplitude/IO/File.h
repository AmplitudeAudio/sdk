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

#pragma once

#ifndef SPARK_AUDIO_FILE_H
#define SPARK_AUDIO_FILE_H

#include <cstdio>
#include <cstring>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    enum FileOpenMode
    {
        eFOM_READ = 0,
        eFOM_WRITE = 1,
        eFOM_APPEND = 2,
        eFOM_READWRITE = 3,
        eFOM_READAPPEND = 4,
    };

    enum FileOpenKind
    {
        eFOK_BINARY = 0,
        eFOK_TEXT = 1,
    };

    class AM_API_PUBLIC File
    {
    public:
        virtual ~File() = default;

        AmUInt8 Read8();

        AmUInt16 Read16();

        AmUInt32 Read32();

        virtual bool Eof() = 0;

        virtual AmSize Read(AmUInt8Buffer dst, AmSize bytes) = 0;

        virtual AmSize Write(AmConstUInt8Buffer src, AmSize bytes) = 0;

        virtual AmSize Length() = 0;

        virtual void Seek(AmSize offset) = 0;

        virtual AmSize Pos() = 0;

        virtual AmFileHandle GetFilePtr()
        {
            return nullptr;
        }

        virtual AmConstUInt8Buffer GetMemPtr()
        {
            return nullptr;
        }
    };

    class AM_API_PUBLIC DiskFile : public File
    {
    public:
        DiskFile();
        explicit DiskFile(AmFileHandle fp);

        ~DiskFile() override;

        bool Eof() override;

        AmSize Read(AmUInt8Buffer dst, AmSize bytes) override;

        AmSize Write(AmConstUInt8Buffer src, AmSize bytes) override;

        AmSize Length() override;

        void Seek(AmSize offset) override;

        void Seek(AmSize offset, int origin);

        AmSize Pos() override;

        AmResult Open(const AmOsString& fileName, FileOpenMode mode = eFOM_READ, FileOpenKind kind = eFOK_BINARY);

        void Close();

        AmFileHandle GetFilePtr() override;

    private:
        AmFileHandle mFileHandle;
    };

    class AM_API_PUBLIC MemoryFile : public File
    {
    public:
        MemoryFile();

        ~MemoryFile() override;

        bool Eof() override;

        AmSize Read(AmUInt8Buffer dst, AmSize bytes) override;

        AmSize Write(AmConstUInt8Buffer src, AmSize bytes) override;

        AmSize Length() override;

        void Seek(AmSize offset) override;
        AmSize Pos() override;
        AmConstUInt8Buffer GetMemPtr() override;
        AmResult OpenMem(AmConstUInt8Buffer data, AmSize dataLength, bool copy = false, bool takeOwnership = true);
        AmResult OpenToMem(const AmOsString& fileName);
        AmResult OpenFileToMem(File* aFile);

    private:
        AmConstUInt8Buffer mDataPtr;
        AmSize mDataLength;
        AmSize mOffset;
        bool mDataOwned;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_FILE_H
