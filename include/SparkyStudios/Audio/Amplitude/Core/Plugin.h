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

#ifndef _AM_CORE_PLUGIN_H
#define _AM_CORE_PLUGIN_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

/**
 * @brief The version of the plugin ABI.
 *
 * A plugin records the version it was built with in its @ref SparkyStudios::Audio::Amplitude::PluginDescriptor "descriptor",
 * and the engine only registers plugins built with the exact same version. Plugins share C++ types with the engine, so this
 * version is bumped whenever the SDK changes anything a plugin depends on.
 *
 * @ingroup engine
 */
#define AM_PLUGIN_ABI_VERSION 1

/**
 * @brief The name of the symbol a dynamic plugin library exports to give its descriptor to the engine.
 *
 * @ingroup engine
 */
#define AM_PLUGIN_ENTRY_POINT_NAME "AmGetPluginDescriptor"

namespace SparkyStudios::Audio::Amplitude
{
    class Engine;
    class MemoryManager;
    struct PluginHost;

    /**
     * @brief When a plugin frame callback runs within a frame.
     *
     * @ingroup engine
     */
    enum ePluginFrameStage : AmUInt32
    {
        /**
         * @brief Before the engine updates the listeners, rooms and entities. The callback sees the state the host left.
         */
        ePluginFrameStage_BeforeUpdate = 0,

        /**
         * @brief After the engine updated the listeners, rooms and entities, before the channels update. What the callback
         * sets applies to this frame's channel updates.
         */
        ePluginFrameStage_AfterUpdate = 1,

        /**
         * @brief The number of frame stages.
         */
        ePluginFrameStage_Count
    };

    /**
     * @brief A function the engine calls once per frame, at the stage it was registered for.
     *
     * @param[in] delta The time elapsed since the last frame, in seconds.
     * @param[in] cookie The value given when the callback was registered.
     *
     * @ingroup engine
     */
    typedef void (*PluginFrameCallback)(AmTime delta, AmVoidPtr cookie);

    /**
     * @brief The services the engine gives to a plugin.
     *
     * The engine creates one host per registered plugin and passes it to the plugin's @c Register and @c Unregister
     * functions. The host stays valid until @c Unregister returns.
     *
     * @ingroup engine
     */
    struct PluginHost
    {
        /**
         * @brief The plugin ABI version of the engine. Equal to @ref AM_PLUGIN_ABI_VERSION.
         */
        AmUInt32 abiVersion;

        /**
         * @brief The size of this structure, as the engine was built.
         */
        AmUInt32 structSize;

        /**
         * @brief The engine instance.
         */
        Engine* engine;

        /**
         * @brief The engine memory manager.
         */
        MemoryManager* memory;

        /**
         * @brief Registers a function the engine calls once per frame.
         *
         * Callbacks can only be registered while the engine is not initialized, usually from the plugin's @c Register
         * function. They are removed when the plugin is unregistered.
         *
         * @param[in] host The host given to the plugin.
         * @param[in] stage When the callback runs within a frame.
         * @param[in] callback The function to call.
         * @param[in] cookie A value passed back to the callback.
         *
         * @return @c true if the callback was registered, @c false if the engine is initialized, an argument is invalid, or the
         * same callback and cookie are already registered for that stage.
         */
        bool (*RegisterFrameCallback)(const PluginHost* host, ePluginFrameStage stage, PluginFrameCallback callback, AmVoidPtr cookie);

        /**
         * @brief Removes a frame callback registered with @c RegisterFrameCallback.
         *
         * @param[in] host The host given to the plugin.
         * @param[in] stage The stage the callback was registered for.
         * @param[in] callback The registered function.
         * @param[in] cookie The registered cookie.
         *
         * @return @c true if the callback was removed, @c false if the engine is initialized or no such callback is registered.
         */
        bool (*UnregisterFrameCallback)(const PluginHost* host, ePluginFrameStage stage, PluginFrameCallback callback, AmVoidPtr cookie);

        /**
         * @brief Reserved for the engine.
         */
        AmVoidPtr context;
    };

    /**
     * @brief Describes a plugin to the engine.
     *
     * A plugin defines a single descriptor and exposes it with @ref AM_DEFINE_PLUGIN. The same descriptor is used whether
     * the plugin is linked statically into the game or loaded from a shared library.
     *
     * @code{cpp}
     * static bool Register(const PluginHost* host) { ... }
     * static bool Unregister(const PluginHost* host) { ... }
     *
     * static const PluginDescriptor kDescriptor = {
     *     .abiVersion = AM_PLUGIN_ABI_VERSION,
     *     .structSize = sizeof(PluginDescriptor),
     *     .name = "My Plugin",
     *     .version = "1.0.0",
     *     .Register = Register,
     *     .Unregister = Unregister,
     * };
     *
     * AM_DEFINE_PLUGIN(MyPlugin, kDescriptor)
     * @endcode
     *
     * @ingroup engine
     */
    struct PluginDescriptor
    {
        /**
         * @brief The plugin ABI version the plugin was built with. Must be @ref AM_PLUGIN_ABI_VERSION.
         */
        AmUInt32 abiVersion;

        /**
         * @brief The size of this structure, as the plugin was built. Must be @c sizeof(PluginDescriptor).
         */
        AmUInt32 structSize;

        /**
         * @brief The plugin name. Required, and unique among the registered plugins.
         */
        const char* name;

        /**
         * @brief The plugin version. Required.
         */
        const char* version;

        /**
         * @brief A short description of the plugin. Optional.
         */
        const char* description;

        /**
         * @brief The plugin author. Optional.
         */
        const char* author;

        /**
         * @brief The plugin copyright notice. Optional.
         */
        const char* copyright;

        /**
         * @brief The plugin license. Optional.
         */
        const char* license;

        /**
         * @brief Registers the plugin's extensions and frame callbacks. Required.
         *
         * @param[in] host The services the engine gives to the plugin.
         *
         * @return @c true on success. On failure the plugin must leave nothing registered: @c Unregister is not called.
         */
        bool (*Register)(const PluginHost* host);

        /**
         * @brief Unregisters everything @c Register registered. Required.
         *
         * @param[in] host The host given to @c Register.
         *
         * @return @c true on success.
         */
        bool (*Unregister)(const PluginHost* host);
    };

    /**
     * @brief Adds a statically linked plugin to the list the engine registers with @ref Engine::RegisterStaticPlugins().
     *
     * Do not use this class directly. Use @ref AM_DEFINE_PLUGIN in a plugin built with @c AM_BUILDSYSTEM_STATIC_PLUGIN,
     * and @ref AM_STATIC_LINK_PLUGIN in the game.
     *
     * @ingroup engine
     */
    class AM_API_PUBLIC StaticPluginRegistration
    {
    public:
        /**
         * @brief Adds the descriptor to the list of statically linked plugins.
         *
         * @param[in] descriptor The plugin descriptor. Must outlive the program.
         */
        explicit StaticPluginRegistration(const PluginDescriptor* descriptor);

        StaticPluginRegistration(const StaticPluginRegistration&) = delete;
        StaticPluginRegistration& operator=(const StaticPluginRegistration&) = delete;

        /**
         * @brief Gets the first statically linked plugin.
         *
         * @return The first registration, or @c nullptr if no plugin is statically linked.
         */
        [[nodiscard]] static const StaticPluginRegistration* GetFirst();

        /**
         * @brief Gets the plugin descriptor.
         */
        [[nodiscard]] const PluginDescriptor* GetDescriptor() const;

        /**
         * @brief Gets the next statically linked plugin.
         *
         * @return The next registration, or @c nullptr if this one is the last.
         */
        [[nodiscard]] const StaticPluginRegistration* GetNext() const;

    private:
        const PluginDescriptor* _descriptor;
        const StaticPluginRegistration* _next;
    };
} // namespace SparkyStudios::Audio::Amplitude

/**
 * @brief Defines a plugin linked statically into the game.
 *
 * Write it once, at global scope, in one source file of the plugin. The game must reference the plugin with
 * @ref AM_STATIC_LINK_PLUGIN so the linker keeps it.
 *
 * @param _name_ An identifier naming the plugin, the same given to @ref AM_STATIC_LINK_PLUGIN.
 * @param _descriptor_ The plugin @ref SparkyStudios::Audio::Amplitude::PluginDescriptor "descriptor".
 *
 * @ingroup engine
 */
#define AM_DEFINE_STATIC_PLUGIN(_name_, _descriptor_)                                                                                      \
    ::SparkyStudios::Audio::Amplitude::StaticPluginRegistration _name_##_AmPluginRegistration(&(_descriptor_));

/**
 * @brief Defines a plugin loaded from a shared library with @ref Engine::LoadPlugin().
 *
 * Write it once, at global scope, in one source file of the plugin.
 *
 * @param _name_ An identifier naming the plugin. Unused, kept for symmetry with @ref AM_DEFINE_STATIC_PLUGIN.
 * @param _descriptor_ The plugin @ref SparkyStudios::Audio::Amplitude::PluginDescriptor "descriptor".
 *
 * @ingroup engine
 */
#define AM_DEFINE_DYNAMIC_PLUGIN(_name_, _descriptor_)                                                                                     \
    extern "C" AM_LIB_EXPORT const ::SparkyStudios::Audio::Amplitude::PluginDescriptor* AmGetPluginDescriptor()                            \
    {                                                                                                                                      \
        return &(_descriptor_);                                                                                                            \
    }

/**
 * @brief Defines a plugin, statically linked when @c AM_BUILDSYSTEM_STATIC_PLUGIN is defined, dynamically loaded otherwise.
 *
 * The plugin build system defines @c AM_BUILDSYSTEM_STATIC_PLUGIN when building the plugin as a static library, so the same
 * source builds both ways.
 *
 * @ingroup engine
 */
#if defined(AM_BUILDSYSTEM_STATIC_PLUGIN)
#define AM_DEFINE_PLUGIN(_name_, _descriptor_) AM_DEFINE_STATIC_PLUGIN(_name_, _descriptor_)
#else
#define AM_DEFINE_PLUGIN(_name_, _descriptor_) AM_DEFINE_DYNAMIC_PLUGIN(_name_, _descriptor_)
#endif

/**
 * @brief Makes the game keep a statically linked plugin.
 *
 * Write it once, at global scope, in one source file of the game. Without it the linker may drop the plugin, since nothing
 * else references it. Then call @ref Engine::RegisterStaticPlugins() before initializing the engine.
 *
 * @param _name_ The identifier given to @ref AM_DEFINE_STATIC_PLUGIN.
 *
 * @ingroup engine
 */
#define AM_STATIC_LINK_PLUGIN(_name_)                                                                                                      \
    extern ::SparkyStudios::Audio::Amplitude::StaticPluginRegistration _name_##_AmPluginRegistration;                                      \
    void* _name_##_AmPluginLinkOnce = &_name_##_AmPluginRegistration;

#endif // _AM_CORE_PLUGIN_H
