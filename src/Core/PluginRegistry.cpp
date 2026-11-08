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

#include <algorithm>
#include <array>
#include <cstring>
#include <memory>
#include <mutex>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Core/Engine.h>
#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/IO/Log.h>

#include <Core/PluginRegistry.h>

namespace SparkyStudios::Audio::Amplitude
{
    namespace
    {
        // Constant-initialized, so plugins registering themselves during static initialization always find the list ready.
        const StaticPluginRegistration* gFirstStaticPlugin = nullptr;

        struct RegisteredPlugin
        {
            const PluginDescriptor* descriptor = nullptr;
            PluginHost host = {};
            PluginRegistry::LibraryInfo library;
        };

        struct FrameCallback
        {
            PluginFrameCallback callback = nullptr;
            AmVoidPtr cookie = nullptr;
            const RegisteredPlugin* owner = nullptr;
        };

        // The registry storage is never destroyed: an engine instance left alive until exit unregisters its plugins from its
        // static destructor, which may run after the destructors of function-local statics.

        // Recursive: a plugin registers its frame callbacks from its Register function, while the registry is locked.
        std::recursive_mutex& RegistryMutex()
        {
            static auto* mutex = new std::recursive_mutex();
            return *mutex;
        }

        // Heap-allocated entries, so each plugin's host keeps its address while the list grows.
        std::vector<std::unique_ptr<RegisteredPlugin>>& Plugins()
        {
            static auto* plugins = new std::vector<std::unique_ptr<RegisteredPlugin>>();
            return *plugins;
        }

        std::array<std::vector<FrameCallback>, ePluginFrameStage_Count>& FrameCallbacks()
        {
            static auto* callbacks = new std::array<std::vector<FrameCallback>, ePluginFrameStage_Count>();
            return *callbacks;
        }

        void Remove(const RegisteredPlugin* plugin)
        {
            std::erase_if(
                Plugins(),
                [plugin](const std::unique_ptr<RegisteredPlugin>& entry)
                {
                    return entry.get() == plugin;
                });
        }

        const char* NameOf(const PluginDescriptor* descriptor)
        {
            return descriptor->name != nullptr && descriptor->name[0] != '\0' ? descriptor->name : "<unnamed>";
        }

        bool HasText(const char* text)
        {
            return text != nullptr && text[0] != '\0';
        }

        bool ValidateDescriptor(const PluginDescriptor* descriptor)
        {
            if (descriptor == nullptr)
            {
                amLogError("Cannot register a plugin without a descriptor.");
                return false;
            }

            if (descriptor->abiVersion != AM_PLUGIN_ABI_VERSION)
            {
                amLogError(
                    "Cannot register the plugin '%s': it was built for the plugin ABI version %u, but the engine uses version %u. "
                    "Rebuild the plugin against this SDK.",
                    NameOf(descriptor), descriptor->abiVersion, AM_PLUGIN_ABI_VERSION);
                return false;
            }

            if (descriptor->structSize != sizeof(PluginDescriptor))
            {
                amLogError(
                    "Cannot register the plugin '%s': its descriptor has a size of %u bytes, but the engine expects %u bytes.",
                    NameOf(descriptor), descriptor->structSize, static_cast<AmUInt32>(sizeof(PluginDescriptor)));
                return false;
            }

            if (!HasText(descriptor->name) || !HasText(descriptor->version))
            {
                amLogError("Cannot register the plugin '%s': its descriptor must have a name and a version.", NameOf(descriptor));
                return false;
            }

            if (descriptor->Register == nullptr || descriptor->Unregister == nullptr)
            {
                amLogError(
                    "Cannot register the plugin '%s': its descriptor must have the Register and Unregister functions.", NameOf(descriptor));
                return false;
            }

            return true;
        }

        RegisteredPlugin* FindByName(const char* name)
        {
            for (const auto& plugin : Plugins())
                if (std::strcmp(plugin->descriptor->name, name) == 0)
                    return plugin.get();

            return nullptr;
        }

        const RegisteredPlugin* OwnerOf(const PluginHost* host)
        {
            if (host == nullptr)
                return nullptr;

            for (const auto& plugin : Plugins())
                if (&plugin->host == host && plugin->host.context == plugin.get())
                    return plugin.get();

            return nullptr;
        }

        bool IsValidStage(ePluginFrameStage stage)
        {
            return static_cast<AmUInt32>(stage) < ePluginFrameStage_Count;
        }

        auto FindCallback(std::vector<FrameCallback>& callbacks, PluginFrameCallback callback, AmVoidPtr cookie)
        {
            return std::find_if(
                callbacks.begin(), callbacks.end(),
                [&](const FrameCallback& entry)
                {
                    return entry.callback == callback && entry.cookie == cookie;
                });
        }

        bool HostRegisterFrameCallback(const PluginHost* host, ePluginFrameStage stage, PluginFrameCallback callback, AmVoidPtr cookie)
        {
            std::lock_guard lock(RegistryMutex());

            const RegisteredPlugin* owner = OwnerOf(host);
            if (owner == nullptr || callback == nullptr || !IsValidStage(stage))
            {
                amLogError("Cannot register a plugin frame callback: invalid host, callback or stage.");
                return false;
            }

            if (IsEngineRunning())
            {
                amLogError(
                    "The plugin '%s' cannot register a frame callback while the engine is initialized.", owner->descriptor->name);
                return false;
            }

            auto& callbacks = FrameCallbacks()[stage];
            if (FindCallback(callbacks, callback, cookie) != callbacks.end())
            {
                amLogWarning("The plugin '%s' registered the same frame callback twice.", owner->descriptor->name);
                return false;
            }

            callbacks.push_back({ .callback = callback, .cookie = cookie, .owner = owner });
            return true;
        }

        bool HostUnregisterFrameCallback(const PluginHost* host, ePluginFrameStage stage, PluginFrameCallback callback, AmVoidPtr cookie)
        {
            std::lock_guard lock(RegistryMutex());

            const RegisteredPlugin* owner = OwnerOf(host);
            if (owner == nullptr || !IsValidStage(stage) || IsEngineRunning())
                return false;

            auto& callbacks = FrameCallbacks()[stage];
            const auto it = FindCallback(callbacks, callback, cookie);
            if (it == callbacks.end() || it->owner != owner)
                return false;

            callbacks.erase(it);
            return true;
        }

        void RemoveFrameCallbacksOf(const RegisteredPlugin* owner)
        {
            for (auto& callbacks : FrameCallbacks())
                std::erase_if(
                    callbacks,
                    [owner](const FrameCallback& entry)
                    {
                        return entry.owner == owner;
                    });
        }

        // Plugins must not throw into the engine; a throwing Register or Unregister counts as a failure.
        bool CallPlugin(bool (*function)(const PluginHost*), const PluginHost* host, const char* name, const char* what)
        {
            try
            {
                return function(host);
            } catch (const std::exception& e)
            {
                amLogError("The plugin '%s' threw an exception from %s: %s", name, what, e.what());
            } catch (...)
            {
                amLogError("The plugin '%s' threw an exception from %s.", name, what);
            }

            return false;
        }
    } // namespace

    StaticPluginRegistration::StaticPluginRegistration(const PluginDescriptor* descriptor)
        : _descriptor(descriptor)
        , _next(gFirstStaticPlugin)
    {
        gFirstStaticPlugin = this;
    }

    const StaticPluginRegistration* StaticPluginRegistration::GetFirst()
    {
        return gFirstStaticPlugin;
    }

    const PluginDescriptor* StaticPluginRegistration::GetDescriptor() const
    {
        return _descriptor;
    }

    const StaticPluginRegistration* StaticPluginRegistration::GetNext() const
    {
        return _next;
    }

    namespace PluginRegistry
    {
        bool Register(const PluginDescriptor* descriptor, LibraryInfo library)
        {
            if (!ValidateDescriptor(descriptor))
                return false;

            // Taken before the registry lock: Engine::DestroyInstance() takes the instance lock, then the registry lock.
            Engine* engine = Engine::GetInstance();
            if (engine == nullptr)
            {
                amLogError("Cannot register the plugin '%s': the memory manager is not initialized.", descriptor->name);
                return false;
            }

            std::lock_guard lock(RegistryMutex());

            if (IsEngineRunning())
            {
                amLogError(
                    "The plugin '%s' cannot be registered while the engine is initialized. Register plugins before "
                    "Engine::Initialize().",
                    descriptor->name);
                return false;
            }

            if (FindByName(descriptor->name) != nullptr)
            {
                amLogError("Cannot register the plugin '%s': a plugin with the same name is already registered.", descriptor->name);
                return false;
            }

            auto plugin = std::make_unique<RegisteredPlugin>();
            plugin->descriptor = descriptor;
            plugin->host = PluginHost{
                .abiVersion = AM_PLUGIN_ABI_VERSION,
                .structSize = sizeof(PluginHost),
                .engine = engine,
                .memory = MemoryManager::GetInstance(),
                .RegisterFrameCallback = HostRegisterFrameCallback,
                .UnregisterFrameCallback = HostUnregisterFrameCallback,
                .context = plugin.get(),
            };

            // Listed first, so the plugin can register its frame callbacks from Register.
            RegisteredPlugin* registered = plugin.get();
            Plugins().push_back(std::move(plugin));

            if (!CallPlugin(descriptor->Register, &registered->host, descriptor->name, "Register"))
            {
                amLogError("Failed to register the plugin '%s'.", descriptor->name);
                RemoveFrameCallbacksOf(registered);
                Remove(registered);
                return false;
            }

            registered->library = std::move(library);
            amLogInfo("Registered plugin '%s' version %s.", descriptor->name, descriptor->version);
            return true;
        }

        bool IsRegistered(const AmString& name)
        {
            std::lock_guard lock(RegistryMutex());
            return FindByName(name.c_str()) != nullptr;
        }

        bool IsRegistered(const PluginDescriptor* descriptor)
        {
            std::lock_guard lock(RegistryMutex());

            for (const auto& plugin : Plugins())
                if (plugin->descriptor == descriptor)
                    return true;

            return false;
        }

        AmVoidPtr FindLibrary(const AmOsString& path)
        {
            std::lock_guard lock(RegistryMutex());

            for (const auto& plugin : Plugins())
                if (plugin->library.nativeHandle != nullptr && plugin->library.path == path)
                    return plugin->library.nativeHandle;

            return nullptr;
        }

        void UnregisterAll()
        {
            std::lock_guard lock(RegistryMutex());

            auto& plugins = Plugins();
            while (!plugins.empty())
            {
                // Still listed while it unregisters, so its host keeps working until Unregister returns.
                RegisteredPlugin* registered = plugins.back().get();

                const PluginDescriptor* descriptor = registered->descriptor;
                if (!CallPlugin(descriptor->Unregister, &registered->host, descriptor->name, "Unregister"))
                    amLogError("An error occurred while unregistering the plugin '%s'.", descriptor->name);

                RemoveFrameCallbacksOf(registered);

                const std::unique_ptr<RegisteredPlugin> plugin = std::move(plugins.back());
                plugins.pop_back();

                // The library goes last: the descriptor and the callbacks live in it.
                if (plugin->library.close)
                    plugin->library.close();
            }
        }

        void CallFrameCallbacks(ePluginFrameStage stage, AmTime delta)
        {
            for (const auto& entry : FrameCallbacks()[stage])
                entry.callback(delta, entry.cookie);
        }
    } // namespace PluginRegistry
} // namespace SparkyStudios::Audio::Amplitude
