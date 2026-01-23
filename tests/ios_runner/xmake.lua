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

-- iOS Test Runner Application
-- This target builds an iOS application that runs the Amplitude SDK tests
-- on iOS devices and simulators.

if is_plat("iphoneos") then
  target("AmplitudeTests_iOS")
    add_rules("xcode.application")
    set_kind("binary")
    set_basename("AmplitudeTests")

    -- iOS specific settings
    set_targetdir("$(builddir)/iphoneos/$(arch)/$(mode)/tests")

    -- Set Xcode-specific values
    set_values("xcode.bundle_identifier", "com.amplitudeaudiosdk.tests")

    -- Disable auto-checking of flags that might cause issues
    set_policy("check.auto_ignore_flags", false)

    -- Required iOS frameworks
    add_frameworks("Foundation", "UIKit", "CoreGraphics")
    add_frameworks("Accelerate", "CoreAudio", "AudioToolbox", "CoreFoundation")

    -- Enable ARC for Objective-C files
    add_mxflags("-fobjc-arc")

    -- Source files
    add_files("AmplitudeTests/*.m")
    add_files("AmplitudeTests/*.mm")

    -- Storyboard and Info.plist files
    add_files("AmplitudeTests/*.storyboard")
    add_files("AmplitudeTests/Info.plist")

    -- Test common files (platform abstraction and test registry)
    add_files("$(projectdir)/tests/common/PlatformTestCase_iOS.mm")
    add_files("$(projectdir)/tests/common/TestRegistry.cpp")
    add_files("$(projectdir)/tests/common/TestUtils.cpp")
    add_includedirs("$(projectdir)/tests/common")

    -- Include directories
    add_includedirs("$(projectdir)/include")
    add_includedirs("$(projectdir)/src")
    add_includedirs("$(builddir)/include")

    -- Link with Amplitude static library
    add_deps("Amplitude::Static")

    -- C++ settings
    set_languages("c++20")
    add_cxxflags("-stdlib=libc++")
    add_ldflags("-lc++")

    -- Suppress warnings for third-party code
    add_cxflags("-Wno-shorten-64-to-32", "-Wno-sign-conversion")

    -- Bundle test assets after build
    -- Note: Assets must be pre-built on a desktop platform first
    after_build(function(target)
      import("core.project.config")

      local assets_src = path.join(config.builddir(), "samples/assets")
      local bundle_path = path.join(target:targetdir(), target:basename() .. ".app")
      local assets_dest = path.join(bundle_path, "assets")

      if os.isdir(assets_src) then
        os.mkdir(assets_dest)
        os.cp(assets_src .. "/*", assets_dest)
        print("Bundled test assets to: " .. assets_dest)
      else
        print("Warning: Test assets not found at: " .. assets_src)
        print("Run 'xmake build build_sample_project' on a desktop platform first,")
        print("then copy the assets to the iOS build directory.")
      end
    end)
  target_end()
end
