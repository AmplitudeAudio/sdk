// Copyright (c) 2025-present Sparky Studios. All rights reserved.
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

#if AM_PLATFORM_ANDROID

#ifndef _AM_IO_ANDROID_ASSET_MANAGER_FILE_SYSTEM_H
#define _AM_IO_ANDROID_ASSET_MANAGER_FILE_SYSTEM_H

#include <android/asset_manager.h>

#include <SparkyStudios/Audio/Amplitude/IO/FileSystem.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief A @c FileSystem implementation that reads and writes files from an Android AssetManager.
     *
     * @ingroup io
     */
    class AM_API_PUBLIC AssetManagerFileSystem final : public FileSystem
    {
    public:
        /**
         * @brief Creates a new instance of the @c AssetManagerFileSystem class.
         *
         * @param[in] assetManager The Android @c AAssetManager instance to use for file operations.
         */
        explicit AssetManagerFileSystem(AAssetManager* assetManager);

        /**
         * @inherit
         */
        void SetBasePath(const AmOsString& basePath) override;

        /**
         * @inherit
         */
        [[nodiscard]] const AmOsString& GetBasePath() const override;

        /**
         * @inherit
         */
        [[nodiscard]] AmOsString ResolvePath(const AmOsString& path) const override;

        /**
         * @inherit
         */
        [[nodiscard]] bool Exists(const AmOsString& path) const override;

        /**
         * @inherit
         */
        [[nodiscard]] bool IsDirectory(const AmOsString& path) const override;

        /**
         * @inherit
         */
        [[nodiscard]] AmOsString Join(const std::vector<AmOsString>& parts) const override;

        /**
         * @inherit
         */
        [[nodiscard]] std::shared_ptr<File> OpenFile(const AmOsString& path, eFileOpenMode mode) const override;

        /**
         * @inherit
         */
        void StartOpenFileSystem() override;

        /**
         * @inherit
         */
        bool TryFinalizeOpenFileSystem() override;

        /**
         * @inherit
         */
        void StartCloseFileSystem() override;

        /**
         * @inherit
         */
        bool TryFinalizeCloseFileSystem() override;

    private:
        /**
         * @brief Cleans a path for use in the Android's asset manager system.
         *
         * @param path The path to clean.
         *
         * @return The cleaned version of the given path.
         *
         * @internal
         */
        static AmOsString CleanPath(const AmOsString& path);

        AAssetManager* m_assetManager;
        AmOsString m_basePath;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IO_ANDROID_ASSET_MANAGER_FILE_SYSTEM_H

#endif // AM_PLATFORM_ANDROID
