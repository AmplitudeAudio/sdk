# Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

cmake_minimum_required(VERSION 3.22.1)

project("amplitude_tests" LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch Eigen 5.x (required by internal SDK headers - must match SDK's eigen ^5.0.0 requirement)
include(FetchContent)
FetchContent_Declare(
    eigen
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
    GIT_TAG 5.0.1
    GIT_SHALLOW TRUE
)
# Don't build Eigen tests and examples
set(EIGEN_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(EIGEN_BUILD_PKGCONFIG OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(eigen)

# Define platform macros required by Amplitude SDK headers
add_compile_definitions(
    AM_PLATFORM_ANDROID=1
    AM_PLATFORM_UNIX=1
    AM_COMPILER_CLANG=1
)

# Architecture-specific macros
if(${ANDROID_ABI} STREQUAL "arm64-v8a")
  add_compile_definitions(AM_ARCH_ARM_64=1)
elseif(${ANDROID_ABI} STREQUAL "armeabi-v7a")
  add_compile_definitions(AM_ARCH_ARM_V7=1)
elseif(${ANDROID_ABI} STREQUAL "x86_64")
  add_compile_definitions(AM_ARCH_X86_64=1)
elseif(${ANDROID_ABI} STREQUAL "x86")
  add_compile_definitions(AM_ARCH_X86=1)
endif()

# Path to Amplitude SDK root directory
# This assumes the android_runner is in tests/android_runner/
set(AMPLITUDE_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../..")

# Normalize the path
get_filename_component(AMPLITUDE_SDK_DIR "${AMPLITUDE_SDK_DIR}" ABSOLUTE)

message(STATUS "Amplitude SDK directory: ${AMPLITUDE_SDK_DIR}")
message(STATUS "Android ABI: ${ANDROID_ABI}")

# Include directories
include_directories(
    ${AMPLITUDE_SDK_DIR}/include
    ${AMPLITUDE_SDK_DIR}/src
    ${AMPLITUDE_SDK_DIR}/tests/common
    ${eigen_SOURCE_DIR}
)

# Check if pre-built Amplitude library exists
set(AMPLITUDE_LIB_DIR "${AMPLITUDE_SDK_DIR}/sdk/lib/${ANDROID_ABI}-android")
set(AMPLITUDE_INCLUDE_DIR "${AMPLITUDE_SDK_DIR}/sdk/include")

if(EXISTS "${AMPLITUDE_INCLUDE_DIR}")
  include_directories(${AMPLITUDE_INCLUDE_DIR})
  message(STATUS "Using generated headers from: ${AMPLITUDE_INCLUDE_DIR}")
endif()

# Test common sources
set(TEST_COMMON_SOURCES
    ${AMPLITUDE_SDK_DIR}/tests/common/PlatformTestCase.cpp
    ${AMPLITUDE_SDK_DIR}/tests/common/TestRegistry.cpp
    ${AMPLITUDE_SDK_DIR}/tests/common/TestUtils.cpp
)

# Native test bridge library
add_library(amplitude_tests SHARED
    native_test_bridge.cpp
    ${TEST_COMMON_SOURCES}
)

# Try to find pre-built Amplitude library
# Note: Debug builds have _d suffix (e.g., libAmplitude_d.a)
find_library(AMPLITUDE_LIB
    NAMES Amplitude_d Amplitude libAmplitude_d libAmplitude
    PATHS
        ${AMPLITUDE_LIB_DIR}/static
        ${AMPLITUDE_LIB_DIR}/shared
        ${AMPLITUDE_SDK_DIR}
    NO_DEFAULT_PATH
)

if(AMPLITUDE_LIB)
  message(STATUS "Found Amplitude library: ${AMPLITUDE_LIB}")
  target_link_libraries(amplitude_tests
        ${AMPLITUDE_LIB}
    )
else()
  message(WARNING "Amplitude library not found. You may need to build it first.")
  message(WARNING "Looked in: ${AMPLITUDE_LIB_DIR}")

  # Still set up the target so the build doesn't fail immediately
  # The actual linking will fail, but this provides a better error message
endif()

# Link Android system libraries
target_link_libraries(amplitude_tests
    android
    log
)

# Compiler flags
target_compile_options(amplitude_tests PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
)
