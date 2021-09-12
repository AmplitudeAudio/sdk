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

#ifndef SS_AMPLITUDE_AUDIO_SWITCHCONTAINER_H
#define SS_AMPLITUDE_AUDIO_SWITCHCONTAINER_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/RefCounter.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Attenuation.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Fader.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Switch.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct SwitchContainerDefinition;

    class BusInternalState;
    struct EngineInternalState;

    struct SwitchContainerItem
    {
        AmObjectID m_id;
        bool m_continueBetweenStates;
        AmTime m_fadeInDuration;
        AmUInt8 m_fadeInAlgorithm;
        AmTime m_fadeOutDuration;
        AmUInt8 m_fadeOutAlgorithm;
        AmReal32 m_gain;
    };

    class SwitchContainer
    {
    public:
        SwitchContainer();
        ~SwitchContainer();

        bool LoadSwitchContainerDefinition(const std::string& source, EngineInternalState* state);
        bool LoadSwitchContainerDefinitionFromFile(AmOsString filename, EngineInternalState* state);
        void AcquireReferences(EngineInternalState* state);
        void ReleaseReferences(EngineInternalState* state);
        [[nodiscard]] const SwitchContainerDefinition* GetSwitchContainerDefinition() const;

        /**
         * @brief Returns the unique ID of this SwitchContainer.
         *
         * @return The SwitchContainer unique ID.
         */
        [[nodiscard]] AmCollectionID GetId() const;

        /**
         * @brief Returns the name of this SwitchContainer.
         *
         * @return The SwitchContainer name.
         */
        [[nodiscard]] const std::string& GetName() const;

        /**
         * @brief Return the bus this SwitchContainer will play on.
         *
         * @return The bus this SwitchContainer will play on.
         */
        [[nodiscard]] BusInternalState* GetBus() const;

        /**
         * @brief Returns the attenuation attached to this SwitchContainer.
         *
         * @return The attenuation of this SwitchContainer if available or nullptr.
         */
        [[nodiscard]] const Attenuation* GetAttenuation() const;

        /**
         * @brief Returns the switch attached to this SwitchContainer.
         *
         * @return The switch of this SwitchContainer if available or nullptr.
         */
        [[nodiscard]] const Switch* GetSwitch() const;

        /**
         * @brief Get the fade in Fader for the given sound object ID.
         *
         * @param id The ID of the sound object.
         *
         * @return The fade in Fader.
         */
        Fader* GetFaderIn(AmObjectID id) const;

        /**
         * @brief Get the fade out Fader for the given sound object ID.
         *
         * @param id The ID of the sound object.
         *
         * @return The fade out Fader.
         */
        Fader* GetFaderOut(AmObjectID id) const;

        /**
         * @brief Get the references counter of this instance.
         *
         * @return The references counter.
         */
        RefCounter* GetRefCounter();

        /**
         * @brief Returns the list of sound objects referenced in this SwitchContainer for the given state.
         *
         * @param state The switch state to get the objects for.
         *
         * @return The list of sound object IDs.
         */
        [[nodiscard]] const std::vector<SwitchContainerItem>& GetSoundObjects(AmObjectID stateId) const;

    private:
        // The bus this SwitchContainer will play on.
        BusInternalState* _bus;

        // The World scope sound scheduler
        Switch* _switch;

        std::string _source;
        std::map<AmObjectID, std::vector<SwitchContainerItem>> _sounds;
        std::map<AmObjectID, Fader*> _fadersIn;
        std::map<AmObjectID, Fader*> _fadersOut;

        AmSwitchContainerID _id;
        std::string _name;

        Attenuation* _attenuation;

        RefCounter _refCounter;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_SWITCHCONTAINER_H
