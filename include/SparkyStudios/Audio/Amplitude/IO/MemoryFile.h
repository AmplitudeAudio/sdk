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

#ifndef SS_AMPLITUDE_AUDIO_IO_MEMORYFILE_H
#define SS_AMPLITUDE_AUDIO_IO_MEMORYFILE_H

#include <SparkyStudios/Audio/Amplitude/IO/File.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief A File implementation that reads and writes a memory buffer.
     */
    class AM_API_PUBLIC MemoryFile : public File
    {
    public:
        /**
         * @brief Creates a new MemoryFile instance.
         */
        MemoryFile();

        /**
         * @brief Creates a new MemoryFile instance from a memory buffer.
         *
         * @param buffer The memory buffer to manage in this instance.
         * @param size The size of the memory buffer.
         * @param copy If true, the memory buffer will be copied.
         * @param takeOwnership If true, the memory buffer will be owned by this instance, and released when this instance is destroyed.
         */
        MemoryFile(AmUInt8Buffer buffer, AmSize size, bool copy = false, bool takeOwnership = true);

        /**
         * @brief Destroys the instance and release the memory buffer if owned.
         */
        ~MemoryFile() override;

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
         * @brief Opens a new memory buffer with the specified size.
         *
         * @param size The size of the buffer.
         *
         * @return The result of the operation.
         */
        AmResult Open(AmSize size);

        /**
         * @brief Opens a memory buffer.
         *
         * @param buffer The memory buffer to open.
         * @param size The size of the memory buffer.
         * @param copy If true, the memory buffer will be copied.
         * @param takeOwnership If true, the memory buffer will be owned by this instance, and released when this instance is destroyed.
         *
         * @return The result of the operation.
         */
        AmResult OpenMem(AmConstUInt8Buffer buffer, AmSize size, bool copy = false, bool takeOwnership = true);

        /**
         * @brief Opens a memory buffer from a file content.
         *
         * @param fileName The path to the file to open.
         *
         * @return The result of the operation.
         */
        AmResult OpenToMem(const std::filesystem::path& fileName);

        /**
         * @brief Copies data from a file instance to a memory buffer. The file content is entirely copied.
         *
         * @param file The file isntance to copy data from.
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

#endif // SS_AMPLITUDE_AUDIO_IO_MEMORYFILE_H
