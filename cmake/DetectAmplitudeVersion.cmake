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

file(READ "${CMAKE_CURRENT_LIST_DIR}/../include/SparkyStudios/Audio/Amplitude/Core/Version.h" AM_VERSION_CPP_CONTENT)

string(REGEX REPLACE ".*#define AM_VERSION_MAJOR ([0-9]+).*" "\\1" DETECTED_AM_VERSION_MAJOR "${AM_VERSION_CPP_CONTENT}")
string(REGEX REPLACE ".*#define AM_VERSION_MINOR ([0-9]+).*" "\\1" DETECTED_AM_VERSION_MINOR "${AM_VERSION_CPP_CONTENT}")
string(REGEX REPLACE ".*#define AM_VERSION_PATCH ([0-9]+).*" "\\1" DETECTED_AM_VERSION_PATCH "${AM_VERSION_CPP_CONTENT}")

set(DETECTED_AM_VERSION "${DETECTED_AM_VERSION_MAJOR}.${DETECTED_AM_VERSION_MINOR}.${DETECTED_AM_VERSION_PATCH}")

message(STATUS "Detected Amplitude Audio SDK Version - ${DETECTED_AM_VERSION}")
