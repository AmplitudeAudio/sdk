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

#ifndef _AM_IO_MEMORY_FILE_H
#define _AM_IO_MEMORY_FILE_H

#include <SparkyStudios/Audio/Amplitude/IO/File.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief A `File` implementation that reads from and writes to a memory buffer.
     *
     * @ingroup io
     */
    class AM_API_PUBLIC MemoryFile : public File
    {
    public:
        /**
         * @brief Creates a new `MemoryFile` instance.
         */
        MemoryFile();

        /**
         * @brief Creates a new `MemoryFile` instance from a memory buffer.
         *
         * @param[in] buffer The memory buffer to manage in this instance.
         * @param[in] size The size of the memory buffer.
         * @param[in] copy If true, the memory buffer will be copied.
         * @param[in] takeOwnership If true, the memory buffer will be owned by this instance, and released when this instance is destroyed.
         */
        MemoryFile(AmUInt8Buffer buffer, AmSize size, bool copy = false, bool takeOwnership = true);

        /**
         * @brief Destroys the instance and release the memory buffer if owned.
         */
        ~MemoryFile() override;

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
         * @brief Opens a new memory buffer with the specified size.
         *
         * @param[in] size The size of the buffer.
         *
         * @return The result of the operation.
         */
        AmResult Open(AmSize size);

        /**
         * @brief Opens a memory buffer.
         *
         * @param[in] buffer The memory buffer to open.
         * @param[in] size The size of the memory buffer.
         * @param[in] copy If true, the memory buffer will be copied.
         * @param[in] takeOwnership If true, the memory buffer will be owned by this instance, and released when this instance is destroyed.
         *
         * @return The result of the operation.
         */
        AmResult OpenMem(AmConstUInt8Buffer buffer, AmSize size, bool copy = false, bool takeOwnership = true);

        /**
         * @brief Opens a memory buffer from a file content.
         *
         * @param[in] fileName The path to the file to open.
         *
         * @return The result of the operation.
         */
        AmResult OpenToMem(const std::filesystem::path& fileName);

        /**
         * @brief Copies data from a file instance to a memory buffer. The file content is entirely copied.
         *
         * @param[in] file The file instance to copy data from.
         *
         * @return The result of the operation.
         */
        AmResult OpenFileToMem(File* file);

        /**
         * @brief Closes the memory buffer and releases associated resources.
         */
        void Close();

    private:
        AmUInt8Buffer m_dataPtr;
        AmSize m_dataSize;
        AmSize m_offset;
        bool m_dataOwned;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IO_MEMORY_FILE_H
