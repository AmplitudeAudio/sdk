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

#ifndef _AM_IO_RESOURCE_H
#define _AM_IO_RESOURCE_H

#include <SparkyStudios/Audio/Amplitude/IO/FileSystem.h>

namespace SparkyStudios::Audio::Amplitude
{

    /**
     * @brief An Amplitude resource in a `FileSystem`.
     *
     * This base class represents a resource (sound files, assets, etc.) in a `FileSystem`.
     *
     * @ingroup io
     */
    class AM_API_PUBLIC Resource
    {
    public:
        /**
         * @brief Default destructor.
         */
        virtual ~Resource() = default;

        /**
         * @brief Gets the path to the resource.
         */
        [[nodiscard]] virtual const AmOsString& GetPath() const = 0;

        /**
         * @brief Loads the resource from the given FileSystem.
         */
        virtual void Load(const FileSystem* loader) = 0;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IO_RESOURCE_H