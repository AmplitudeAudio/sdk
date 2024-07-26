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

#ifndef _AM_IO_DISK_FILE_H
#define _AM_IO_DISK_FILE_H

#include <SparkyStudios/Audio/Amplitude/IO/File.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief A File implementation that reads and writes a file on disk.
     */
    class AM_API_PUBLIC DiskFile : public File
    {
    public:
        /**
         * @brief Creates a new DiskFile instance.
         */
        DiskFile();

        /**
         * @brief Creates a new DiskFile instance from a file handle.
         *
         * @param fp The file handle to manage in this instance.
         */
        explicit DiskFile(AmFileHandle fp);

        /**
         * @brief Creates a new DiskFile instance by opening a file at the given path.
         *
         * @param fileName The path to the file to open.
         * @param mode The open mode to use.
         * @param kind The type of file to open.
         */
        explicit DiskFile(const std::filesystem::path& fileName, FileOpenMode mode = eFOM_READ, FileOpenKind kind = eFOK_BINARY);

        /**
         * @brief Destroys the instance and release the file handler.
         */
        ~DiskFile() override;

        /**
         * @copydoc File::GetPath
         */
        [[nodiscard]] AmOsString GetPath() const override;

        /**
         * @copydoc File::Eof
         */
        bool Eof() override;

        /**
         * @copydoc File::Read
         */
        AmSize Read(AmUInt8Buffer dst, AmSize bytes) override;

        /**
         * @copydoc File::Write
         */
        AmSize Write(AmConstUInt8Buffer src, AmSize bytes) override;

        /**
         * @copydoc File::Length
         */
        AmSize Length() override;

        /**
         * @copydoc File::Seek
         */
        void Seek(AmInt64 offset, FileSeekOrigin origin) override;

        /**
         * @copydoc File::Position
         */
        AmSize Position() override;

        /**
         * @copydoc File::GetPtr
         */
        AmVoidPtr GetPtr() override;

        /**
         * @copydoc File::IsValid
         */
        [[nodiscard]] bool IsValid() const override;

        /**
         * @brief Opens a file at the given path.
         *
         * @param filePath The path to the file to open.
         * @param mode The open mode to use.
         * @param kind The type of file to open.
         *
         * @return The result of the operation.
         */
        AmResult Open(const std::filesystem::path& filePath, FileOpenMode mode = eFOM_READ, FileOpenKind kind = eFOK_BINARY);

        /**
         * @brief Closes the file.
         */
        void Close();

    private:
        std::filesystem::path m_filePath;
        AmFileHandle m_fileHandle;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IO_DISK_FILE_H
