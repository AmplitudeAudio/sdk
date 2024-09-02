// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#ifndef _AM_IMPLEMENTATION_CORE_ASSET_H
#define _AM_IMPLEMENTATION_CORE_ASSET_H

#include <SparkyStudios/Audio/Amplitude/Core/Asset.h>
#include <SparkyStudios/Audio/Amplitude/Core/RefCounter.h>
#include <SparkyStudios/Audio/Amplitude/IO/File.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct EngineInternalState;

    /**
     * @brief Asset class' private implementation.
     *
     * @tparam Definition The type of the asset definition.
     */
    template<typename Id, class Definition>
    class AssetImpl : public Asset<Id>
    {
    public:
        ~AssetImpl() override;

        /**
         * @copydoc Asset::GetId
         */
        [[nodiscard]] AM_INLINE Id GetId() const override
        {
            return m_id;
        }

        /**
         * @copydoc Asset::GetName
         */
        [[nodiscard]] AM_INLINE const AmString& GetName() const override
        {
            return m_name;
        }

        /**
         * @brief Load the asset from the given definition instance.
         *
         * @param definition The asset definition.
         * @param state The engine internal state.
         *
         * @return @c true on success, @c false otherwise.
         */
        virtual bool LoadDefinition(const Definition* definition, EngineInternalState* state) = 0;

        /**
         * @brief Load the asset from the given definition file.
         *
         * @param file The asset definition file.
         * @param state The engine internal state.
         *
         * @return @c true on success, @c false otherwise.
         */
        virtual bool LoadDefinitionFromFile(std::shared_ptr<File> file, EngineInternalState* state);

        /**
         * @brief Load the asset from the definition file at the given path.
         *
         * @param path The asset definition file path.
         * @param state The engine internal state.
         *
         * @return @c true on success, @c false otherwise.
         */
        virtual bool LoadDefinitionFromPath(const AmOsString& path, EngineInternalState* state);

        /**
         * @brief Gets the asset definition instance.
         *
         * This method works only if the asset has been successfully loaded.
         *
         * @return The asset definition instance.
         */
        [[nodiscard]] virtual const Definition* GetDefinition() const = 0;

        /**
         * @brief Acquires referenced objects in this asset.
         *
         * @param state The engine state used while loading the asset.
         */
        virtual void AcquireReferences(EngineInternalState* state);

        /**
         * @brief Releases the references acquired when loading the asset.
         *
         * @param state The engine state used while loading the asset.
         */
        virtual void ReleaseReferences(EngineInternalState* state);

        /**
         * @brief Gets the asset reference counter.
         *
         * @return The reference counter instance.
         */
        AM_INLINE RefCounter* GetRefCounter()
        {
            return &m_refCounter;
        }

        /**
         * @brief Gets the asset reference counter.
         *
         * @return The reference counter instance.
         */
        [[nodiscard]] AM_INLINE const RefCounter* GetRefCounter() const
        {
            return &m_refCounter;
        }

    protected:
        AmString m_name;
        AmObjectID m_id = kAmInvalidObjectId;

        AmString m_source;
        RefCounter m_refCounter;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_CORE_ASSET_H
