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

// Desktop-specific implementation of PlatformTestCase
// This file is compiled only on Windows, macOS, and Linux

#include "PlatformTestCase.h"

#include <SparkyStudios/Audio/Amplitude/IO/DiskFileSystem.h>

#if !AM_PLATFORM_IOS && !AM_PLATFORM_ANDROID

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // Global platform test configuration instance
    PlatformTestConfig g_platformTestConfig = {};

    std::shared_ptr<FileSystem> CreatePlatformFileSystem()
    {
        // Desktop platforms use DiskFileSystem
        return ampoolshared(eMemoryPoolKind_IO, DiskFileSystem);
    }

    AmOsString GetPlatformAssetsBasePath()
    {
        // Desktop uses relative path from working directory
        return AM_OS_STRING("./samples/assets");
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests

#endif // !AM_PLATFORM_IOS && !AM_PLATFORM_ANDROID
