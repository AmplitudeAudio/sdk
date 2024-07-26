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

#ifndef _AM_IMPLEMENTATION_SOUND_SWITCH_H
#define _AM_IMPLEMENTATION_SOUND_SWITCH_H

#include <SparkyStudios/Audio/Amplitude/Sound/Switch.h>

#include <Core/Asset.h>

#include "switch_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    class SwitchImpl final
        : public Switch
        , public AssetImpl<AmSwitchID, SwitchDefinition>
    {
    public:
        /**
         * @brief Creates an uninitialized Switch.
         *
         * An uninitialized Switch cannot set no provide the actual switch state.
         */
        SwitchImpl();

        /**
         * \brief Destroys the switch asset and releases all related resources.
         */
        ~SwitchImpl() override;

        /**
         * @copydoc Asset::GetId
         */
        [[nodiscard]] AM_INLINE AmSwitchID GetId() const override
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
         * @copydoc Switch::GetState
         */
        [[nodiscard]] const SwitchState& GetState() const override;

        /**
         * @copydoc Switch::SetState
         */
        void SetState(const SwitchState& state) override;

        /**
         * @copydoc Switch::SetState
         */
        void SetState(AmObjectID id) override;

        /**
         * @copydoc Switch::SetState
         */
        void SetState(const AmString& name) override;

        /**
         * @copydoc Switch::GetSwitchStates
         */
        [[nodiscard]] const std::vector<SwitchState>& GetSwitchStates() const override;

        /**
         * @copydoc AssetImpl::LoadDefinition
         */
        bool LoadDefinition(const SwitchDefinition* definition, EngineInternalState* state) override;

        /**
         * @copydoc AssetImpl::GetDefinition
         */
        [[nodiscard]] const SwitchDefinition* GetDefinition() const override;

    private:
        SwitchState _activeState;
        std::vector<SwitchState> _states;

        RefCounter _refCounter;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_SOUND_SWITCH_H
