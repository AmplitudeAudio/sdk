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

#include <catch2/catch_session.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>
#include <iostream>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

using namespace SparkyStudios::Audio::Amplitude;

struct MyListener : Catch::EventListenerBase
{
    using EventListenerBase::EventListenerBase; // inherit constructor

    // Get rid of Wweak-tables
    ~MyListener() override = default;

    static void run(AmVoidPtr listener)
    {
        const auto* self = static_cast<MyListener*>(listener);

        while (self->running)
        {
            amEngine->AdvanceFrame(1.0 / 60);
            Thread::Sleep(static_cast<AmInt32>(kAmSecond / 60.0));
        }
    }

    // The whole test run starting
    void testRunStarting(Catch::TestRunInfo const& testRunInfo) override
    {
        MemoryManager::Initialize({});

        fileSystem.SetBasePath(AM_OS_STRING("./samples/assets"));

        amEngine->SetFileSystem(&fileSystem);

        // Wait for the file system to complete loading.
        amEngine->StartOpenFileSystem();
        while (!amEngine->TryFinalizeOpenFileSystem())
            Thread::Sleep(1);

        // Register all the default plugins shipped with the engine
        Engine::RegisterDefaultPlugins();

        const auto sdkPath = std::filesystem::path(std::getenv("AM_SDK_PATH"));

        Engine::AddPluginSearchPath(AM_OS_STRING("./assets/plugins"));
#if defined(AM_WINDOWS_VERSION)
        Engine::AddPluginSearchPath(sdkPath / AM_OS_STRING("lib/win/plugins"));
#elif defined(AM_LINUX_VERSION)
        Engine::AddPluginSearchPath(sdkPath / AM_OS_STRING("lib/linux/plugins"));
#elif defined(AM_OSX_VERSION)
        Engine::AddPluginSearchPath(sdkPath / AM_OS_STRING("lib/osx/plugins"));
#endif

        Engine::LoadPlugin(AM_OS_STRING("AmplitudeVorbisCodecPlugin_d"));
        Engine::LoadPlugin(AM_OS_STRING("AmplitudeFlacCodecPlugin_d"));

        running = true;

        threadHandle = Thread::CreateThread(run, this);
    }

    // The whole test run ending
    void testRunEnded(Catch::TestRunStats const& testRunStats) override
    {
        running = false;

        Thread::Wait(threadHandle);
        Thread::Release(threadHandle);

        if (amEngine->IsInitialized())
        {
            amEngine->UnloadSoundBanks();

            amEngine->Deinitialize();

            // Wait for the file system to complete loading.
            amEngine->StartCloseFileSystem();
            while (!amEngine->TryFinalizeCloseFileSystem())
                Thread::Sleep(1);

            amEngine->DestroyInstance();
        }

        // Unregister all default plugins
        Engine::UnregisterDefaultPlugins();

        MemoryManager::Deinitialize();
    }

    AmThreadHandle threadHandle;
    DiskFileSystem fileSystem;
    bool running = false;
};

CATCH_REGISTER_LISTENER(MyListener)

int main(int argc, char* argv[])
{
    const auto res = Catch::Session().run(argc, argv);

    return res;
}