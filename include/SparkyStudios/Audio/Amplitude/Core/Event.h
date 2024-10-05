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
     * @brief An helper class used to cancel a running `Event`.
     *
     * @see Event
     *
     * @ingroup engine
     */
    class AM_API_PUBLIC EventCanceler
    {
    public:
        /**
         * @brief Creates an uninitialized `EventCanceler`.
         *
         * An uninitialized `EventCanceler` cannot be canceled.
         */
        EventCanceler();

        /**
         * @brief Creates an `EventCanceler` which will abort
         * the given event once cancelled.
         *
         * @param[in] event The event instance to cancel.
         */
        explicit EventCanceler(EventInstance* event);

        /**
         * @brief Destroys the event canceller and releases
         * the wrapped event instance.
         */
        ~EventCanceler();

        /**
         * @brief Checks whether this `EventCanceler` has been initialized.
         *
         * @return `true` if this `EventCanceler` has been initialized, `false` otherwise.
         */
        [[nodiscard]] bool Valid() const;

        /**
         * @brief Cancels and abort the wrapped `Event`.
         */
        void Cancel() const;

        /**
         * @brief Returns the `Event` wrapped by this `EventCanceler`.
         *
         * @return The `Event` wrapped by this `EventCanceler`.
         */
        [[nodiscard]] EventInstance* GetEvent() const;

    private:
        /**
         * @brief The wrapped event instance.
         *
         * @internal
         */
        EventInstance* _event;
    };

    /**
     * @brief Amplitude Event Asset.
     *
     * An event is used to apply a set of actions (synchronously or asynchronously) at a given time
     * in the game.
     *
     * This `Event` class is only referenced through an `EventCanceler` object and it is
     * managed by the `Engine`. Events can be triggered at runtime by using the `Trigger()`
     * method of the `Engine` instance:
     * ```cpp
     * amEngine->Trigger("an_event_name"); // You can also use the event ID, or its handle.
     * ```
     *
     * @ingroup assets
     */
    class AM_API_PUBLIC Event : public Asset<AmEventID>
    {
    };

    /**
     * @brief A triggered event.
     *
     * `EventInstance` objects are created when an `Event` is triggered. They represent
     * the lifetime of that event at that particular time.
     *
     * The internal state of an `EventInstance` is owned by that `EventInstance`, that means
     * each time you trigger an `Event`, a new instance with its own state is created.
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
         * @brief Applies a frame update on this `Event`.
         *
         * This method is called once per frame to update the event instance's state.
         *
         * @param[in] deltaTime The time elapsed since the last frame.
         *
         * @warning This method is for internal usage only.
         */
        virtual void AdvanceFrame(AmTime deltaTime) = 0;

        /**
         * @brief Returns whether this `EventInstance` is running.
         *
         * @return `true` if the event is running, `false` otherwise.
         */
        [[nodiscard]] virtual bool IsRunning() const = 0;

        /**
         * @brief Aborts the execution of this `Event`.
         */
        virtual void Abort() = 0;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_EVENT_H
