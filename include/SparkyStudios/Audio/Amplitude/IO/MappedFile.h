// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#ifndef _AM_IO_MAPPED_FILE_H
#define _AM_IO_MAPPED_FILE_H

#include <SparkyStudios/Audio/Amplitude/IO/File.h>

#include <filesystem>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief A @ref File implementation that uses memory-mapped I/O for efficient read-only file access.
     *
     * On desktop platforms (Windows, Linux, macOS), files larger than or equal to 1 MB are memory-mapped
     * using platform-native APIs (mmap on POSIX, CreateFileMapping on Windows). For smaller files or when
     * memory mapping is unavailable, the file is loaded into a heap-allocated buffer.
     *
     * @note This implementation is read-only. @ref Write operations return 0.
     *
     * @ingroup io
     */
    class AM_API_PUBLIC MappedFile : public File
    {
    public:
        /**
         * @brief Creates a new @c MappedFile instance.
         */
        MappedFile();

        /**
         * @brief Creates a new @c MappedFile instance by opening a file at the given path.
         *
         * @param[in] fileName The path to the file to open.
         */
        explicit MappedFile(const std::filesystem::path& fileName);

        /**
         * @brief Destroys the instance and releases the memory mapping or heap buffer.
         */
        ~MappedFile() override;

        /**
         * @inherit
         */
        [[nodiscard]] AmOsString GetPath() const override;

        /**
         * @inherit
         */
        [[nodiscard]] bool Eof() const override;

        /**
         * @inherit
         */
        AmSize Read(AmUInt8Buffer dst, AmSize bytes) const override;

        /**
         * @brief Write is not supported for memory-mapped files.
         *
         * @return Always returns 0.
         */
        AmSize Write(AmConstUInt8Buffer src, AmSize bytes) override;

        /**
         * @inherit
         */
        [[nodiscard]] AmSize Length() const override;

        /**
         * @inherit
         */
        void Seek(AmInt64 offset, eFileSeekOrigin origin) override;

        /**
         * @inherit
         */
        [[nodiscard]] AmSize Position() const override;

        /**
         * @inherit
         */
        [[nodiscard]] AmVoidPtr GetPtr() const override;

        /**
         * @inherit
         */
        [[nodiscard]] bool IsValid() const override;

        /**
         * @inherit
         */
        void Close() override;

        /**
         * @brief Opens a file using memory mapping or heap-based fallback.
         *
         * Files larger than or equal to 1 MB are memory-mapped on supported desktop platforms.
         * Smaller files, or files on unsupported platforms, are loaded into a heap buffer.
         *
         * @param[in] filePath The path to the file to open.
         *
         * @return The result of the operation.
         */
        AmResult Open(const std::filesystem::path& filePath);

        /**
         * @brief Checks whether this file is backed by a memory mapping.
         *
         * @return @c true if memory-mapped, @c false if using heap-based fallback.
         */
        [[nodiscard]] bool IsMapped() const;

    private:
        std::filesystem::path m_filePath;
        AmUInt8Buffer m_dataPtr;
        AmSize m_dataSize;
        mutable AmSize m_offset;
        bool m_isMapped;
        bool m_dataOwned;
        AmVoidPtr m_mapHandle;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IO_MAPPED_FILE_H
