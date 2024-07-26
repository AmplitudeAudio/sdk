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

#ifndef _AM_IO_DISK_FILESYSTEM_H
#define _AM_IO_DISK_FILESYSTEM_H

#include <filesystem>

#include <SparkyStudios/Audio/Amplitude/IO/FileSystem.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief A FileSystem implementation that reads and write files
     * from disk.
     */
    class AM_API_PUBLIC DiskFileSystem final : public FileSystem
    {
    public:
        /**
         * @brief Create a new instance of the DiskFileSystem class.
         */
        DiskFileSystem();

        /**
         * @copydoc FileSystem::SetBasePath
         */
        void SetBasePath(const AmOsString& basePath) override;

        /**
         * @copydoc FileSystem::GetBasePath
         */
        [[nodiscard]] const AmOsString& GetBasePath() const override;

        /**
         * @copydoc FileSystem::ResolvePath
         */
        [[nodiscard]] AmOsString ResolvePath(const AmOsString& path) const override;

        /**
         * @copydoc FileSystem::Exists
         */
        [[nodiscard]] bool Exists(const AmOsString& path) const override;

        /**
         * @copydoc FileSystem::IsDirectory
         */
        [[nodiscard]] bool IsDirectory(const AmOsString& path) const override;

        /**
         * @copydoc FileSystem::Join
         */
        [[nodiscard]] AmOsString Join(const std::vector<AmOsString>& parts) const override;

        /**
         * @copydoc FileSystem::OpenFile
         */
        [[nodiscard]] std::shared_ptr<File> OpenFile(const AmOsString& path) const override;

        /**
         * @copydoc FileSystem::StartOpenFileSystem
         */
        void StartOpenFileSystem() override;

        /**
         * @copydoc FileSystem::TryFinalizeOpenFileSystem
         */
        bool TryFinalizeOpenFileSystem() override;

        /**
         * @copydoc FileSystem::StartCloseFileSystem
         */
        void StartCloseFileSystem() override;

        /**
         * @copydoc FileSystem::TryFinalizeCloseFileSystem
         */
        bool TryFinalizeCloseFileSystem() override;

    private:
        std::filesystem::path _basePath;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IO_DISK_FILESYSTEM_H