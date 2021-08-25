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
project(ss_amplitude_audio)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(OBJ_DIR ${CMAKE_CURRENT_BINARY_DIR}/obj)

# Include FlatBuffers in this project.
set(FLATBUFFERS_BUILD_TESTS OFF CACHE BOOL "")
if(NOT TARGET flatc)
    add_subdirectory("dependencies/flatbuffers" ${OBJ_DIR}/flatbuffers)
    set(FLATC_LIBRARY flatc)
    set(FLATC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/flatbuffers/include)
endif()

# Generate source files for all FlatBuffers schema files under the src
# directory.
set(SPARK_AUDIO_FLATBUFFERS_GENERATED_INCLUDES_DIR ${OBJ_DIR}/include/SparkyStudios/Audio/Amplitude)
file(GLOB_RECURSE FLATBUFFERS_SCHEMAS ${CMAKE_CURRENT_SOURCE_DIR}/schemas/*.fbs)

# Generate rules to build the set of output files from the set of input
# schema files.
build_flatbuffers("${FLATBUFFERS_SCHEMAS}"
    ""
    generated_includes
    ""
    ${SPARK_AUDIO_FLATBUFFERS_GENERATED_INCLUDES_DIR}
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
    include/SparkyStudios/Audio/Amplitude/Core/Common/Types.h
    include/SparkyStudios/Audio/Amplitude/Core/Bus.h
    include/SparkyStudios/Audio/Amplitude/Core/Channel.h
    include/SparkyStudios/Audio/Amplitude/Core/Codec.h
    include/SparkyStudios/Audio/Amplitude/Core/Common.h
    include/SparkyStudios/Audio/Amplitude/Core/Driver.h
    include/SparkyStudios/Audio/Amplitude/Core/Engine.h
    include/SparkyStudios/Audio/Amplitude/Core/Entity.h
    include/SparkyStudios/Audio/Amplitude/Core/Event.h
    include/SparkyStudios/Audio/Amplitude/Core/Fader.h
    include/SparkyStudios/Audio/Amplitude/Core/Listener.h
    include/SparkyStudios/Audio/Amplitude/Core/Log.h
    include/SparkyStudios/Audio/Amplitude/Core/RefCounter.h
    include/SparkyStudios/Audio/Amplitude/Core/Thread.h
    include/SparkyStudios/Audio/Amplitude/Core/Version.h
    include/SparkyStudios/Audio/Amplitude/IO/FileLoader.h
    include/SparkyStudios/Audio/Amplitude/Math/HandmadeMath.h
    include/SparkyStudios/Audio/Amplitude/Sound/Schedulers/SequenceScheduler.h
    include/SparkyStudios/Audio/Amplitude/Sound/Schedulers/RandomScheduler.h
    include/SparkyStudios/Audio/Amplitude/Sound/Scheduler.h
    include/SparkyStudios/Audio/Amplitude/Sound/Sound.h
    include/SparkyStudios/Audio/Amplitude/Sound/SoundBank.h
    include/SparkyStudios/Audio/Amplitude/Sound/SoundCollection.h
    include/SparkyStudios/Audio/Amplitude/Amplitude.h

    src/Core/Codecs/WAV/Codec.cpp
    src/Core/Codecs/WAV/Codec.h
    src/Core/Codecs/WAV/dr_wav.h

    src/Core/Codecs/OGG/Codec.cpp
    src/Core/Codecs/OGG/Codec.h
    src/Core/Codecs/OGG/stb_vorbis.c
    src/Core/Codecs/OGG/stb_vorbis.h

    src/Core/Drivers/MiniAudio/Driver.cpp
    src/Core/Drivers/MiniAudio/Driver.h
    src/Core/Drivers/MiniAudio/miniaudio.h

    src/Core/Bus.cpp
    src/Core/BusInternalState.cpp
    src/Core/BusInternalState.h
    src/Core/Channel.cpp
    src/Core/ChannelInternalState.cpp
    src/Core/ChannelInternalState.h
    src/Core/Codec.cpp
    src/Core/Common.cpp
    src/Core/Driver.cpp
    src/Core/Engine.cpp
    src/Core/EngineInternalState.h
    src/Core/Entity.cpp
    src/Core/EntityInternalState.cpp
    src/Core/EntityInternalState.h
    src/Core/Event.cpp
    src/Core/Fader.cpp
    src/Core/ListenerInternalState.h
    src/Core/Listener.cpp
    src/Core/Log.cpp
    src/Core/Thread.cpp
    src/Core/Version.cpp

    src/IO/FileLoader.cpp

    src/Math/HMM_Impl.cpp

    src/Mixer/Atomix/atomix.h
    src/Mixer/Atomix/Mixer.cpp
    src/Mixer/Atomix/Mixer.h
    src/Mixer/Atomix/RealChannel.cpp
    src/Mixer/Atomix/RealChannel.h
    src/Mixer/Atomix/Sound.cpp

    src/Sound/Schedulers/SequenceScheduler.cpp
    src/Sound/Schedulers/RandomScheduler.cpp
    src/Sound/SoundCollection.cpp
    src/Sound/SoundBank.cpp
    src/Sound/RefCounter.cpp

    src/Utils/intrusive_list.h

    src/IO/File.h
    src/IO/File.cpp
)

# Includes for this project
include_directories(src
    include
    src/Mixer/Atomix
    ${FLATC_INCLUDE_DIR}
)

add_library(ss_amplitude_audio ${SA_SOURCE})

add_dependencies(ss_amplitude_audio
    generated_includes
)

add_subdirectory(samples)
