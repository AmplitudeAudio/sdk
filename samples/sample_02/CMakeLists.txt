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

cmake_minimum_required(VERSION 3.3.1)

set(PROJECT_NAME ss_amplitude_audio_sample_02)

project(${PROJECT_NAME})

find_package(SDL2 CONFIG REQUIRED)

set(ss_amplitude_audio_sample_SRCS
    main.cpp
)

add_executable(${PROJECT_NAME} ${ss_amplitude_audio_sample_SRCS})

target_link_libraries(${PROJECT_NAME}
    PRIVATE $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main> $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    PUBLIC Static
)

add_dependencies(${PROJECT_NAME}
    Static
    ss_amplitude_audio_sample_assets
)
