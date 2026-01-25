-- Copyright (c) 2026-present Sparky Studios. All rights reserved.
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
--     http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.

-- Android Test Runner Application
-- This target builds a NativeActivity-based Android application that runs
-- the Amplitude SDK tests on Android devices and emulators.
--
-- This file is only included when building for Android platform.
--
-- Usage:
--   xmake f -p android --ndk=/path/to/ndk -a arm64-v8a
--   xmake build AmplitudeTests_Android
--   xmake install AmplitudeTests_Android
--   xmake run AmplitudeTests_Android
--
-- The build process:
--   1. Compiles all test files and the native entry point
--   2. Links with Amplitude static library
--   3. Packages into an APK using xmake's android.native_app rule
--
-- To view test output:
--   adb logcat -s AmplitudeTests:V

target("AmplitudeTests_Android")
    set_kind("binary")
    set_languages("c++20")

    -- Use the android.native_app rule for APK generation
    add_rules("android.native_app", {
        package_name = "com.amplitudeaudiosdk.tests",
        android_sdk_version = "34",
        keystore = "debug.jks",
        keystore_pass = "123456",
        android_manifest = "AndroidManifest.xml"
    })

    -- Output directory
    set_targetdir("$(builddir)/android/$(arch)/$(mode)/tests")

    -- Main entry point
    add_files(path.join(os.scriptdir(), "main.cpp"))

    -- Test common files (platform abstraction and test registry)
    add_files("$(projectdir)/tests/common/PlatformTestCase_Android.cpp")
    add_files("$(projectdir)/tests/common/TestRegistry.cpp")
    add_files("$(projectdir)/tests/common/TestUtils.cpp")
    add_includedirs("$(projectdir)/tests/common")

    -- All test files (they self-register via static initialization)
    add_files("$(projectdir)/tests/core_*/test_*.cpp")
    add_files("$(projectdir)/tests/dsp_*/test_*.cpp")
    add_files("$(projectdir)/tests/fs_*/test_*.cpp")
    add_files("$(projectdir)/tests/hrtf_*/test_*.cpp")
    add_files("$(projectdir)/tests/io_*/test_*.cpp")
    add_files("$(projectdir)/tests/math_*/test_*.cpp")
    add_files("$(projectdir)/tests/mixer_*/test_*.cpp")
    add_files("$(projectdir)/tests/threading_*/test_*.cpp")

    -- Exclude shared library tests (not supported on Android due to plugin restrictions)
    remove_files("$(projectdir)/tests/**/*__shared.cpp")

    -- Include directories
    add_includedirs("$(projectdir)/include")
    add_includedirs("$(projectdir)/src")
    add_includedirs("$(builddir)/include")

    -- Link with Amplitude static library
    add_deps("Amplitude::Static")

    -- Android system libraries
    add_syslinks("log", "android")

    -- Suppress warnings for third-party code
    add_cxflags("-Wno-shorten-64-to-32", "-Wno-sign-conversion", {force = true})

    -- Bundle test assets after build
    -- Note: Assets must be pre-built on a desktop platform first
    after_build(function(target)
        import("core.project.config")

        local assets_src = path.join(config.builddir(), "samples/assets")
        local apk_assets = path.join(target:targetdir(), "assets")

        if os.isdir(assets_src) then
            os.mkdir(apk_assets)
            os.cp(assets_src .. "/*", apk_assets)
            print("Bundled test assets to: " .. apk_assets)
        else
            print("Warning: Test assets not found at: " .. assets_src)
            print("Run 'xmake build build_sample_project' on a desktop platform first,")
            print("then copy the assets to the Android build directory.")
        end
    end)
target_end()
