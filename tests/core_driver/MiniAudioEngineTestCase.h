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

#include "PlatformTestCase.h"
#include "TestCase.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    constexpr AmTime kMiniAudioFrameDelta = kAmSecond / 60.0;

    /**
     * @brief Test case base class for MiniAudio driver tests that require engine initialization.
     *
     * This test case initializes the engine with the MiniAudio driver instead of the null driver.
     * It sets up the full audio pipeline for testing real audio device operations.
     */
    class MiniAudioEngineTestCase : public TestCase
    {
    public:
        static void RunAudioThread(AmVoidPtr listener)
        {
            const auto* self = static_cast<MiniAudioEngineTestCase*>(listener);

            amLogDebug("MiniAudio Test Thread started");

            while (self->IsRunning())
            {
                if (amEngine->IsInitialized() && !amEngine->IsStopping())
                    amEngine->AdvanceFrame(kMiniAudioFrameDelta);

                Thread::Sleep(static_cast<AmInt32>(kMiniAudioFrameDelta));
            }

            amLogDebug("MiniAudio Test Thread ended");
        }

        void SetUp() override
        {
            MemoryManager::Initialize();

            amLogDebug("MiniAudio Test run started");

            _fileSystem = CreatePlatformFileSystem();
            _fileSystem->SetBasePath(GetPlatformAssetsBasePath());

            amEngine->SetFileSystem(_fileSystem);

            amEngine->StartOpenFileSystem();
            while (!amEngine->TryFinalizeOpenFileSystem())
                Thread::Sleep(1);

            amLogDebug("File system loaded");

            Engine::RegisterDefaultExtensions();

            amLogDebug("Extensions registered");

            Driver::SetDefault("miniaudio");

            _running = true;
            _threadHandle = Thread::CreateThread(RunAudioThread, this);

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
            Deinitialize();

            Engine::UnregisterDefaultExtensions();

            amEngine->DestroyInstance();

            amLogDebug("MiniAudio Test run ended");

            _fileSystem.reset();

            MemoryManager::Deinitialize();
        }

        [[nodiscard]] AM_INLINE bool IsRunning() const
        {
            return _running;
        }

    protected:
        bool Deinitialize()
        {
            _running = false;

            if (_threadHandle)
                Thread::Release(_threadHandle);

            bool success = true;
            if (amEngine->IsInitialized())
            {
                success = amEngine->Deinitialize();

                amEngine->StartCloseFileSystem();
                while (!amEngine->TryFinalizeCloseFileSystem())
                    Thread::Sleep(1);
            }

            return success;
        }

        std::shared_ptr<FileSystem> _fileSystem = nullptr;

    private:
        AmThreadHandle _threadHandle = nullptr;
        bool _running = false;
    };
} // namespace SparkyStudios::Audio::Amplitude::Tests
