-- Copyright (c) 2025-present Sparky Studios. All rights reserved.
-- Licensed under the Apache License, Version 2.0 (the "License");
-- You may not use this file except in compliance with the License.
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
-- Inspired from AUI, licensed under MPL 2.0 - https://github.com/aui-framework/aui/blob/master/cmake/aui.build.cmake
function am_apply_detected_platform_defines(target)
  -- Platforms
  if target:is_plat("windows") then
    target:add("defines", "AM_PLATFORM_WIN=1", { public = false })
  else
    target:add("defines", "AM_PLATFORM_WIN=0", { public = false })
  end

  if target:is_plat("linux") then
    target:add("defines", "AM_PLATFORM_LINUX=1", { public = false })
  else
    target:add("defines", "AM_PLATFORM_LINUX=0", { public = false })
  end

  if target:is_plat("macosx") then
    target:add("defines", "AM_PLATFORM_APPLE=1", { public = false })
    target:add("defines", "AM_PLATFORM_MACOS=1", { public = false })
  else
    target:add("defines", "AM_PLATFORM_APPLE=0", { public = false })
    target:add("defines", "AM_PLATFORM_MACOS=0", { public = false })
  end

  if target:is_plat("android") then
    target:add("defines", "AM_PLATFORM_ANDROID=1", { public = false })
  else
    target:add("defines", "AM_PLATFORM_ANDROID=0", { public = false })
  end

  if target:is_plat("iphoneos") then
    target:add("defines", "AM_PLATFORM_IOS=1", { public = false })
  else
    target:add("defines", "AM_PLATFORM_IOS=0", { public = false })
  end

  if target:is_plat("wasm") then
    target:add("defines", "AM_PLATFORM_EMSCRIPTEN=1", { public = false })
  else
    target:add("defines", "AM_PLATFORM_EMSCRIPTEN=0", { public = false })
  end

  if target:is_plat("linux") or target:is_plat("macosx") or target:is_plat("android") or target:is_plat("iphoneos") or
      target:is_plat("wasm") then
    target:add("defines", "AM_PLATFORM_UNIX=1", { public = false })
  else
    target:add("defines", "AM_PLATFORM_UNIX=0", { public = false })
  end

  -- Compilers
  local cc = target:tool("cc")
  local cxx = target:tool("cxx")

  if cc or cxx then
    if cc:find("clang") or cxx:find("clang") then
      target:add("defines", "AM_COMPILER_CLANG=1", { public = false })
    else
      target:add("defines", "AM_COMPILER_CLANG=0", { public = false })
    end

    if cc:find("gcc") or cxx:find("gcc") then
      target:add("defines", "AM_COMPILER_GCC=1", { public = false })
    else
      target:add("defines", "AM_COMPILER_GCC=0", { public = false })
    end

    if cc:find("cl.exe") or cxx:find("cl.exe") then
      target:add("defines", "AM_COMPILER_MSVC=1", { public = false })
    else
      target:add("defines", "AM_COMPILER_MSVC=0", { public = false })
    end
  end

  -- Architectures
  if target:is_arch("x86_64") or target:is_arch("x64") or target:is_arch("amd64") then
    target:add("defines", "AM_ARCH_X86_64=1", { public = false })
    target:add("defines", "AM_ARCH_X86=0", { public = false })
    target:add("defines", "AM_ARCH_ARM_64=0", { public = false })
    target:add("defines", "AM_ARCH_ARM_V7=0", { public = false })
  elseif target:is_arch("x86") then
    target:add("defines", "AM_ARCH_X86_64=0", { public = false })
    target:add("defines", "AM_ARCH_X86=1", { public = false })
    target:add("defines", "AM_ARCH_ARM_64=0", { public = false })
    target:add("defines", "AM_ARCH_ARM_V7=0", { public = false })
  elseif target:is_arch("arm64") or target:is_arch("aarch64") or target:is_arch("arm64-v8a") then
    target:add("defines", "AM_ARCH_X86_64=0", { public = false })
    target:add("defines", "AM_ARCH_X86=0", { public = false })
    target:add("defines", "AM_ARCH_ARM_64=1", { public = false })
    target:add("defines", "AM_ARCH_ARM_V7=0", { public = false })
  elseif target:is_arch("armv7") or target:is_arch("armeabi-v7a") then
    target:add("defines", "AM_ARCH_X86_64=0", { public = false })
    target:add("defines", "AM_ARCH_X86=0", { public = false })
    target:add("defines", "AM_ARCH_ARM_64=0", { public = false })
    target:add("defines", "AM_ARCH_ARM_V7=1", { public = false })
  else
    target:add("defines", "AM_ARCH_X86_64=0", { public = false })
    target:add("defines", "AM_ARCH_X86=0", { public = false })
    target:add("defines", "AM_ARCH_ARM_64=0", { public = false })
    target:add("defines", "AM_ARCH_ARM_V7=0", { public = false })
  end

  if target:is_arch("armv7") or target:is_arch("armeabi-v7a") or target:is_arch("arm64") or target:is_arch("aarch64") or target:is_arch("arm64-v8a") then
    target:add("defines", "AM_ARCH_ARM=1", { public = false })
  else
    target:add("defines", "AM_ARCH_ARM=0", { public = false })
  end
end
