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

#ifndef _AM_CORE_ASSET_H
#define _AM_CORE_ASSET_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Amplitude Asset.
     *
     * This is the base class for all Amplitude assets. An Amplitude asset is a
     * `.json` file with a specific format (definition) specified by the corresponding
     * asset's flatbuffer schema.
     *
     * @tparam Id The type of the asset id.
     *
     * @ingroup assets
     */
    template<typename Id>
    class AM_API_PUBLIC Asset
    {
    public:
        /**
         * @brief Destroys the asset and all related resources.
         */
        virtual ~Asset() = default;

        /**
         * @brief Returns the unique ID of this asset.
         *
         * @return The asset unique ID.
         */
        [[nodiscard]] virtual Id GetId() const = 0;

        /**
         * @brief Gets the name of this asset.
         *
         * @return The asset name.
         */
        [[nodiscard]] virtual const AmString& GetName() const = 0;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_ASSET_H
