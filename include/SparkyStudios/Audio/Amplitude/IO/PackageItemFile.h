// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#ifndef SS_AMPLITUDE_AUDIO_IO_PACKAGEFILE_H
#define SS_AMPLITUDE_AUDIO_IO_PACKAGEFILE_H

#include <SparkyStudios/Audio/Amplitude/IO/DiskFile.h>
#include <SparkyStudios/Audio/Amplitude/IO/PackageFileSystem.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief A File implementation that provides access to an item in an
     * Amplitude package file.
     */
    class AM_API_PUBLIC PackageItemFile : public DiskFile
    {
    public:
        PackageItemFile(const PackageFileItemDescription* item, const std::filesystem::path& packageFile, AmSize headerSize);

        /**
         * @copydoc DiskFile::GetPath
         */
        [[nodiscard]] AmOsString GetPath() const override;

        /**
         * @copydoc DiskFile::Eof
         */
        bool Eof() override;

        /**
         * @copydoc DiskFile::Read
         */
        AmSize Read(AmUInt8Buffer dst, AmSize bytes) override;

        /**
         * @copydoc DiskFile::Write
         *
         * @note Writing is disabled for packages item files.
         */
        AmSize Write(AmConstUInt8Buffer src, AmSize bytes) override;

        /**
         * @copydoc DiskFile::Length
         */
        AmSize Length() override;

        /**
         * @copydoc DiskFile::Seek
         */
        void Seek(AmSize offset, FileSeekOrigin origin) override;

        /**
         * @copydoc DiskFile::Position
         */
        AmSize Position() override;

    private:
        const PackageFileItemDescription* _description;
        AmSize _headerSize;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_IO_PACKAGEFILE_H