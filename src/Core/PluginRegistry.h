// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#ifndef _AM_IMPLEMENTATION_CORE_PLUGIN_REGISTRY_H
#define _AM_IMPLEMENTATION_CORE_PLUGIN_REGISTRY_H

#include <functional>

#include <SparkyStudios/Audio/Amplitude/Core/Plugin.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Checks whether the engine instance has a running state.
     *
     * Plugins, and their frame callbacks, can only change while it does not. Defined in Engine.cpp.
     */
    bool IsEngineRunning();

    /**
     * @brief The plugins registered with the engine, static and dynamic alike, and their frame callbacks.
     *
     * Registration only happens while the engine is not running, so the frame reads the callbacks without locking.
     */
    namespace PluginRegistry
    {
        /**
         * @brief What a plugin loaded from a shared library brings along with its descriptor.
         */
        struct LibraryInfo
        {
            /**
             * @brief The path of the library the plugin was loaded from.
             */
            AmOsString path;

            /**
             * @brief The native handle of the library.
             */
            AmVoidPtr nativeHandle = nullptr;

            /**
             * @brief Closes the library. Called after the plugin is unregistered.
             */
            std::function<void()> close;
        };

        /**
         * @brief Checks the descriptor, then calls the plugin's @c Register function.
         *
         * @param[in] descriptor The plugin descriptor.
         * @param[in] library The library the plugin was loaded from. Empty for a statically linked plugin. Kept, and closed when
         * the plugin is unregistered, only if the registration succeeds.
         *
         * @return @c true if the plugin is registered.
         */
        bool Register(const PluginDescriptor* descriptor, LibraryInfo library = {});

        /**
         * @brief Checks whether a plugin with the given name is registered.
         */
        bool IsRegistered(const AmString& name);

        /**
         * @brief Checks whether a plugin with the given descriptor is registered.
         */
        bool IsRegistered(const PluginDescriptor* descriptor);

        /**
         * @brief Finds the native handle of a registered plugin's library.
         *
         * @return The handle, or @c nullptr if no registered plugin was loaded from that path.
         */
        AmVoidPtr FindLibrary(const AmOsString& path);

        /**
         * @brief Unregisters every plugin, in the reverse order of their registration, and closes their libraries.
         */
        void UnregisterAll();

        /**
         * @brief Calls the frame callbacks registered for the given stage, in the order they were registered.
         */
        void CallFrameCallbacks(ePluginFrameStage stage, AmTime delta);
    } // namespace PluginRegistry
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_CORE_PLUGIN_REGISTRY_H
