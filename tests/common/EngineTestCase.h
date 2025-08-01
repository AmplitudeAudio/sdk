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

#include "TestCase.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    class EngineTestCase : public TestCase
    {
    public:
        static void run(AmVoidPtr listener)
        {
            const auto* self = static_cast<EngineTestCase*>(listener);

            amLogDebug("Amplitude Thread started");

            while (self->IsRunning())
            {
                constexpr AmTime delta = kAmSecond / 60.0;

                amEngine->AdvanceFrame(delta);
                Thread::Sleep(static_cast<AmInt32>(delta));
            }

            amLogDebug("Amplitude Thread ended");
        }

        void SetUp() override
        {
            amLogDebug("Test run started");

            _fileSystem->SetBasePath(AM_OS_STRING("./samples/assets"));

            amEngine->SetFileSystem(_fileSystem);

            // Wait for the file system to complete loading.
            amEngine->StartOpenFileSystem();
            while (!amEngine->TryFinalizeOpenFileSystem())
                Thread::Sleep(1);

            amLogDebug("File system loaded");

            // Register all the default plugins shipped with the engine
            Engine::RegisterDefaultExtensions();
            Driver::Unregister(Driver::Find("miniaudio"));

            amLogDebug("Extensions registered");

            // const auto sdkPath = std::filesystem::path(std::getenv("AM_SDK_PATH"));

            // Engine::AddPluginSearchPath(AM_OS_STRING("./assets/plugins"));
            // Engine::AddPluginSearchPath(sdkPath / AM_OS_STRING("lib/" AM_SDK_PLATFORM "/plugins"));

            // Engine::LoadPlugin(AM_OS_STRING("AmplitudeVorbisCodecPlugin_d"));
            // Engine::LoadPlugin(AM_OS_STRING("AmplitudeFlacCodecPlugin_d"));

            _running = true;

            _threadHandle = Thread::CreateThread(run, this);

            amEngine->Initialize(AM_OS_STRING("tests.config.amconfig"));

            amEngine->EnsureSoundBankLoaded(AM_OS_STRING("tests.init.ambank"));

            amEngine->StartLoadSoundFiles();
            while (!amEngine->TryFinalizeLoadSoundFiles())
                Thread::Sleep(1);

            AM_EXPECT(amEngine->TryFinalizeLoadSoundFiles());

            AM_UNUSED(amEngine->AddListener(1));
            amEngine->SetDefaultListener(1);
        }

        void TearDown() override
        {
            _running = false;

            Thread::Wait(_threadHandle);
            Thread::Release(_threadHandle);

            if (amEngine->IsInitialized())
            {
                amEngine->SetDefaultListener(nullptr);
                amEngine->RemoveListener(1);

                amEngine->UnloadSoundBanks();

                amEngine->Deinitialize();

                // Wait for the file system to complete loading.
                amEngine->StartCloseFileSystem();
                while (!amEngine->TryFinalizeCloseFileSystem())
                    Thread::Sleep(1);
            }

            // Unregister all default plugins
            Engine::UnregisterDefaultExtensions();

            amEngine->DestroyInstance();

            amLogDebug("Test run ended");
        }

        void Run() override;

        AM_INLINE bool IsRunning() const
        {
            return _running;
        }

    protected:
        std::shared_ptr<DiskFileSystem> _fileSystem = AmSharedPtr<DiskFileSystem, eMemoryPoolKind_IO>::Make();

    private:
        AmThreadHandle _threadHandle = nullptr;
        bool _running = false;
    };

    std::shared_ptr<TestCase> MakeTestCase()
    {
        return AmSharedPtr<EngineTestCase>::Make();
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
