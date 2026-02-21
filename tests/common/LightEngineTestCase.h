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
    class LightEngineTestCase : public TestCase
    {
    public:
        void SetUp() override
        {
            MemoryManager::Initialize();

            _fileSystem = CreatePlatformFileSystem();
            _fileSystem->SetBasePath(GetPlatformAssetsBasePath());
            amEngine->SetFileSystem(_fileSystem);

            amEngine->StartOpenFileSystem();
            while (!amEngine->TryFinalizeOpenFileSystem())
                Thread::Sleep(1);

            Engine::RegisterDefaultExtensions();
        }

        void TearDown() override
        {
            if (amEngine->IsInitialized())
            {
                amEngine->Deinitialize();

                amEngine->StartCloseFileSystem();
                while (!amEngine->TryFinalizeCloseFileSystem())
                    Thread::Sleep(1);
            }

            Engine::UnregisterDefaultExtensions();
            amEngine->DestroyInstance();
            _fileSystem.reset();
            MemoryManager::Deinitialize();
        }

    protected:
        void InitializeConfig(const AmOsString& configPath)
        {
            amEngine->Initialize(configPath);
        }

        void LoadSoundBank(const AmOsString& bankName)
        {
            amEngine->EnsureSoundBankLoaded(bankName);
        }

        std::shared_ptr<FileSystem> _fileSystem;
    };
} // namespace SparkyStudios::Audio::Amplitude::Tests
