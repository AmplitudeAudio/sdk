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

#ifndef _AM_IMPLEMENTATION_SOUND_RTPC_H
#define _AM_IMPLEMENTATION_SOUND_RTPC_H

#include <SparkyStudios/Audio/Amplitude/Sound/Rtpc.h>

#include <Core/Asset.h>

#include "rtpc_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    class RtpcImpl final
        : public Rtpc
        , public AssetImpl<AmRtpcID, RtpcDefinition>
    {
    public:
        /**
         * @brief Creates an unitialized Rtpc object.
         *
         * An unitialized Rtpc object cannot be used to update values.
         */
        RtpcImpl();

        /**
         * @brief Destroys the Rtpc asset and releases all associated resources.
         */
        ~RtpcImpl() override;

        /**
         * @copydoc Asset::GetId
         */
        [[nodiscard]] AM_INLINE AmRtpcID GetId() const override
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
         * @copydoc Rtpc::Update
         */
        void Update(AmTime deltaTime) override;

        /**
         * @copydoc Rtpc::GetMinValue
         */
        [[nodiscard]] AM_INLINE AmReal64 GetMinValue() const override
        {
            return _minValue;
        }

        /**
         * @copydoc Rtpc::GetMaxValue
         */
        [[nodiscard]] AM_INLINE AmReal64 GetMaxValue() const override
        {
            return _maxValue;
        }

        /**
         * @copydoc Rtpc::GetValue
         */
        [[nodiscard]] AM_INLINE AmReal64 GetValue() const override
        {
            return _currentValue;
        }

        /**
         * @copydoc Rtpc::SetValue
         */
        void SetValue(AmReal64 value) override;

        /**
         * @copydoc Rtpc::GetDefaultValue
         */
        [[nodiscard]] AM_INLINE AmReal64 GetDefaultValue() const override
        {
            return _defValue;
        }

        /**
         * @copydoc Rtpc::Reset
         */
        void Reset() override;

        /**
         * @copydoc AssetImpl::LoadDefinition
         */
        bool LoadDefinition(const RtpcDefinition* definition, EngineInternalState* state) override;

        /**
         * @copydoc AssetImpl::GetDefinition
         */
        [[nodiscard]] const RtpcDefinition* GetDefinition() const override;

    private:
        AmReal64 _minValue;
        AmReal64 _maxValue;
        AmReal64 _defValue;

        AmReal64 _currentValue;
        AmReal64 _targetValue;

        Fader* _faderAttackFactory;
        Fader* _faderReleaseFactory;

        FaderInstance* _faderAttack;
        FaderInstance* _faderRelease;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_SOUND_RTPC_H
