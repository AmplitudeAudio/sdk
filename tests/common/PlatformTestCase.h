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

#pragma once

#ifndef _AM_TESTS_COMMON_PLATFORM_TEST_CASE_H
#define _AM_TESTS_COMMON_PLATFORM_TEST_CASE_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#if AM_PLATFORM_ANDROID
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <SparkyStudios/Audio/Amplitude/IO/Android/AssetManagerFileSystem.h>
#endif

namespace SparkyStudios::Audio::Amplitude::Tests
{
    /**
     * @brief Platform-specific test configuration.
     *
     * This struct holds platform-specific resources needed for test execution.
     * On mobile platforms, this must be configured before running tests.
     */
    struct PlatformTestConfig
    {
#if AM_PLATFORM_ANDROID
        /**
         * @brief The Android AAssetManager instance.
         *
         * Must be set before running tests on Android.
         */
        AAssetManager* assetManager = nullptr;
#endif

#if AM_PLATFORM_IOS
        /**
         * @brief The iOS bundle path for locating assets.
         *
         * Should be set to [[NSBundle mainBundle] resourcePath] before running tests.
         */
        const char* bundlePath = nullptr;
#endif

        /**
         * @brief The subdirectory within the base path containing test assets.
         *
         * Defaults to "assets".
         */
        const AmOsChar* assetsSubPath = AM_OS_STRING("assets");
    };

    /**
     * @brief Global platform test configuration.
     *
     * Set this before running tests on mobile platforms. On desktop platforms,
     * the default configuration is typically sufficient.
     */
    extern PlatformTestConfig g_platformTestConfig;

    /**
     * @brief Creates the appropriate file system for the current platform.
     *
     * On Android, this returns an AssetManagerFileSystem if an asset manager is configured.
     * On iOS, this returns an NSFileSystem.
     * On desktop platforms, this returns a DiskFileSystem.
     *
     * @return A shared pointer to the platform-appropriate FileSystem.
     */
    std::shared_ptr<FileSystem> CreatePlatformFileSystem();

    /**
     * @brief Gets the base path for test assets on the current platform.
     *
     * On Android with AssetManager, this returns the assets subdirectory path.
     * On iOS, this returns the bundle path combined with the assets subdirectory.
     * On desktop, this returns "./samples/assets".
     *
     * @return The platform-appropriate base path for assets.
     */
    AmOsString GetPlatformAssetsBasePath();

    /**
     * @brief Checks if the current platform is a mobile platform.
     *
     * @return true if running on Android or iOS, false otherwise.
     */
    constexpr bool IsMobilePlatform()
    {
#if AM_PLATFORM_ANDROID || AM_PLATFORM_IOS
        return true;
#else
        return false;
#endif
    }

    /**
     * @brief Checks if plugin loading is supported on the current platform.
     *
     * Plugin loading is not supported on mobile platforms due to platform restrictions.
     *
     * @return true if plugins can be loaded, false otherwise.
     */
    constexpr bool SupportsPluginLoading()
    {
#if AM_PLATFORM_ANDROID || AM_PLATFORM_IOS
        return false;
#else
        return true;
#endif
    }

} // namespace SparkyStudios::Audio::Amplitude::Tests

#endif // _AM_TESTS_COMMON_PLATFORM_TEST_CASE_H
