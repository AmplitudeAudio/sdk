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

#ifndef SS_AMPLITUDE_AUDIO_ASSET_H
#define SS_AMPLITUDE_AUDIO_ASSET_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Core/RefCounter.h>

#include <SparkyStudios/Audio/Amplitude/IO/File.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct EngineInternalState;

    /**
     * @brief Amplitude Asset.
     *
     * This is the base class for all Amplitude assets. An Amplitude asset is a
     * .json file with a specific format (definition), which is specified by the
     * corresponding asset flatbuffer schema.
     *
     * @tparam Id The type of the asset id.
     * @tparam Definition The type of the asset definition.
     */
    template<typename Id, class Definition>
    class AM_API_PUBLIC Asset
    {
    public:
        /**
         * @brief Destroys the asset and all related resources.
         */
        virtual ~Asset();

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
         * @brief Returns the unique ID of this asset.
         *
         * @return The asset unique ID.
         */
        [[nodiscard]] virtual Id GetId() const;

        /**
         * @brief Gets the name of this asset.
         *
         * @return The asset name.
         */
        [[nodiscard]] virtual const AmString& GetName() const;

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
        RefCounter* GetRefCounter();

        /**
         * @brief Gets the asset reference counter.
         *
         * @return The reference counter instance.
         */
        [[nodiscard]] const RefCounter* GetRefCounter() const;

    protected:
        AmString _source;

        AmString _name;
        AmObjectID _id = kAmInvalidObjectId;

        RefCounter _refCounter;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_ASSET_H
