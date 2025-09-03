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

#if AM_PLATFORM_ANDROID

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/IO/Android/AssetManagerFile.h>
#include <SparkyStudios/Audio/Amplitude/IO/Android/AssetManagerFileSystem.h>

#include <ranges>
#include <sstream>
#include <vector>

namespace SparkyStudios::Audio::Amplitude
{
    AssetManagerFileSystem::AssetManagerFileSystem(AAssetManager* assetManager)
        : m_assetManager(assetManager)
    {}

    void AssetManagerFileSystem::SetBasePath(const AmOsString& basePath)
    {
        m_basePath = basePath;
    }

    const AmOsString& AssetManagerFileSystem::GetBasePath() const
    {
        return m_basePath;
    }

    AmOsString AssetManagerFileSystem::ResolvePath(const AmOsString& path) const
    {
        AmOsString resolvedPath = m_basePath;

        // If the base path is not empty and doesn't end with a slash, add one
        if (!resolvedPath.empty() && resolvedPath.back() != '/')
            resolvedPath += '/';

        // Append the given path
        resolvedPath += path;

        return CleanPath(resolvedPath);
    }

    bool AssetManagerFileSystem::Exists(const AmOsString& path) const
    {
        if (m_assetManager == nullptr)
            return false;

        const AmOsString resolvedPath = ResolvePath(path);

        if (AAsset* asset = AAssetManager_open(m_assetManager, resolvedPath.c_str(), AASSET_MODE_UNKNOWN); asset != nullptr)
        {
            AAsset_close(asset);
            return true;
        }

        return false;
    }

    bool AssetManagerFileSystem::IsDirectory(const AmOsString& path) const
    {
        if (m_assetManager == nullptr)
            return false;

        const AmOsString resolvedPath = ResolvePath(path);

        // Try to open the path as a directory
        AAssetDir* assetDir = AAssetManager_openDir(m_assetManager, resolvedPath.c_str());
        if (assetDir == nullptr)
            return false;

        // Close the asset directory
        AAssetDir_close(assetDir);

        return true;
    }

    AmOsString AssetManagerFileSystem::Join(const std::vector<AmOsString>& parts) const
    {
        if (parts.empty())
            return AM_OS_STRING("");

        AmOsString joined(parts[0]);

        for (AmSize i = 1, l = parts.size(); i < l; i++)
        {
            if (!parts[i].empty() && joined.back() != '/')
                joined += '/';

            joined += parts[i];
        }

        return CleanPath(joined);
    }

    std::shared_ptr<File> AssetManagerFileSystem::OpenFile(const AmOsString& path, eFileOpenMode mode) const
    {
        const AmOsString resolvedPath = ResolvePath(path);
        if (AAsset* asset = AAssetManager_open(m_assetManager, resolvedPath.c_str(), AASSET_MODE_RANDOM))
            return AmSharedPtr<AssetManagerFile, eMemoryPoolKind_IO>::Make(asset, resolvedPath);

        return nullptr;
    }

    void AssetManagerFileSystem::StartOpenFileSystem()
    {}

    bool AssetManagerFileSystem::TryFinalizeOpenFileSystem()
    {
        return true;
    }

    void AssetManagerFileSystem::StartCloseFileSystem()
    {}

    bool AssetManagerFileSystem::TryFinalizeCloseFileSystem()
    {
        return true;
    }

    AmOsString AssetManagerFileSystem::CleanPath(const AmOsString& path)
    {
        AmOsString cleanedPath = path;

        // Replace backslashes with forward slashes (Android uses forward slashes)
        std::ranges::replace(cleanedPath, '\\', '/');

        // Remove any double slashes
        size_t pos;
        while ((pos = cleanedPath.find("//")) != AmOsString::npos)
            cleanedPath.erase(pos, 1);

        // Remove leading slash if present
        if (!cleanedPath.empty() && cleanedPath.front() == '/')
            cleanedPath.erase(0, 1);

        // Normalize paths with '..'
        std::vector<AmOsString> components;
        std::istringstream pathStream(cleanedPath);
        AmOsString component;

        while (std::getline(pathStream, component, '/'))
        {
            if (component == "..")
            {
                if (!components.empty() && components.back() != "..")
                    components.pop_back();
                else
                    components.push_back(component);
            }
            else if (!component.empty() && component != ".")
            {
                components.push_back(component);
            }
        }

        // Reconstruct the path
        cleanedPath.clear();
        for (const auto& comp : components)
        {
            if (!cleanedPath.empty())
                cleanedPath += '/';

            cleanedPath += comp;
        }

        return cleanedPath;
    }
} // namespace SparkyStudios::Audio::Amplitude

#endif // AM_PLATFORM_ANDROID
