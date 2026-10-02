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

#include <SparkyStudios/Audio/Amplitude/IO/Log.h>
#include <SparkyStudios/Audio/Amplitude/IO/PackageFileSystem.h>
#include <SparkyStudios/Audio/Amplitude/IO/PackageItemFile.h>

#include <algorithm>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief The last supported package file version.
     */
    static constexpr AmUInt16 kLastPackageFileVersion = 1;

    PackageFileSystem::PackageFileSystem()
        : _packageFile(nullptr)
        , _loadingThreadHandle(nullptr)
        , _initialized{ false }
        , _valid{ false }
        , _header()
        , _headerSize(0)
    {}

    PackageFileSystem::~PackageFileSystem()
    {
        if (_loadingThreadHandle != nullptr)
        {
            Thread::Wait(_loadingThreadHandle);
            _loadingThreadHandle = nullptr;
        }

        if (_packageFile != nullptr)
        {
            _packageFile->Close();
            _packageFile.reset();
        }
    }

    void PackageFileSystem::SetBasePath(const AmOsString& basePath)
    {
        if (_fileSystem == nullptr)
            return;

        _packagePath = _fileSystem->ResolvePath(basePath);
    }

    const AmOsString& PackageFileSystem::GetBasePath() const
    {
        return _packagePath;
    }

    AmOsString PackageFileSystem::ResolvePath(const AmOsString& path) const
    {
        AmOsString resolvedPath = path;

        // Normalize path separators and resolve relative components manually
        // Replace all backslashes with forward slashes
        std::ranges::replace(resolvedPath, '\\', '/');

        // Remove duplicate slashes
        for (size_t i = 0; i < resolvedPath.length() - 1;)
        {
            if (resolvedPath[i] == '/' && resolvedPath[i + 1] == '/')
                resolvedPath.erase(i + 1, 1);
            else
                ++i;
        }

        // Handle . and .. components
        std::vector<AmOsString> components;
        AmSize start = 0;
        bool canPop = false;
        for (AmSize i = 0; i <= resolvedPath.length(); ++i)
        {
            if (i == resolvedPath.length() || resolvedPath[i] == '/')
            {
                if (i > start)
                {
                    AmOsString component = resolvedPath.substr(start, i - start);
                    if (component == AM_OS_STRING(".."))
                    {
                        if (!components.empty() && canPop)
                        {
                            components.pop_back();
                            canPop = !components.empty() && components.back() != AM_OS_STRING("..");
                        }
                        else
                        {
                            components.push_back(component);
                        }
                    }
                    else if (component != AM_OS_STRING("."))
                    {
                        components.push_back(component);
                        canPop = true;
                    }
                }
                start = i + 1;
            }
        }

        // Reconstruct the path
        if (components.empty())
        {
            resolvedPath = AM_OS_STRING(".");
        }
        else
        {
            resolvedPath.clear();
            for (size_t i = 0; i < components.size(); ++i)
            {
                if (i > 0)
                    resolvedPath += AM_OS_STRING("/");

                resolvedPath += components[i];
            }
        }

        // Remove possible "./" prefix
        if (resolvedPath.length() >= 2 && resolvedPath.substr(0, 2) == AM_OS_STRING("./"))
            resolvedPath = resolvedPath.substr(2);

        return resolvedPath;
    }

    bool PackageFileSystem::Exists(const AmOsString& path) const
    {
        if (!IsValid())
            return false;

        return _itemIndices.contains(path);
    }

    bool PackageFileSystem::IsDirectory(const AmOsString& path) const
    {
        // Never true for packaged assets
        return false;
    }

    AmOsString PackageFileSystem::Join(const std::vector<AmOsString>& parts) const
    {
        if (parts.empty())
            return AM_OS_STRING("");

        AmOsString joined(parts[0]);

        for (AmSize i = 1, l = parts.size(); i < l; i++)
            joined += AM_OS_STRING("/") + parts[i];

        return ResolvePath(joined);
    }

    std::shared_ptr<File> PackageFileSystem::OpenFile(const AmOsString& path, eFileOpenMode mode) const
    {
        if (!IsValid())
            return nullptr;

        const auto it = _itemIndices.find(path);
        if (it == _itemIndices.end())
            return nullptr;

        const auto packageFile = _fileSystem->OpenFile(_packagePath);
        if (packageFile == nullptr || !packageFile->IsValid())
        {
            amLogError("Cannot open the package item: unable to reopen the package at " AM_OS_CHAR_FMT, _packagePath.c_str());
            return nullptr;
        }

        const auto& item = _header.m_Items[it->second];
        return ampoolshared(eMemoryPoolKind_IO, PackageItemFile, &item, packageFile, _headerSize);
    }

    void PackageFileSystem::StartOpenFileSystem()
    {
        if (_loadingThreadHandle != nullptr)
        {
            Thread::Wait(_loadingThreadHandle);
            _loadingThreadHandle = nullptr;
        }

        if (_packageFile != nullptr)
            StartCloseFileSystem();

        _header.m_Items.clear();
        _itemIndices.clear();
        _headerSize = 0;
        _valid.store(false, std::memory_order_release);

        // The package file is read through the platform file system: without it, opening ends as an invalid file system.
        if (_fileSystem == nullptr)
        {
            amLogError("Cannot open the package file system: no platform file system set. Call SetPlatformFileSystem() first.");
            _initialized.store(true, std::memory_order_release);
            return;
        }

        _initialized.store(false, std::memory_order_release);
        _loadingThreadHandle = Thread::CreateThread(&PackageFileSystem::LoadPackage, this);
    }

    bool PackageFileSystem::TryFinalizeOpenFileSystem()
    {
        if (!_initialized.load(std::memory_order_acquire))
            return false;

        if (_loadingThreadHandle != nullptr)
        {
            Thread::Wait(_loadingThreadHandle);
            _loadingThreadHandle = nullptr;
        }

        return true;
    }

    void PackageFileSystem::StartCloseFileSystem()
    {
        if (_loadingThreadHandle != nullptr)
        {
            Thread::Wait(_loadingThreadHandle);
            _loadingThreadHandle = nullptr;
        }

        if (_packageFile != nullptr)
        {
            _packageFile->Close();
            _packageFile.reset();
        }

        _header.m_Items.clear();
        _itemIndices.clear();
        _headerSize = 0;
        _initialized.store(false, std::memory_order_release);
        _valid.store(false, std::memory_order_release);
    }

    bool PackageFileSystem::TryFinalizeCloseFileSystem()
    {
        return _packageFile == nullptr;
    }

    void PackageFileSystem::SetPlatformFileSystem(std::shared_ptr<FileSystem> fileSystem)
    {
        AMPLITUDE_ASSERT(fileSystem != nullptr);
        _fileSystem = fileSystem;
    }

    bool PackageFileSystem::IsValid() const
    {
        return _valid.load(std::memory_order_acquire);
    }

    void PackageFileSystem::LoadPackage(AmVoidPtr pParam)
    {
        auto* pFileSystem = static_cast<PackageFileSystem*>(pParam);

        constexpr AmSize kMaxPackageItems = 1000000;
        constexpr AmSize kMaxPackageChunks = 10000000;

        // Smallest on-disk records: an item is a name length plus four 64-bit fields, a chunk is three 64-bit fields.
        constexpr AmSize kMinItemRecordSize = sizeof(AmUInt32) + 4 * sizeof(AmUInt64);
        constexpr AmSize kMinChunkRecordSize = 3 * sizeof(AmUInt64);

        const auto rejectPackage = [pFileSystem]()
        {
            pFileSystem->_header.m_Items.clear();
            pFileSystem->_initialized.store(true, std::memory_order_release);
        };

        const auto remainingBytes = [pFileSystem](AmSize length)
        {
            const AmSize position = pFileSystem->_packageFile->Position();
            return position < length ? length - position : 0;
        };

        pFileSystem->_packageFile = pFileSystem->_fileSystem->OpenFile(pFileSystem->_packagePath);

        if (pFileSystem->_packageFile == nullptr || !pFileSystem->_packageFile->IsValid())
        {
            amLogError("Invalid package file at: " AM_OS_CHAR_FMT, pFileSystem->_packagePath.c_str());
            pFileSystem->_initialized.store(true, std::memory_order_release);
            return;
        }

        const AmSize packageLength = pFileSystem->_packageFile->Length();
        if (packageLength < sizeof(AmUInt32) + sizeof(AmUInt16) + sizeof(AmUInt8))
        {
            amLogError("Package file too small to contain a valid header: " AM_OS_CHAR_FMT, pFileSystem->_packagePath.c_str());
            pFileSystem->_initialized.store(true, std::memory_order_release);
            return;
        }

        // Read package file header
        {
            // Tag
            pFileSystem->_packageFile->Read(pFileSystem->_header.m_Header, 4);
            if (pFileSystem->_header.m_Header[0] != 'A' || pFileSystem->_header.m_Header[1] != 'M' ||
                pFileSystem->_header.m_Header[2] != 'P' || pFileSystem->_header.m_Header[3] != 'K')
            {
                amLogError("Invalid package file at: " AM_OS_CHAR_FMT, pFileSystem->_packagePath.c_str());
                pFileSystem->_initialized.store(true, std::memory_order_release);
                return;
            }

            // Version
            pFileSystem->_header.m_Version = pFileSystem->_packageFile->Read16();
            if (pFileSystem->_header.m_Version > kLastPackageFileVersion)
            {
                amLogError("Unsupported package file version at: " AM_OS_CHAR_FMT, pFileSystem->_packagePath.c_str());
                pFileSystem->_initialized.store(true, std::memory_order_release);
                return;
            }

            // Compression Algorithm
            pFileSystem->_header.m_CompressionMode = static_cast<ePackageFileCompressionMode>(pFileSystem->_packageFile->Read8());

            // Item Descriptions
            const AmSize itemsCount = pFileSystem->_packageFile->Read64();
            if (itemsCount > kMaxPackageItems || itemsCount > remainingBytes(packageLength) / kMinItemRecordSize)
            {
                amLogError(
                    "Package item count (%zu) exceeds the sanity limit or the package size: " AM_OS_CHAR_FMT, itemsCount,
                    pFileSystem->_packagePath.c_str());
                rejectPackage();
                return;
            }

            if (itemsCount > 0)
            {
                pFileSystem->_header.m_Items.reserve(itemsCount);
                for (AmSize i = 0; i < itemsCount; i++)
                {
                    auto& item = pFileSystem->_header.m_Items.emplace_back();

                    // Item Name/Path
                    item.m_Name = pFileSystem->_packageFile->ReadString();

                    // Item Offset
                    item.m_Offset = pFileSystem->_packageFile->Read64();

                    // Item Size
                    item.m_Size = pFileSystem->_packageFile->Read64();

                    // Compressed Block Size
                    item.m_CompressedBlockSize = pFileSystem->_packageFile->Read64();

                    if (item.m_Offset > packageLength)
                    {
                        amLogError("Package item out of bounds: " AM_OS_CHAR_FMT, pFileSystem->_packagePath.c_str());
                        rejectPackage();
                        return;
                    }

                    // Item Chunks
                    const AmSize chunksCount = pFileSystem->_packageFile->Read64();
                    if (chunksCount > kMaxPackageChunks || chunksCount > remainingBytes(packageLength) / kMinChunkRecordSize)
                    {
                        amLogError(
                            "Package chunk count (%zu) exceeds the sanity limit or the package size: " AM_OS_CHAR_FMT, chunksCount,
                            pFileSystem->_packagePath.c_str());
                        rejectPackage();
                        return;
                    }

                    if (chunksCount > 0)
                    {
                        item.m_CompressedChunks.reserve(chunksCount);
                        for (AmSize j = 0; j < chunksCount; j++)
                        {
                            auto& chunk = item.m_CompressedChunks.emplace_back();

                            // Chunk Offset
                            chunk.m_Offset = pFileSystem->_packageFile->Read64();

                            // Chunk Size
                            chunk.m_Size = pFileSystem->_packageFile->Read64();

                            // Chunk Compressed Size
                            chunk.m_CompressedSize = pFileSystem->_packageFile->Read64();

                            if (chunk.m_Offset > packageLength || chunk.m_CompressedSize > packageLength - chunk.m_Offset)
                            {
                                amLogError("Package chunk out of bounds: " AM_OS_CHAR_FMT, pFileSystem->_packagePath.c_str());
                                rejectPackage();
                                return;
                            }
                        }
                    }

                    AmSize itemExtent = item.m_Size;
                    if (item.m_CompressedBlockSize != 0)
                    {
                        itemExtent = 0;
                        for (const auto& chunk : item.m_CompressedChunks)
                            itemExtent += chunk.m_CompressedSize;
                    }

                    if (itemExtent > packageLength - item.m_Offset)
                    {
                        amLogError("Package item out of bounds: " AM_OS_CHAR_FMT, pFileSystem->_packagePath.c_str());
                        rejectPackage();
                        return;
                    }
                }
            }
        }

        pFileSystem->_itemIndices.reserve(pFileSystem->_header.m_Items.size());
        for (AmSize i = 0; i < pFileSystem->_header.m_Items.size(); ++i)
            pFileSystem->_itemIndices[AM_STRING_TO_OS_STRING(pFileSystem->_header.m_Items[i].m_Name)] = i;

        pFileSystem->_headerSize = pFileSystem->_packageFile->Position();
        pFileSystem->_valid.store(true, std::memory_order_release);
        pFileSystem->_initialized.store(true, std::memory_order_release);
    }
} // namespace SparkyStudios::Audio::Amplitude
