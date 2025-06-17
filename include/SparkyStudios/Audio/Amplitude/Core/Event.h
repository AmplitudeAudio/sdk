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
     * @brief Helper class used to cancel a running @c Event.
     *
     * @see Event
     *
     * @ingroup engine
     */
    class AM_API_PUBLIC EventCanceler
    {
    public:
        /**
         * @brief Creates an uninitialized event canceler.
         *
         * An uninitialized event canceler cannot be canceled.
         */
        EventCanceler();

        /**
         * @brief Creates an event canceler which will abort the given event once canceled.
         *
         * @param[in] event The event instance to cancel.
         */
        explicit EventCanceler(std::shared_ptr<EventInstance> event);

        /**
         * @brief Destroys the event canceler and releases the wrapped event instance.
         */
        ~EventCanceler();

        /**
         * @brief Checks whether this event canceler has been initialized.
         *
         * @return @c true if this event canceler has been initialized, @c false otherwise.
         */
        [[nodiscard]] bool Valid() const;

        /**
         * @brief Cancels and abort the wrapped @c Event.
         */
        void Cancel() const;

        /**
         * @brief Returns the @c EventInstance wrapped by this event canceler.
         *
         * @return The @c EventInstance wrapped by this event canceler.
         */
        [[nodiscard]] std::shared_ptr<EventInstance> GetEvent() const;

    private:
        /**
         * @brief The wrapped event instance.
         *
         * @internal
         */
        std::shared_ptr<EventInstance> _event;
    };

    /**
     * @brief Amplitude event asset.
     *
     * An event is used to apply a set of actions (synchronously or asynchronously) at a given time
     * in the game.
     *
     * This @c Event class is only referenced through an @c EventCanceler object, and it is
     * managed by the @c Engine. Events can be triggered at runtime by using the Engine::Trigger() method:
     *
     * @code{cpp}
     * amEngine->Trigger("an_event_name"); // You can also use the event ID, or its handle.
     * @endcode
     *
     * @ingroup assets
     */
    class AM_API_PUBLIC Event : public Asset<AmEventID>
    {
    };

    /**
     * @brief A triggered event.
     *
     * @c EventInstance objects are created when an @c Event is triggered. They represent
     * the lifetime of that event at that particular time.
     *
     * The internal state of an @c EventInstance is owned by that @c EventInstance. That means
     * each time you trigger an @c Event, a new instance with its own state is created.
     *
     * @see Event
     *
     * @ingroup engine
     */
    class AM_API_PUBLIC EventInstance
    {
    public:
        /**
         * @brief Default constructor.
         */
        virtual ~EventInstance() = default;

        /**
         * @brief Applies a frame update on this @c Event.
         *
         * This method is called once per frame to update the event instance's state.
         *
         * @param[in] deltaTime The time elapsed since the last frame.
         *
         * @warning This method is for internal usage only.
         */
        virtual void AdvanceFrame(AmTime deltaTime) = 0;

        /**
         * @brief Returns whether this @c EventInstance is running.
         *
         * @return @c true if the event is running, @c false otherwise.
         */
        [[nodiscard]] virtual bool IsRunning() const = 0;

        /**
         * @brief Aborts the execution of this event.
         */
        virtual void Abort() = 0;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_EVENT_H
