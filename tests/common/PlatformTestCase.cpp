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

#include "PlatformTestCase.h"

#include <SparkyStudios/Audio/Amplitude/IO/DiskFileSystem.h>

#if AM_PLATFORM_ANDROID
#include <SparkyStudios/Audio/Amplitude/IO/Android/AssetManagerFileSystem.h>
#endif

// iOS platform has its own implementation in PlatformTestCase_iOS.mm
#if !AM_PLATFORM_IOS

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // Global platform test configuration instance
    PlatformTestConfig g_platformTestConfig = {};

    std::shared_ptr<FileSystem> CreatePlatformFileSystem()
    {
#if AM_PLATFORM_ANDROID
        if (g_platformTestConfig.assetManager != nullptr)
            return ampoolshared(eMemoryPoolKind_IO, AssetManagerFileSystem, g_platformTestConfig.assetManager);

        // Fallback for Android with direct file access (e.g., rooted devices, testing)
        amLogWarning("Android: No AssetManager configured, falling back to DiskFileSystem");
        return ampoolshared(eMemoryPoolKind_IO, DiskFileSystem);

#else
        // Desktop platforms use DiskFileSystem
        return ampoolshared(eMemoryPoolKind_IO, DiskFileSystem);
#endif
    }

    AmOsString GetPlatformAssetsBasePath()
    {
#if AM_PLATFORM_ANDROID
        // Android AssetManager uses paths relative to assets/ folder
        if (g_platformTestConfig.assetsSubPath != nullptr)
            return AM_OS_STRING(g_platformTestConfig.assetsSubPath);

        return AM_OS_STRING("assets");

#elif AM_PLATFORM_IOS
        // iOS uses the bundle path provided by the app
        if (g_platformTestConfig.bundlePath != nullptr)
        {
            AmOsString basePath = AM_OS_STRING(g_platformTestConfig.bundlePath);
            basePath += AM_OS_STRING("/");

            if (g_platformTestConfig.assetsSubPath != nullptr)
                basePath += AM_OS_STRING(g_platformTestConfig.assetsSubPath);
            else
                basePath += AM_OS_STRING("assets");

            return basePath;
        }
        // Fallback if bundle path is not set
        amLogWarning("iOS: No bundle path configured, using default path");
        return AM_OS_STRING("./assets");

#else
        // Desktop uses relative path from working directory
        return AM_OS_STRING("./samples/assets");
#endif
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests

#endif // !AM_PLATFORM_IOS
