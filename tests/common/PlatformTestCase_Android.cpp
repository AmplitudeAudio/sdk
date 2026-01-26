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

// Android-specific implementation of PlatformTestCase
// This file is compiled only on Android

#include "PlatformTestCase.h"

#if AM_PLATFORM_ANDROID

#include <SparkyStudios/Audio/Amplitude/IO/Android/AssetManagerFileSystem.h>
#include <SparkyStudios/Audio/Amplitude/IO/DiskFileSystem.h>

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // Global platform test configuration instance
    PlatformTestConfig g_platformTestConfig = {};

    std::shared_ptr<FileSystem> CreatePlatformFileSystem()
    {
        if (g_platformTestConfig.assetManager != nullptr)
            return ampoolshared(eMemoryPoolKind_IO, AssetManagerFileSystem, g_platformTestConfig.assetManager);

        // Fallback for Android with direct file access (e.g., rooted devices, testing)
        amLogWarning("Android: No AssetManager configured, falling back to DiskFileSystem");
        return ampoolshared(eMemoryPoolKind_IO, DiskFileSystem);
    }

    AmOsString GetPlatformAssetsBasePath()
    {
        // Android AssetManager uses paths relative to assets/ folder
        if (g_platformTestConfig.assetsSubPath != nullptr)
            return g_platformTestConfig.assetsSubPath;

        return AM_OS_STRING("assets");
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests

#endif // AM_PLATFORM_ANDROID
