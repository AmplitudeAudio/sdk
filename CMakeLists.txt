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

cmake_minimum_required(VERSION 3.20)

cmake_policy(SET CMP0048 NEW)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(DetectAmplitudeVersion)

option(BUILD_SAMPLES "Build samples" OFF)

if(BUILD_SAMPLES)
    list(APPEND VCPKG_MANIFEST_FEATURES "samples")
endif()

project(Amplitude VERSION ${DETECTED_AM_VERSION})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_DEBUG_POSTFIX "_d")

set(BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(OBJ_DIR ${CMAKE_CURRENT_BINARY_DIR}/obj)

# Setup flatbuffers
# ------------------------------------------------------------------------------
set(FLATBUFFERS_BUILD_TESTS OFF CACHE BOOL "")

if(NOT TARGET flatc)
    add_subdirectory("dependencies/flatbuffers" ${OBJ_DIR}/flatbuffers EXCLUDE_FROM_ALL)
    set(FLATC_LIBRARY flatc)
    set(FLATC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/flatbuffers/include)
endif()

# ------------------------------------------------------------------------------

# Setup libsamplerate
# ------------------------------------------------------------------------------
set(LIBSAMPLERATE_EXAMPLES OFF CACHE BOOL "")
set(LIBSAMPLERATE_INSTALL ON CACHE BOOL "")

if(NOT TARGET samplerate)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/libsamplerate/cmake)
    add_subdirectory("dependencies/libsamplerate" ${OBJ_DIR}/libsamplerate EXCLUDE_FROM_ALL)
    set(LIBSAMPLERATE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/libsamplerate/include)
endif()

# ------------------------------------------------------------------------------

# Setup libsimdpp
# ------------------------------------------------------------------------------
# Check what instruction sets the current host supports. Use them for testing
include("dependencies/libsimdpp/cmake/SimdppMultiarch.cmake")
message(STATUS "Checking instruction sets to run tests for on this host...")
simdpp_get_runnable_archs(NATIVE_ARCHS)

foreach(ARCH ${NATIVE_ARCHS})
    simdpp_get_arch_info(CXX_FLAGS DEFINES_LIST SUFFIX ${ARCH})

    separate_arguments(CXX_FLAGS)
    add_compile_options(${CXX_FLAGS})
endforeach()

# ------------------------------------------------------------------------------
set(SIMDPP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/libsimdpp)

# Generate source files for all FlatBuffers schema files under the src
# directory.
set(AM_FLATBUFFERS_GENERATED_INCLUDES_DIR ${OBJ_DIR}/include/SparkyStudios/Audio/Amplitude)
file(GLOB_RECURSE FLATBUFFERS_SCHEMAS ${CMAKE_CURRENT_SOURCE_DIR}/schemas/*.fbs)

# Generate rules to build the set of output files from the set of input
# schema files.
build_flatbuffers("${FLATBUFFERS_SCHEMAS}"
    ""
    generated_includes
    ""
    ${AM_FLATBUFFERS_GENERATED_INCLUDES_DIR}
    ""
    ""
)

# Setup project source files.
set(SA_SOURCE
    include/SparkyStudios/Audio/Amplitude/Core/Common/Platforms/Android/Config.h
    include/SparkyStudios/Audio/Amplitude/Core/Common/Platforms/Android/Types.h
    include/SparkyStudios/Audio/Amplitude/Core/Common/Platforms/Linux/Config.h
    include/SparkyStudios/Audio/Amplitude/Core/Common/Platforms/Linux/Types.h
    include/SparkyStudios/Audio/Amplitude/Core/Common/Platforms/UNIX/Config.h
    include/SparkyStudios/Audio/Amplitude/Core/Common/Platforms/UNIX/Types.h
    include/SparkyStudios/Audio/Amplitude/Core/Common/Platforms/Windows/Config.h
    include/SparkyStudios/Audio/Amplitude/Core/Common/Platforms/Windows/Types.h
    include/SparkyStudios/Audio/Amplitude/Core/Common/Config.h
    include/SparkyStudios/Audio/Amplitude/Core/Common/Constants.h
    include/SparkyStudios/Audio/Amplitude/Core/Common/Types.h
    include/SparkyStudios/Audio/Amplitude/Core/Bus.h
    include/SparkyStudios/Audio/Amplitude/Core/Channel.h
    include/SparkyStudios/Audio/Amplitude/Core/Codec.h
    include/SparkyStudios/Audio/Amplitude/Core/Common.h
    include/SparkyStudios/Audio/Amplitude/Core/Device.h
    include/SparkyStudios/Audio/Amplitude/Core/Driver.h
    include/SparkyStudios/Audio/Amplitude/Core/Engine.h
    include/SparkyStudios/Audio/Amplitude/Core/Entity.h
    include/SparkyStudios/Audio/Amplitude/Core/Environment.h
    include/SparkyStudios/Audio/Amplitude/Core/Event.h
    include/SparkyStudios/Audio/Amplitude/Core/Listener.h
    include/SparkyStudios/Audio/Amplitude/Core/Log.h
    include/SparkyStudios/Audio/Amplitude/Core/Memory.h
    include/SparkyStudios/Audio/Amplitude/Core/RefCounter.h
    include/SparkyStudios/Audio/Amplitude/Core/Thread.h
    include/SparkyStudios/Audio/Amplitude/Core/Version.h
    include/SparkyStudios/Audio/Amplitude/IO/FileLoader.h
    include/SparkyStudios/Audio/Amplitude/Math/Curve.h
    include/SparkyStudios/Audio/Amplitude/Math/HandmadeMath.h
    include/SparkyStudios/Audio/Amplitude/Math/Shape.h
    include/SparkyStudios/Audio/Amplitude/Mixer/Resampler.h
    include/SparkyStudios/Audio/Amplitude/Mixer/SoundProcessor.h
    include/SparkyStudios/Audio/Amplitude/Sound/Attenuation.h
    include/SparkyStudios/Audio/Amplitude/Sound/Collection.h
    include/SparkyStudios/Audio/Amplitude/Sound/Effect.h
    include/SparkyStudios/Audio/Amplitude/Sound/Fader.h
    include/SparkyStudios/Audio/Amplitude/Sound/Filter.h
    include/SparkyStudios/Audio/Amplitude/Sound/Rtpc.h
    include/SparkyStudios/Audio/Amplitude/Sound/Scheduler.h
    include/SparkyStudios/Audio/Amplitude/Sound/Sound.h
    include/SparkyStudios/Audio/Amplitude/Sound/SoundBank.h
    include/SparkyStudios/Audio/Amplitude/Sound/SoundObject.h
    include/SparkyStudios/Audio/Amplitude/Sound/Switch.h
    include/SparkyStudios/Audio/Amplitude/Sound/SwitchContainer.h
    include/SparkyStudios/Audio/Amplitude/Amplitude.h

    src/Core/Codecs/AMS/Codec.cpp
    src/Core/Codecs/AMS/Codec.h

    src/Core/Codecs/FLAC/Codec.cpp
    src/Core/Codecs/FLAC/Codec.h
    src/Core/Codecs/FLAC/dr_flac.h

    src/Core/Codecs/MP3/Codec.cpp
    src/Core/Codecs/MP3/Codec.h
    src/Core/Codecs/MP3/dr_mp3.h

    src/Core/Codecs/OGG/Codec.cpp
    src/Core/Codecs/OGG/Codec.h
    src/Core/Codecs/OGG/stb_vorbis.c
    src/Core/Codecs/OGG/stb_vorbis.h

    src/Core/Codecs/WAV/Codec.cpp
    src/Core/Codecs/WAV/Codec.h
    src/Core/Codecs/WAV/dr_wav.h

    src/Core/Drivers/MiniAudio/Driver.cpp
    src/Core/Drivers/MiniAudio/Driver.h

    src/Core/Bus.cpp
    src/Core/BusInternalState.cpp
    src/Core/BusInternalState.h
    src/Core/Channel.cpp
    src/Core/ChannelInternalState.cpp
    src/Core/ChannelInternalState.h
    src/Core/Codec.cpp
    src/Core/Common.cpp
    src/Core/Device.cpp
    src/Core/Driver.cpp
    src/Core/Engine.cpp
    src/Core/EngineInternalState.h
    src/Core/Entity.cpp
    src/Core/EntityInternalState.cpp
    src/Core/EntityInternalState.h
    src/Core/Environment.cpp
    src/Core/EnvironmentInternalState.cpp
    src/Core/EnvironmentInternalState.h
    src/Core/Event.cpp
    src/Core/ListenerInternalState.cpp
    src/Core/ListenerInternalState.h
    src/Core/Listener.cpp
    src/Core/Log.cpp
    src/Core/Memory.cpp
    src/Core/Thread.cpp
    src/Core/Version.cpp

    src/IO/FileLoader.cpp

    src/Math/Curve.cpp
    src/Math/Shape.cpp

    src/Mixer/Resamplers/R8BrainResampler.h
    src/Mixer/Resamplers/LibsamplerateResampler.h
    src/Mixer/SoundProcessors/EffectProcessor.h
    src/Mixer/SoundProcessors/EnvironmentProcessor.h
    src/Mixer/SoundProcessors/ObstructionProcessor.h
    src/Mixer/SoundProcessors/OcclusionProcessor.h
    src/Mixer/SoundProcessors/PassThroughProcessor.h
    src/Mixer/Mixer.cpp
    src/Mixer/Mixer.h
    src/Mixer/ProcessorPipeline.cpp
    src/Mixer/ProcessorPipeline.h
    src/Mixer/RealChannel.cpp
    src/Mixer/RealChannel.h
    src/Mixer/Resampler.cpp
    src/Mixer/SoundData.cpp
    src/Mixer/SoundData.h
    src/Mixer/SoundProcessor.cpp

    src/Sound/Faders/ConstantFader.cpp
    src/Sound/Faders/ConstantFader.h
    src/Sound/Faders/ExponentialFader.cpp
    src/Sound/Faders/ExponentialFader.h
    src/Sound/Faders/LinearFader.cpp
    src/Sound/Faders/LinearFader.h
    src/Sound/Faders/SCurveFader.cpp
    src/Sound/Faders/SCurveFader.h
    src/Sound/Filters/BassBoostFilter.cpp
    src/Sound/Filters/BassBoostFilter.h
    src/Sound/Filters/BiquadResonantFilter.cpp
    src/Sound/Filters/BiquadResonantFilter.h
    src/Sound/Filters/DCRemovalFilter.cpp
    src/Sound/Filters/DCRemovalFilter.h
    src/Sound/Filters/DelayFilter.cpp
    src/Sound/Filters/DelayFilter.h
    src/Sound/Filters/EqualizerFilter.cpp
    src/Sound/Filters/EqualizerFilter.h
    src/Sound/Filters/FFTFilter.cpp
    src/Sound/Filters/FFTFilter.h
    src/Sound/Filters/FlangerFilter.cpp
    src/Sound/Filters/FlangerFilter.h
    src/Sound/Filters/FreeverbFilter.cpp
    src/Sound/Filters/FreeverbFilter.h
    src/Sound/Filters/LofiFilter.cpp
    src/Sound/Filters/LofiFilter.h
    src/Sound/Filters/RobotizeFilter.cpp
    src/Sound/Filters/RobotizeFilter.h
    src/Sound/Filters/WaveShaperFilter.cpp
    src/Sound/Filters/WaveShaperFilter.h
    src/Sound/Schedulers/RandomScheduler.cpp
    src/Sound/Schedulers/RandomScheduler.h
    src/Sound/Schedulers/SequenceScheduler.cpp
    src/Sound/Schedulers/SequenceScheduler.h
    src/Sound/Attenuation.cpp
    src/Sound/AttenuationShapes.cpp
    src/Sound/AttenuationShapes.h
    src/Sound/Collection.cpp
    src/Sound/Effect.cpp
    src/Sound/Fader.cpp
    src/Sound/Filter.cpp
    src/Sound/RefCounter.cpp
    src/Sound/Rtpc.cpp
    src/Sound/Sound.cpp
    src/Sound/SoundBank.cpp
    src/Sound/SoundObject.cpp
    src/Sound/Switch.cpp
    src/Sound/SwitchContainer.cpp

    src/Utils/Audio/Compression/ADPCM/ADPCM.cpp
    src/Utils/Audio/Compression/ADPCM/ADPCM.h
    src/Utils/Audio/Resampling/CDSPBlockConvolver.h
    src/Utils/Audio/Resampling/CDSPFIRFilter.h
    src/Utils/Audio/Resampling/CDSPFracInterpolator.h
    src/Utils/Audio/Resampling/CDSPHBDownsampler.h
    src/Utils/Audio/Resampling/CDSPHBUpsampler.h
    src/Utils/Audio/Resampling/CDSPProcessor.h
    src/Utils/Audio/Resampling/CDSPRealFFT.h
    src/Utils/Audio/Resampling/CDSPResampler.h
    src/Utils/Audio/Resampling/CDSPSincFilterGen.h
    src/Utils/Audio/Resampling/r8bbase.cpp
    src/Utils/Audio/Resampling/r8bbase.h
    src/Utils/Audio/Resampling/r8bconf.h
    src/Utils/Audio/Resampling/r8butil.h
    src/Utils/Freeverb/AllPass.cpp
    src/Utils/Freeverb/AllPass.h
    src/Utils/Freeverb/Comb.cpp
    src/Utils/Freeverb/Comb.h
    src/Utils/Freeverb/denormals.h
    src/Utils/Freeverb/ReverbModel.cpp
    src/Utils/Freeverb/ReverbModel.h
    src/Utils/Freeverb/tuning.h
    src/Utils/miniaudio/miniaudio.cpp
    src/Utils/miniaudio/miniaudio.h
    src/Utils/miniaudio/miniaudio_utils.h
    src/Utils/pffft/simd/pf_avx_double.h
    src/Utils/pffft/simd/pf_double.h
    src/Utils/pffft/simd/pf_neon_double_from_avx.h
    src/Utils/pffft/simd/pf_neon_double.h
    src/Utils/pffft/simd/pf_scalar_double.h
    src/Utils/pffft/simd/pf_sse2_double.h
    src/Utils/pffft/pffft_double.cpp
    src/Utils/pffft/pffft_double.h
    src/Utils/pffft/pffft_private_impl.h
    src/Utils/SmMalloc/smmalloc_generic.cpp
    src/Utils/SmMalloc/smmalloc_tls.cpp
    src/Utils/SmMalloc/smmalloc.cpp
    src/Utils/SmMalloc/smmalloc.h
    src/Utils/intrusive_list.h
    src/Utils/Utils.h

    src/IO/File.h
    src/IO/File.cpp
)

# Includes for this project
include_directories(src
    include
    ${FLATC_INCLUDE_DIR}
    ${SIMDPP_INCLUDE_DIR}
    ${LIBSAMPLERATE_INCLUDE_DIR}
    ${AM_FLATBUFFERS_GENERATED_INCLUDES_DIR}
)

add_library(${PROJECT_NAME} STATIC ${SA_SOURCE})

target_link_libraries(${PROJECT_NAME} PRIVATE flatbuffers)
target_link_libraries(${PROJECT_NAME} PRIVATE samplerate)

add_dependencies(${PROJECT_NAME}
    generated_includes
)

if(UNIX)
    find_package(Threads REQUIRED)

    target_link_libraries(${PROJECT_NAME}
        PUBLIC Threads::Threads
        ${CMAKE_DL_LIBS}
    )
endif()

add_subdirectory(tools/amac)

if(BUILD_SAMPLES)
  add_subdirectory(samples)
endif()

# #####################################################
# INSTALL
if(MSVC)
    set(RUNTIME_OUTPUT_DIRECTORY "win")
elseif(APPLE)
    set(RUNTIME_OUTPUT_DIRECTORY "osx")
else()
    set(RUNTIME_OUTPUT_DIRECTORY "linux")
endif()

set(AM_LIB_DESTINATION "sdk/lib/${RUNTIME_OUTPUT_DIRECTORY}")
set(AM_INC_DESTINATION "sdk/include")
set(AM_SHM_DESTINATION "sdk/schemas")
set(AM_BIN_DESTINATION "sdk/bin/${RUNTIME_OUTPUT_DIRECTORY}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${AM_BIN_DESTINATION}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${AM_LIB_DESTINATION}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${AM_BIN_DESTINATION}")

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR})

install(
    TARGETS ${PROJECT_NAME} amac samplerate
    EXPORT AmplitudeAudioSDKConfig
    ARCHIVE DESTINATION ${AM_LIB_DESTINATION}
    LIBRARY DESTINATION ${AM_LIB_DESTINATION}
    RUNTIME DESTINATION ${AM_BIN_DESTINATION}
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${AM_INC_DESTINATION}
    FILES_MATCHING PATTERN "*.h*"
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/schemas/
    DESTINATION ${AM_SHM_DESTINATION}
    FILES_MATCHING PATTERN "*.fbs"
)

install(
    EXPORT AmplitudeAudioSDKConfig
    DESTINATION "${AM_LIB_DESTINATION}/cmake"
    NAMESPACE SparkyStudios::Audio::
)

export(
    TARGETS flatbuffers ProjectConfig
    NAMESPACE flatbuffers::
    FILE "${CMAKE_CURRENT_BINARY_DIR}/FlatbuffersConfig.cmake"
)

export(
    TARGETS samplerate
    NAMESPACE SampleRate::
    FILE "${CMAKE_CURRENT_BINARY_DIR}/SampleRateConfig.cmake"
)

export(
    TARGETS ${PROJECT_NAME}
    NAMESPACE SparkyStudios::Audio::
    FILE "${CMAKE_CURRENT_BINARY_DIR}/AmplitudeAudioSDKConfig.cmake"
)

export(PACKAGE ${PROJECT_NAME})
