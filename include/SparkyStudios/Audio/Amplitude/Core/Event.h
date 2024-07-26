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

#ifndef _AM_CORE_EVENT_H
#define _AM_CORE_EVENT_H

#include <SparkyStudios/Audio/Amplitude/Core/Asset.h>
#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>
#include <SparkyStudios/Audio/Amplitude/Core/Playback/Channel.h>

namespace SparkyStudios::Audio::Amplitude
{
    class EventInstance;

    /**
     * @brief A class which can cancel a triggered Event.
     */
    class AM_API_PUBLIC EventCanceler
    {
    public:
        /**
         * @brief Creates an uninitialized EventCanceler.
         *
         * An uninitialized Event cannot be triggered.
         */
        EventCanceler();

        /**
         * @brief Creates an @c EventCanceler which will abort
         * the given event once cancelled.
         *
         * @param event The event instance to cancel.
         */
        explicit EventCanceler(EventInstance* event);

        /**
         * @brief Destroys the event canceller and releases
         * the wrapped event instance.
         */
        ~EventCanceler();

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
     * @brief An event is mainly used to apply a set of actions at a given time in game.
     *
     * This Event class is only referenced through an EventCanceler object and it is
     * managed by the Engine. Events can be triggered at runtime by calling the
     * <code>Engine::Trigger()</code> method using the name of the event.
     */
    class AM_API_PUBLIC Event : public Asset<AmEventID>
    {
    };

    /**
     * @brief A triggered event.
     *
     * EventInstance are created when an Event is effectively triggered. They represent
     * the lifetime of that event at that particular time.
     */
    class AM_API_PUBLIC EventInstance
    {
    public:
        virtual ~EventInstance() = default;

        /**
         * @brief Applies a frame update on this Event.
         *
         * @param deltaTime The time elapsed since the last frame.
         */
        virtual void AdvanceFrame(AmTime deltaTime) = 0;

        /**
         * @brief Returns whether thisEvent is running.
         *
         * @return true if the event is running, false otherwise.
         */
        [[nodiscard]] virtual bool IsRunning() const = 0;

        /**
         * @brief Aborts the execution of this Event.
         */
        virtual void Abort() = 0;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_EVENT_H
