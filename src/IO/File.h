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
    class File
    {
    public:
        virtual ~File() = default;

        AmUInt32 Read8();

        AmUInt32 Read16();

        AmUInt32 Read32();

        virtual bool Eof() = 0;

        virtual AmUInt32 Read(AmUInt8Buffer dst, AmUInt32 bytes) = 0;

        virtual AmUInt32 Length() = 0;

        virtual void Seek(AmInt32 offset) = 0;

        virtual AmUInt32 Pos() = 0;

        virtual AmFileHandle GetFilePtr()
        {
            return nullptr;
        }

        virtual AmConstUInt8Buffer GetMemPtr()
        {
            return nullptr;
        }
    };

    class DiskFile : public File
    {
    public:
        DiskFile();
        explicit DiskFile(AmFileHandle fp);

        ~DiskFile() override;

        bool Eof() override;

        AmUInt32 Read(AmUInt8Buffer dst, AmUInt32 bytes) override;

        AmUInt32 Length() override;

        void Seek(AmInt32 offset) override;

        AmUInt32 Pos() override;

        AmResult Open(AmOsString fileName);

        AmFileHandle GetFilePtr() override;

    private:
        AmFileHandle mFileHandle;
    };

    class MemoryFile : public File
    {
    public:
        MemoryFile();

        ~MemoryFile() override;

        bool Eof() override;

        AmUInt32 Read(AmUInt8Buffer dst, AmUInt32 bytes) override;

        AmUInt32 Length() override;

        void Seek(AmInt32 offset) override;
        AmUInt32 Pos() override;
        AmConstUInt8Buffer GetMemPtr() override;
        AmResult OpenMem(AmConstUInt8Buffer data, AmUInt32 dataLength, bool copy = false, bool takeOwnership = true);
        AmResult OpenToMem(AmOsString fileName);
        AmResult OpenFileToMem(File* aFile);

    private:
        AmConstUInt8Buffer mDataPtr;
        AmUInt32 mDataLength;
        AmUInt32 mOffset;
        bool mDataOwned;
    };
}; // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_FILE_H
