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

#ifndef _AM_IO_PACKAGE_FILE_H
#define _AM_IO_PACKAGE_FILE_H

#include <SparkyStudios/Audio/Amplitude/IO/DiskFile.h>
#include <SparkyStudios/Audio/Amplitude/IO/PackageFileSystem.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief A @c File implementation that provides access to an item in an Amplitude package file.
     *
     * @ingroup io
     */
    class AM_API_PUBLIC PackageItemFile : public DiskFile
    {
    public:
        /**
         * @brief Constructs a new @c PackageItemFile instance.
         *
         * @param[in] item The description of the package item.
         * @param[in] packageFile The path to the package file.
         * @param[in] headerSize The size of the package file header.
         */
        PackageItemFile(const PackageFileItemDescription* item, const std::filesystem::path& packageFile, AmSize headerSize);

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
         * @inherit
         *
         * @note Writing is disabled for packages item files.
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
         * @brief Get the base position of this file item in the entire package file.
         */
        [[nodiscard]] AmSize GetBasePosition() const;

    private:
        const PackageFileItemDescription* _description;
        const bool _isCompressed;
        AmSize _headerSize;
        mutable AmInt64 _currentPosition;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IO_PACKAGE_FILE_H
