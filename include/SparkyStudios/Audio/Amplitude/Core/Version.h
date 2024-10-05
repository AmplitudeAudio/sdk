// Copyright (c) 2021-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#ifndef _AM_CORE_VERSION_H
#define _AM_CORE_VERSION_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

/**
 * @brief Gets the current Amplitude SDK version.
 *
 * @ingroup core
 */
#define amVersion SparkyStudios::Audio::Amplitude::GetVersion()

#define AM_VERSION_MAJOR 1
#define AM_VERSION_MINOR 0
#define AM_VERSION_PATCH 0

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief A structure containing the version number of the library.
     *
     * @ingroup core
     */
    struct AmVersion
    {
        /**
         * @brief Version number, updated only on major releases.
         */
        AmUInt8 major;

        /**
         * @brief Version number, updated only on point releases.
         */
        AmUInt8 minor;

        /**
         * @brief Version number, updated for tiny releases (ex: Bug fixes).
         */
        AmUInt8 revision;

        /**
         * @brief Text string containing the name and version of the library.
         */
        AmString text;
    };

    /**
     * @brief Returns the version.
     *
     * @return The current SDK version.
     *
     * @ingroup core
     */
    AM_API_PUBLIC const AmVersion& GetVersion();
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_VERSION_H
