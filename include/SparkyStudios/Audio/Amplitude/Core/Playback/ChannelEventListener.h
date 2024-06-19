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

#ifndef SS_AMPLITUDE_AUDIO_EVENTLISTENER_H
#define SS_AMPLITUDE_AUDIO_EVENTLISTENER_H

#include <functional>
#include <list>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    class ChannelInternalState;

    /**
     * @brief The event info passed to the event listener.
     */
    struct ChannelEventInfo
    {
        explicit ChannelEventInfo(ChannelInternalState* source);

        /**
         * @brief The event source.
         */
        ChannelInternalState* m_source;

        /**
         * @brief Additional user data passed to the event listener.
         */
        void* m_userData;
    };

    typedef std::function<void(ChannelEventInfo)> ChannelEventCallback;

    /**
     * @brief Channel Event listener.
     *
     * Event handlers are registered to the event listener through a callback to receive
     * event notifications.
     */
    class AM_API_PUBLIC ChannelEventListener
    {
    public:
        ChannelEventListener();
        ~ChannelEventListener();

        /**
         * @brief Registers a custom callback to this event listener.
         *
         * @param callback The event callback.
         * @param userData The additional data to pass to the handler when this listener receive the event.
         */
        void Add(const ChannelEventCallback& callback, void* userData = nullptr);

        /**
         * @brief Executes the event by calling all the registered event handlers.
         *
         * @param channel The channel which have triggered the event.
         */
        void Call(ChannelInternalState* channel);

    private:
        void AddInternal(const ChannelEventCallback& callback, void* userData = nullptr);
        void CleanLinks();

        struct HandlerInstance
        {
            ChannelEventCallback m_callback;
            void* m_userData;
        };

        std::list<HandlerInstance> _handlers;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_EVENTLISTENER_H
