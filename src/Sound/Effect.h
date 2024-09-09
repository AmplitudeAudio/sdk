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

#ifndef _AM_IMPLEMENTATION_SOUND_EFFECT_H
#define _AM_IMPLEMENTATION_SOUND_EFFECT_H

#include <SparkyStudios/Audio/Amplitude/DSP/Filter.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Effect.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Rtpc.h>

#include <Core/Asset.h>

#include "effect_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    class EffectInstance;

    class EffectImpl final
        : public Effect
        , public AssetImpl<AmEffectID, EffectDefinition>
    {
        friend class EffectInstanceImpl;

    public:
        /**
         * @brief Creates an uninitialized Effect.
         */
        EffectImpl();

        /**
         * \brief Destroys the Effect asset and release all associated resources.
         */
        ~EffectImpl() override;

        /**
         * @copydoc Effect::CreateInstance
         */
        [[nodiscard]] EffectInstance* CreateInstance() const override;

        /**
         * @copydoc Effect::DestroyInstance
         */
        void DestroyInstance(EffectInstance* instance) const override;

        /**
         * @copydoc Asset::GetId
         */
        [[nodiscard]] AM_INLINE AmEffectID GetId() const override
        {
            return AssetImpl::GetId();
        }

        /**
         * @copydoc Asset::GetName
         */
        [[nodiscard]] AM_INLINE const AmString& GetName() const override
        {
            return AssetImpl::GetName();
        }

        /**
         * @copydoc Effect::Update
         */
        void Update() override;

        /**
         * @copydoc AssetImpl::LoadDefinition
         */
        bool LoadDefinition(const EffectDefinition* definition, EngineInternalState* state) override;

        /**
         * @copydoc AssetImpl::GetDefinition
         */
        [[nodiscard]] const EffectDefinition* GetDefinition() const override;

    private:
        mutable std::vector<EffectInstance*> _instances;
        std::vector<RtpcValue> _parameters;
        Filter* _filter;
    };

    class EffectInstanceImpl final : public EffectInstance
    {
    public:
        /**
         * @brief Creates a new EffectInstance.
         *
         * @param parent The parent Effect asset.
         */
        explicit EffectInstanceImpl(const EffectImpl* parent);

        /**
         * @brief Destroys the EffectInstance.
         */
        ~EffectInstanceImpl() override;

        /**
         * @brief Get the filter instance wrapped by this effect.
         *
         * @return The filter instance.
         */
        [[nodiscard]] FilterInstance* GetFilter() const override;

    private:
        const EffectImpl* _parent;
        FilterInstance* _filterInstance;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_SOUND_EFFECT_H
