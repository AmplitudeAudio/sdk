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

#ifndef SS_AMPLITUDE_AUDIO_EVENT_H
#define SS_AMPLITUDE_AUDIO_EVENT_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>
#include <SparkyStudios/Audio/Amplitude/Core/RefCounter.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct EventDefinition;
    struct EventActionDefinition;

    class Event;
    class EventInstance;

    /**
     * @brief An event action that will be applied when
     * the underlying event will be triggered.
     */
    class EventAction
    {
        friend class Event;

    public:
        EventAction();
        explicit EventAction(Event* parent);

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
        bool AdvanceFrame(AmTime delta_time);

    private:
        void ExecutePlay(const Entity& entity);
        void ExecutePause(const Entity& entity);
        void ExecuteResume(const Entity& entity);
        void ExecuteStop(const Entity& entity);
        void ExecuteSeek(const Entity& entity);
        void ExecuteMute(const Entity& entity, bool mute);

        bool _active;
        AmInt8 _type;
        AmInt8 _scope;
        std::vector<AmObjectID> _targets;

        Event* _parent;
    };

    /**
     * @brief An event is mainly used to apply a set of actions at a given time in game.
     *
     * This Event class is only referenced through an EventCanceler object and it is
     * managed by the Engine. Events can be triggered at runtime by calling the
     * <code>Engine::Trigger()</code> method using the name of the event.
     */
    class Event
    {
        friend class EventInstance;

    public:
        Event();

        /**
         * @brief Loads an event definition in this state.
         *
         * @param event The event definition to load.
         *
         * @return true if the event definition was successfully loaded.
         */
        bool LoadEventDefinition(const std::string& event);

        /**
         * @brief Loads an event definition from the given file in this state.
         *
         * @param filename The event definition file to load.
         *
         * @return true if the event definition file was successfully loaded.
         */
        bool LoadEventDefinitionFromFile(AmOsString filename);

        /**
         * @brief Triggers the event for the specified engine state.
         *
         * @param entity The which trigger the event.
         */
        [[nodiscard]] EventInstance Trigger(const Entity& entity) const;

        /**
         * @brief Returns the ID of this event.
         *
         * @return The ID of this event.
         */
        [[nodiscard]] AmEventID GetId() const;

        /**
         * @brief Returns the name of this event.
         *
         * @return The name of this event.
         */
        [[nodiscard]] const std::string& GetName() const;

        /**
         * @brief Get the event definition which generated this event.
         *
         * @return The event definition.
         */
        [[nodiscard]] const EventDefinition* GetEventDefinition() const;

        /**
         * @brief Get the reference counter associated to this event.
         *
         * @return The reference counter.
         */
        RefCounter* GetRefCounter();

    private:
        std::string _source;

        AmEventID _id;
        std::string _name;
        std::vector<EventAction> _actions;

        RefCounter _refCounter;
    };

    /**
     * @brief A class which can cancel an Event.
     */
    class EventCanceler
    {
    public:
        /**
         * @brief Creates an uninitialized EventCanceler.
         *
         * An uninitialized Event cannot be triggered.
         */
        EventCanceler();

        explicit EventCanceler(EventInstance* event);

        /**
         * @brief Checks whether this EventCanceler has been initialized.
         *
         * @return true if this EventCanceler has been initialized, false otherwise.
         */
        [[nodiscard]] bool Valid() const;

        /**
         * @brief Cancels and abort the wrapped Event.
         */
        void Cancel() const;

        /**
         * @brief Returns the Event wrapped by this EventCanceler.
         *
         * @return The Event wrapped by this EventCanceler.
         */
        [[nodiscard]] EventInstance* GetEvent() const;

    private:
        EventInstance* _event;
    };

    /**
     * @brief A triggered event.
     *
     * EventInstance are created when an Event is effectively triggered. They represent
     * the lifetime of that event at that particular time.
     */
    class EventInstance
    {
    public:
        EventInstance();
        explicit EventInstance(const Event* parent);

        /**
         * @brief Starts this Event.
         */
        void Start(const Entity& entity);

        /**
         * @brief Apply a frame update on this Event.
         *
         * @param delta_time The time elapsed since the last frame.
         */
        void AdvanceFrame(AmTime delta_time);

        /**
         * @brief Returns whether thisEvent is running.
         *
         * @return true if the event is running, false otherwise.
         */
        [[nodiscard]] bool IsRunning() const;

        /**
         * @brief Abort the execution of this Event.
         */
        void Abort();

    private:
        std::vector<EventAction> _actions;
        bool _running;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_EVENT_H
