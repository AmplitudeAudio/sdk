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

#ifndef _AM_IMPLEMENTATION_CORE_EVENT_H
#define _AM_IMPLEMENTATION_CORE_EVENT_H

#include <SparkyStudios/Audio/Amplitude/Core/Event.h>

#include <Core/Asset.h>

#include "event_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    class EventImpl;

    /**
     * @brief An event action that will be applied when
     * the parent event will be triggered.
     */
    class EventAction
    {
        friend class EventImpl;

    public:
        EventAction();
        explicit EventAction(EventImpl* parent);

        /**
         * @brief Initialize the event action from a definition.
         *
         * @param definition The definition to load data from.
         */
        void Initialize(const EventActionDefinition* definition);

        /**
         * @brief Runs the action in entity scope.
         *
         * @param entity The entity which trigger the event.
         */
        void Run(const Entity& entity);

        /**
         * @brief Runs the action in world scope.
         */
        void Run();

        /**
         * @brief Apply a frame update on this EventAction.
         *
         * @param delta_time The time elapsed since the last frame.
         *
         * @return true if the event action should be updated on the next frame, false otherwise.
         */
        void AdvanceFrame(AmTime delta_time);

        /**
         * @brief Checks if the event action is active and currently executing.
         *
         * @return true if the event action is active and currently executing, false otherwise.
         */
        [[nodiscard]] bool IsExecuting() const;

    private:
        void ExecutePlay(const Entity& entity);
        void ExecutePause(const Entity& entity);
        void ExecuteResume(const Entity& entity);
        void ExecuteStop(const Entity& entity);
        void ExecuteSeek(const Entity& entity);
        void ExecuteMute(const Entity& entity, bool mute);
        void ExecuteWait(const Entity& entity);

        bool _active;
        EventActionType _type;
        Scope _scope;
        std::vector<AmObjectID> _targets;

        std::vector<Channel> _playingChannels;
        AmTime _accumulatedTime;

        EventImpl* _parent;
    };

    class EventInstanceImpl final : public EventInstance
    {
        friend class EventImpl;

    public:
        EventInstanceImpl();

        explicit EventInstanceImpl(const EventImpl* parent);

        /**
         * @copydoc EventInstance::AdvanceFrame
         */
        void AdvanceFrame(AmTime deltaTime) override;

        /**
         * @copydoc EventInstance::IsRunning
         */
        [[nodiscard]] bool IsRunning() const override;

        /**
         * @copydoc EventInstance::Abort
         */
        void Abort() override;

    private:
        /**
         * @brief Starts this Event.
         */
        void Start(const Entity& entity);

        EventActionRunningMode _runMode;
        std::vector<EventAction> _actions;
        bool _running;

        size_t _runningActionIndex;
        Entity _entity;
    };

    class EventImpl final
        : public Event
        , public AssetImpl<AmEventID, EventDefinition>
    {
        friend class EventInstanceImpl;

    public:
        /**
         * @brief Creates an unitialized event.
         */
        EventImpl();

        /**
         * @brief Destroys the event asset and release all related resources.
         */
        ~EventImpl() override;

        /**
         * @copydoc Asset::GetId
         */
        [[nodiscard]] AM_INLINE AmEventID GetId() const override
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
         * @brief Triggers the event for the specified engine state.
         *
         * @param entity The which trigger the event.
         */
        [[nodiscard]] EventInstanceImpl Trigger(const Entity& entity) const;

        /**
         * @copydoc AssetImpl::LoadDefinition
         */
        bool LoadDefinition(const EventDefinition* definition, EngineInternalState* state) override;

        /**
         * @copydoc AssetImpl::GetDefinition
         */
        [[nodiscard]] const EventDefinition* GetDefinition() const override;

    private:
        EventActionRunningMode _runMode;
        std::vector<EventAction> _actions;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_CORE_EVENT_H
