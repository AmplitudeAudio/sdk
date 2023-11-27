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

#include <SparkyStudios/Audio/Amplitude/Sound/Fader.h>
#include <SparkyStudios/Audio/Amplitude/Sound/SoundObject.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Switch.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct EngineInternalState;
    class BusInternalState;

    struct SwitchContainerDefinition;

    /**
     * @brief Describes a single item within a SwitchContainer.
     */
    struct SwitchContainerItem
    {
        /**
         * @brief The object ID of the item.
         *
         * May be a @a AmSoundID or a @a AmCollectionID.
         */
        AmObjectID m_id;

        /**
         * @brief Whether to continue playing this item when the SwitchContainer
         * changes its state between one of the values where this item is registered.
         *
         * If this value is set to @c false, each the sound will be stopped and played again
         * from the begginning.
         */
        bool m_continueBetweenStates;

        /**
         * @brief The fade duration in milliseconds when this item starts playing.
         */
        AmTime m_fadeInDuration;

        /**
         * @brief The name of the fading algorithm to use when this item starts playing.
         */
        AmString m_fadeInAlgorithm;

        /**
         * @brief The fade duration in milliseconds when this item stops playing.
         */
        AmTime m_fadeOutDuration;

        /**
         * @brief The name of the fading algorithm to use when this item stops playing.
         */
        AmString m_fadeOutAlgorithm;

        /**
         * @brief The custom gain applied on this item.
         *
         * The final gain will be computed with this value multiplied with the gain of the
         * attenuation model, if any.
         */
        RtpcValue m_gain;
    };

    /**
     * @brief Amplitude Switch Container.
     *
     * A switch container is a container sound object where sounds and collections can be registered on
     * one or multiple switches. Only one switch can be active at a time in a switch container. When a
     * switch is active, all the sounds and collections that are registered on it will be played.
     */
    class AM_API_PUBLIC SwitchContainer final
        : public Asset<AmSwitchContainerID, SwitchContainerDefinition>
        , public SoundObject
    {
    public:
        /**
         * @brief Creates an uninitialized switch container.
         */
        SwitchContainer();

        /**
         * @brief Destroys the switch container asset and all related resources.
         */
        ~SwitchContainer() override;

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
        [[nodiscard]] FaderInstance* GetFaderIn(AmObjectID id) const;

        /**
         * @brief Get the fade out Fader for the given sound object ID.
         *
         * @param id The ID of the sound object.
         *
         * @return The fade out Fader.
         */
        [[nodiscard]] FaderInstance* GetFaderOut(AmObjectID id) const;

        /**
         * @brief Returns the list of sound objects referenced in this SwitchContainer for the given state.
         *
         * @param stateId The switch state to get the objects for.
         *
         * @return The list of sound object IDs registered to the given state.
         */
        [[nodiscard]] const std::vector<SwitchContainerItem>& GetSoundObjects(AmObjectID stateId) const;

        bool LoadDefinition(const SwitchContainerDefinition* definition, EngineInternalState* state) override;
        [[nodiscard]] const SwitchContainerDefinition* GetDefinition() const override;
        void AcquireReferences(EngineInternalState* state) override;
        void ReleaseReferences(EngineInternalState* state) override;

    private:
        Switch* _switch;

        std::map<AmObjectID, std::vector<SwitchContainerItem>> _sounds;
        std::map<AmObjectID, std::tuple<Fader*, FaderInstance*>> _fadersIn;
        std::map<AmObjectID, std::tuple<Fader*, FaderInstance*>> _fadersOut;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_SWITCHCONTAINER_H
