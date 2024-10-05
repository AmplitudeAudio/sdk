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
     * @brief A `File` implementation that reads and writes a file on disk.
     *
     * @ingroup io
     */
    class AM_API_PUBLIC DiskFile : public File
    {
    public:
        /**
         * @brief Creates a new `DiskFile` instance.
         */
        DiskFile();

        /**
         * @brief Creates a new `DiskFile` instance from a file handle.
         *
         * @param[in] fp The file handle to manage in this instance.
         */
        explicit DiskFile(AmFileHandle fp);

        /**
         * @brief Creates a new DiskFile instance by opening a file at the given path.
         *
         * @param[in] fileName The path to the file to open.
         * @param[in] mode The open mode to use.
         * @param[in] kind The type of file to open.
         */
        explicit DiskFile(
            const std::filesystem::path& fileName, eFileOpenMode mode = eFileOpenMode_Read, eFileOpenKind kind = eFileOpenKind_Binary);

        /**
         * @brief Destroys the instance and release the file handler.
         */
        ~DiskFile() override;

        /**
         * @inherit
         */
        [[nodiscard]] AmOsString GetPath() const override;

        /**
         * @inherit
         */
        bool Eof() override;

        /**
         * @inherit
         */
        AmSize Read(AmUInt8Buffer dst, AmSize bytes) override;

        /**
         * @inherit
         */
        AmSize Write(AmConstUInt8Buffer src, AmSize bytes) override;

        /**
         * @inherit
         */
        AmSize Length() override;

        /**
         * @inherit
         */
        void Seek(AmInt64 offset, eFileSeekOrigin origin) override;

        /**
         * @inherit
         */
        AmSize Position() override;

        /**
         * @inherit
         */
        AmVoidPtr GetPtr() override;

        /**
         * @inherit
         */
        [[nodiscard]] bool IsValid() const override;

        /**
         * @brief Opens a file at the given path.
         *
         * @param[in] filePath The path to the file to open.
         * @param[in] mode The open mode to use.
         * @param[in] kind The type of file to open.
         *
         * @return The result of the operation.
         */
        AmResult Open(
            const std::filesystem::path& filePath, eFileOpenMode mode = eFileOpenMode_Read, eFileOpenKind kind = eFileOpenKind_Binary);

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
