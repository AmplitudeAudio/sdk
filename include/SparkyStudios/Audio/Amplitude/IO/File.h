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

#ifndef SS_AMPLITUDE_AUDIO_IO_FILE_H
#define SS_AMPLITUDE_AUDIO_IO_FILE_H

#include <filesystem>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Describes the mode in which open the file.
     */
    enum FileOpenMode : AmUInt8
    {
        eFOM_READ = 0,
        eFOM_WRITE = 1,
        eFOM_APPEND = 2,
        eFOM_READWRITE = 3,
        eFOM_READAPPEND = 4,
    };

    /**
     * @brief The type of file being opened.
     */
    enum FileOpenKind : AmUInt8
    {
        eFOK_BINARY = 0,
        eFOK_TEXT = 1,
    };

    /**
     * @brief Defines from where to seek in the file.
     */
    enum FileSeekOrigin : AmUInt8
    {
        eFSO_START = SEEK_SET,
        eFSO_CURRENT = SEEK_CUR,
        eFSO_END = SEEK_END,
    };

    /**
     * @brief Base class for a file in a FileSystem.
     */
    class AM_API_PUBLIC File
    {
    public:
        /**
         * @brief Virtual destructor.
         */
        virtual ~File() = default;

        /**
         * @brief Gets the path to the file in the loaded FileSystem.
         *
         * @return The path to the file.
         */
        [[nodiscard]] virtual AmOsString GetPath() const = 0;

        /**
         * @brief Reads a single byte from the file in an @c AmUInt8.
         *
         * @return The read value.
         */
        AmUInt8 Read8();

        /**
         * @brief Reads two bytes from the file in an @c AmUInt16.
         *
         * @return The read value.
         */
        AmUInt16 Read16();

        /**
         * @brief Reads four bytes from the file in an @c AmUInt32.
         *
         * @return The read value.
         */
        AmUInt32 Read32();

        /**
         * @brief Reads eight bytes from the file in an @c AmUInt64.
         *
         * @return The read value.
         */
        AmUInt64 Read64();

        /**
         * @brief Reads a string from the file.
         *
         * @return The read value.
         */
        AmString ReadString();

        /**
         * @brief Writes a single byte to the file from an @c AmUInt8.
         *
         * @param value The value to write.
         */
        void Write8(AmUInt8 value);

        /**
         * @brief Writes two bytes to the file from an @c AmUInt16.
         *
         * @param value The value to write.
         */
        void Write16(AmUInt16 value);

        /**
         * @brief Writes four bytes to the file from an @c AmUInt32.
         *
         * @param value The value to write.
         */
        void Write32(AmUInt32 value);

        /**
         * @brief Writes eight bytes to the file from an @c AmUInt64.
         *
         * @param value The value to write.
         */
        void Write64(AmUInt64 value);

        /**
         * @brief Writes a string to the file.
         *
         * @param value The value to write.
         */
        void WriteString(const AmString& value);

        /**
         * @brief Check if the read cursor is at the end of the file.
         *
         * @return True if the read cursor is at the end of the file, false otherwise.
         */
        virtual bool Eof() = 0;

        /**
         * @brief Reads data from the file.
         *
         * @param dst The destination buffer of the read data.
         * @param bytes The number of bytes to read from the file. The destination buffer must be at least as large as the number of bytes
         * to read.
         *
         * @return The number of bytes read from the file.
         */
        virtual AmSize Read(AmUInt8Buffer dst, AmSize bytes) = 0;

        /**
         * @brief Writes data to the file.
         *
         * @param src The source buffer of the data to write.
         * @param bytes The number of bytes to write to the file. The source buffer must be at least as large as the number of bytes to
         * write.
         *
         * @return The number of bytes written to the file.
         */
        virtual AmSize Write(AmConstUInt8Buffer src, AmSize bytes) = 0;

        /**
         * @brief Gets the size of the file in bytes.
         *
         * @return The size of the file in bytes.
         */
        virtual AmSize Length() = 0;

        /**
         * @brief Seeks the read/write to the specified offset.
         *
         * @param offset The offset in bytes from the beginning of the file.
         */
        void Seek(AmSize offset);

        /**
         * @brief Seeks the read/write to the specified offset, starting at the given
         * origin.
         *
         * @param offset The offset in bytes from the beginning of the file.
         * @param origin The origin from which to begin seeking.
         */
        virtual void Seek(AmSize offset, FileSeekOrigin origin) = 0;

        /**
         * @brief Gets the current position of the read/write cursor.
         *
         * @return The actual position of the read/write cursor.
         */
        virtual AmSize Position() = 0;

        /**
         * @brief Gets the pointer to the internal file handle.
         *
         * @return The internal file handle. This depends on the implementation.
         */
        virtual AmVoidPtr GetPtr();

        /**
         * @brief Checks if the file is valid.
         *
         * Validity of a file is determined by the underlying implementation. But this should
         * primarily mean that the file exists AND has been opened.
         *
         * @return True if the file is valid, false otherwise.
         */
        [[nodiscard]] virtual bool IsValid() const = 0;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_IO_FILE_H
