# Copyright (c) 2025-present Sparky Studios. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Inspired from AUI, licensed under MPL 2.0 - https://github.com/aui-framework/aui/blob/master/cmake/aui.build.cmake

# Platforms
if (WIN32)
    set(AM_PLATFORM_WIN 1 CACHE INTERNAL "Platform")
    list(REMOVE_ITEM AM_EXCLUDE_PLATFORMS win32)
else ()
    set(AM_PLATFORM_WIN 0 CACHE INTERNAL "Platform")
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(AM_PLATFORM_LINUX 1 CACHE INTERNAL "Platform")
    list(REMOVE_ITEM AM_EXCLUDE_PLATFORMS linux)
else ()
    set(AM_PLATFORM_LINUX 0 CACHE INTERNAL "Platform")
endif ()

if (UNIX AND APPLE)
    set(AM_PLATFORM_APPLE 1 CACHE INTERNAL "Platform")
    list(REMOVE_ITEM AM_EXCLUDE_PLATFORMS apple)
else ()
    set(AM_PLATFORM_APPLE 0 CACHE INTERNAL "Platform")
endif ()

if (UNIX AND APPLE AND NOT IOS)
    set(AM_PLATFORM_MACOS 1 CACHE INTERNAL "Platform")
    list(REMOVE_ITEM AM_EXCLUDE_PLATFORMS macos)
else ()
    set(AM_PLATFORM_MACOS 0 CACHE INTERNAL "Platform")
endif ()

if (ANDROID)
    set(AM_PLATFORM_ANDROID 1 CACHE INTERNAL "Platform")
    list(REMOVE_ITEM AM_EXCLUDE_PLATFORMS android)
else ()
    set(AM_PLATFORM_ANDROID 0 CACHE INTERNAL "Platform")
endif ()

if (IOS)
    set(AM_PLATFORM_IOS 1 CACHE INTERNAL "Platform")
    list(REMOVE_ITEM AM_EXCLUDE_PLATFORMS ios)
else ()
    set(AM_PLATFORM_IOS 0 CACHE INTERNAL "Platform")
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(AM_PLATFORM_EMSCRIPTEN 1 CACHE INTERNAL "Platform")
    list(REMOVE_ITEM AM_EXCLUDE_PLATFORMS emscripten)
else ()
    set(AM_PLATFORM_EMSCRIPTEN 0 CACHE INTERNAL "Platform")
endif ()

if (UNIX AND NOT AM_PLATFORM_EMSCRIPTEN)
    set(AM_PLATFORM_UNIX 1 CACHE INTERNAL "Platform")
    list(REMOVE_ITEM AM_EXCLUDE_PLATFORMS unix)
else ()
    set(AM_PLATFORM_UNIX 0 CACHE INTERNAL "Platform")
endif ()

# Compilers
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(AM_COMPILER_CLANG 1 CACHE INTERNAL "Compiler")
else ()
    set(AM_COMPILER_CLANG 0 CACHE INTERNAL "Compiler")
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(AM_COMPILER_GCC 1 CACHE INTERNAL "Compiler")
else ()
    set(AM_COMPILER_GCC 0 CACHE INTERNAL "Compiler")
endif ()

if (MSVC)
    set(AM_COMPILER_MSVC 1 CACHE INTERNAL "Compiler")
else ()
    set(AM_COMPILER_MSVC 0 CACHE INTERNAL "Compiler")
endif ()

# Architectures
if (CMAKE_GENERATOR_PLATFORM MATCHES "(arm64)|(ARM64)" OR CMAKE_SYSTEM_PROCESSOR MATCHES "(aarch64|arm64)")
    set(AM_ARCH_X86_64 0 CACHE INTERNAL "Arch")
    set(AM_ARCH_X86 0 CACHE INTERNAL "Arch")
    set(AM_ARCH_ARM_64 1 CACHE INTERNAL "Arch")
    set(AM_ARCH_ARM_V7 0 CACHE INTERNAL "Arch")
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "armv7-a")
    set(AM_ARCH_X86_64 0 CACHE INTERNAL "Arch")
    set(AM_ARCH_X86 0 CACHE INTERNAL "Arch")
    set(AM_ARCH_ARM_64 0 CACHE INTERNAL "Arch")
    set(AM_ARCH_ARM_V7 1 CACHE INTERNAL "Arch")
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)|(i.86)")
    if (CMAKE_SIZEOF_VOID_P STREQUAL 8)
        set(AM_ARCH_X86_64 1 CACHE INTERNAL "Arch")
        set(AM_ARCH_X86 0 CACHE INTERNAL "Arch")
        set(AM_ARCH_ARM_64 0 CACHE INTERNAL "Arch")
        set(AM_ARCH_ARM_V7 0 CACHE INTERNAL "Arch")
    else ()
        set(AM_ARCH_X86_64 0 CACHE INTERNAL "Arch")
        set(AM_ARCH_X86 1 CACHE INTERNAL "Arch")
        set(AM_ARCH_ARM_64 0 CACHE INTERNAL "Arch")
        set(AM_ARCH_ARM_V7 0 CACHE INTERNAL "Arch")
    endif ()
endif ()

if (AM_ARCH_ARM_V7 OR AM_ARCH_ARM_64)
    set(AM_ARCH_ARM 1 CACHE INTERNAL "Arch")
else ()
    set(AM_ARCH_ARM 0 CACHE INTERNAL "Arch")
endif ()

set(AM_EXCLUDE_PLATFORMS ${AM_EXCLUDE_PLATFORMS} CACHE INTERNAL "")