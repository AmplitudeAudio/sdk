-- Copyright (c) 2021-present Sparky Studios. All rights reserved.
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

--
-- Amplitude Audio SDK Test Infrastructure
--
-- This file orchestrates test runners for different platforms:
--   - Desktop (Windows, Linux, macOS):  tests/runners/desktop/
--   - iOS:                              tests/runners/ios/
--   - Android:                          tests/runners/android/ (uses Gradle/CMake)
--
-- Usage:
--   Desktop: xmake build amplitude_tests && ./amplitude_tests -v
--   iOS:     xmake build AmplitudeTests_iOS
--   Android: cd tests/runners/android && ./gradlew assembleDebug
--

-- Include platform-specific test runners
if is_plat("iphoneos") then
    -- iOS test runner (Xcode application)
    includes("runners/ios/xmake.lua")
elseif not is_plat("android") then
    -- Desktop test runner (CLI executable)
    -- Note: Android uses Gradle/CMake build system, not XMake

    -- Desktop-only targets (tools and test infrastructure)
    target("generate_test_package")
        set_kind("phony")

        add_deps("ampk", "build_sample_project")

        on_build(function(target)
            import("core.project.config")
            import("core.project.project")
            import("lib.detect.find_tool")

            local ampk = project.target("ampk")

            local program = ampk:targetfile()
            if program then
                local assets_dir = path.join(path.absolute(config.builddir()), "samples/assets")
                local output_uncompressed_dir = path.join(path.absolute(config.builddir()), "samples/assets_uncompressed.ampk")
                local output_compressed_dir = path.join(path.absolute(config.builddir()), "samples/assets_compressed.ampk")

                os.exec("%s -q -c 0 %s %s", program, assets_dir, output_uncompressed_dir)
                os.exec("%s -q -c 1 %s %s", program, assets_dir, output_compressed_dir)
            else
                print("ampk not found.")
            end
        end)
    target_end()

    target("test_plugin")
        set_kind("shared")
        set_targetdir("$(builddir)/$(plat)/$(arch)/$(mode)/shared")
        add_defines("AM_BUILDSYSTEM_BUILDING_PLUGIN")

        add_deps("Amplitude::Shared")

        add_files("test_plugin/*.cpp")
    target_end()

    -- Include desktop test runner
    includes("runners/desktop/xmake.lua")
end
