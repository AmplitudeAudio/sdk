# Copyright (c) 2021-present Sparky Studios. All rights reserved.
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

# Amplitude installation path
set(AM_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/.." CACHE PATH "The path to Amplitude Audio SDK libraries.")

# Check for a known file in the SDK path to verify the path
function(is_valid_sdk sdk_path is_valid)
    set(${is_valid} FALSE PARENT_SCOPE)
    if(EXISTS ${sdk_path})
        set(sdk_file ${sdk_path}/include/SparkyStudios/Audio/Amplitude/Amplitude.h)
        if(EXISTS ${sdk_file})
            set(${is_valid} TRUE PARENT_SCOPE)
        endif()
    endif()
endfunction()

# Paths that will be checked, in order:
# - CMake cache variable
# - A Environment Variable
set(AMPLITUDE_SDK_PATHS
    "${AM_SDK_PATH}"
    "$ENV{SS_AMPLITUDE_ROOT_PATH}"
)

set(found_sdk FALSE)
foreach(candidate_path ${AMPLITUDE_SDK_PATHS})
    is_valid_sdk(${candidate_path} found_sdk)
    if(found_sdk)
        # Update the Amplitude installation path variable internally
        set(AM_SDK_PATH "${candidate_path}")
        break()
    endif()
endforeach()

if(NOT found_sdk)
    # If we don't find a path that appears to be a valid Amplitude install, we can bail here.
    message(SEND_ERROR "Unable to find a valid Amplitude Audio SDK installation.")
    return()
endif()

message(STATUS "Using Amplitude Audio SDK at ${AM_SDK_PATH}")

set(AMPLITUDE_COMPILE_DEFINITIONS
    $<IF:$<CONFIG:Release>,AMPLITUDE_NO_ASSERTS,>
    $<IF:$<CONFIG:Release>,AM_NO_MEMORY_STATS,>
)

# Use these to get the parent path and folder name before adding the external 3rd party target.
get_filename_component(AMPLITUDE_INSTALL_ROOT ${AM_SDK_PATH} DIRECTORY)
get_filename_component(AMPLITUDE_FOLDER ${AM_SDK_PATH} NAME)

if(WIN32)
    set(AMPLITUDE_LIB_OS "win")

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(AMPLITUDE_STATIC_LIB_NAME "libAmplitude.a")
        set(AMPLITUDE_STATIC_LIB_NAME_DEBUG "libAmplitude_d.a")
        set(AMPLITUDE_SHARED_LIB_NAME "libAmplitude.dll")
        set(AMPLITUDE_SHARED_LIB_NAME_DEBUG "libAmplitude_d.dll")
    else()
        set(AMPLITUDE_STATIC_LIB_NAME "Amplitude.lib")
        set(AMPLITUDE_STATIC_LIB_NAME_DEBUG "Amplitude_d.lib")
        set(AMPLITUDE_SHARED_LIB_NAME "Amplitude.dll")
        set(AMPLITUDE_SHARED_LIB_NAME_DEBUG "Amplitude_d.dll")
    endif()
elseif(APPLE)
    set(AMPLITUDE_LIB_OS "osx")
    set(AMPLITUDE_STATIC_LIB_NAME "libAmplitude.a")
    set(AMPLITUDE_STATIC_LIB_NAME_DEBUG "libAmplitude_d.a")
    set(AMPLITUDE_SHARED_LIB_NAME "libAmplitude.dylib")
    set(AMPLITUDE_SHARED_LIB_NAME_DEBUG "libAmplitude_d.dylib")
else()
    set(AMPLITUDE_LIB_OS "linux")
    set(AMPLITUDE_STATIC_LIB_NAME "libAmplitude.a")
    set(AMPLITUDE_STATIC_LIB_NAME_DEBUG "libAmplitude_d.a")
    set(AMPLITUDE_SHARED_LIB_NAME "libAmplitude.so")
    set(AMPLITUDE_SHARED_LIB_NAME_DEBUG "libAmplitude_d.so")
endif()

add_library(SparkyStudios::Audio::Amplitude::SDK::Static STATIC IMPORTED GLOBAL)
set_target_properties(SparkyStudios::Audio::Amplitude::SDK::Static PROPERTIES
    IMPORTED_LOCATION "${AM_SDK_PATH}/lib/${AMPLITUDE_LIB_OS}/static/${AMPLITUDE_STATIC_LIB_NAME}"
    IMPORTED_LOCATION_DEBUG "${AM_SDK_PATH}/lib/${AMPLITUDE_LIB_OS}/static/${AMPLITUDE_STATIC_LIB_NAME_DEBUG}"
    INTERFACE_INCLUDE_DIRECTORIES "${AM_SDK_PATH}/include"
    INTERFACE_COMPILE_DEFINITIONS "AM_BUILDSYSTEM_STATIC"
)

add_library(SparkyStudios::Audio::Amplitude::SDK::Shared SHARED IMPORTED GLOBAL)
set_target_properties(SparkyStudios::Audio::Amplitude::SDK::Shared PROPERTIES
    IMPORTED_LOCATION "${AM_SDK_PATH}/lib/${AMPLITUDE_LIB_OS}/shared/${AMPLITUDE_SHARED_LIB_NAME}"
    IMPORTED_LOCATION_DEBUG "${AM_SDK_PATH}/lib/${AMPLITUDE_LIB_OS}/shared/${AMPLITUDE_SHARED_LIB_NAME_DEBUG}"
    INTERFACE_INCLUDE_DIRECTORIES "${AM_SDK_PATH}/include"
    INTERFACE_COMPILE_DEFINITIONS "AM_BUILDSYSTEM_SHARED"
)

if(WIN32)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(AMPLITUDE_STATIC_LIB_NAME "libAmplitude.dll.a")
        set(AMPLITUDE_STATIC_LIB_NAME_DEBUG "libAmplitude_d.dll.a")
    endif()

    set_target_properties(SparkyStudios::Audio::Amplitude::SDK::Shared PROPERTIES
        IMPORTED_IMPLIB "${AM_SDK_PATH}/lib/${AMPLITUDE_LIB_OS}/shared/${AMPLITUDE_STATIC_LIB_NAME}"
        IMPORTED_IMPLIB_DEBUG "${AM_SDK_PATH}/lib/${AMPLITUDE_LIB_OS}/shared/${AMPLITUDE_STATIC_LIB_NAME_DEBUG}"
    )
endif()
