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

// iOS-specific implementation of PlatformTestCase
// This file is compiled as Objective-C++ to handle Foundation framework includes

#if AM_PLATFORM_IOS

#include "PlatformTestCase.h"

#include <SparkyStudios/Audio/Amplitude/IO/iOS/NSFileSystem.h>

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // Global platform test configuration instance
    PlatformTestConfig g_platformTestConfig = {};

    std::shared_ptr<FileSystem> CreatePlatformFileSystem()
    {
        // iOS uses NSFileSystem for native file operations
        return ampoolshared(eMemoryPoolKind_IO, NSFileSystem);
    }

    AmOsString GetPlatformAssetsBasePath()
    {
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
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests

#endif // AM_PLATFORM_IOS
