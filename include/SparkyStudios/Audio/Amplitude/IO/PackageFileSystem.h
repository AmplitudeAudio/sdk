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

#ifndef SS_AMPLITUDE_AUDIO_IO_PACKAGEFILESYSTEM_H
#define SS_AMPLITUDE_AUDIO_IO_PACKAGEFILESYSTEM_H

#include <SparkyStudios/Audio/Amplitude/Core/Thread.h>
#include <SparkyStudios/Audio/Amplitude/IO/DiskFile.h>
#include <SparkyStudios/Audio/Amplitude/IO/FileSystem.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Defines the compression algorithms a package file can be compressed with.
     */
    enum PackageFileCompressionAlgorithm : AmUInt8
    {
        /**
         * @brief No compression algorithm has been used for the package file.
         */
        ePCA_None,

        /**
         * @brief The package file has been compressed using ZLib.
         */
        ePCA_ZLib,

        /**
         * @brief Invalid compression algorithm.
         */
        ePCA_Invalid
    };

    /**
     * @brief Describes an item in the package file.
     *
     * The item description is stored in the package file's header.
     */
    struct PackageFileItemDescription
    {
        /**
         * @brief The name of the package item. It usually refers to the
         * resource path.
         */
        AmString m_Name;

        /**
         * @brief The offset of the package item in the package file.
         */
        AmSize m_Offset = 0;

        /**
         * @brief The size of the package item in bytes.
         */
        AmSize m_Size = 0;
    };

    /**
     * @brief Provides metadata about the package file.
     */
    struct PackageFileHeaderDescription
    {
        /**
         * @brief Package file header tag. Should be equal to 'AMPK'.
         */
        AmUInt8 m_Header[4];

        /**
         * @brief Package file version. Used to implement new features in package
         * files and still be backward compatible with old versions.
         */
        AmUInt16 m_Version = 0;

        /**
         * @brief The compression algorithm used for this package file.
         */
        PackageFileCompressionAlgorithm m_CompressionAlgorithm = ePCA_Invalid;

        /**
         * @brief The description of each item in the package file.
         *
         * The total number of descriptions should match the number of items.
         */
        std::vector<PackageFileItemDescription> m_Items;
    };

    /**
     * @brief A FileSystem implementation that provides access
     * to an Amplitude package file.
     */
    class AM_API_PUBLIC PackageFileSystem final : public FileSystem
    {
    public:
        /**
         * @brief Constructs a new @c PackageFileSystem instance.
         */
        PackageFileSystem();

        ~PackageFileSystem() override;

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
        static void LoadPackage(AmVoidPtr pParam);

        std::filesystem::path _packagePath;
        AmUniquePtr<MemoryPoolKind::IO, DiskFile> _packageFile;

        AmThreadHandle _loadingThreadHandle;
        mutable bool _initialized;
        bool _valid;

        PackageFileHeaderDescription _header;
        AmSize _headerSize;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_IO_PACKAGEFILESYSTEM_H
