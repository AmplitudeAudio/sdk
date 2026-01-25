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
--   - Android:                          tests/runners/android/
--
-- Usage:
--   Desktop: xmake build amplitude_tests
--   iOS:     xmake build AmplitudeTests_iOS
--   Android: xmake build AmplitudeTests_Android
--

-- Include platform-specific test runners
if is_plat("iphoneos") then
    includes("runners/ios/xmake.lua")
elseif is_plat("android") then
    includes("runners/android/xmake.lua")
else
    target("test_plugin")
        set_kind("shared")
        set_targetdir("$(builddir)/$(plat)/$(arch)/$(mode)/shared")
        add_defines("AM_BUILDSYSTEM_BUILDING_PLUGIN")

        add_deps("Amplitude::Shared")

        add_files("test_plugin/*.cpp")
    target_end()

    includes("runners/desktop/xmake.lua")
end
