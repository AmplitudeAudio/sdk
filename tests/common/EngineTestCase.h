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
    constexpr AmTime kFrameDelta = kAmSecond / 60.0;

    class InvalidConsumerNodeInstance final
        : public NodeInstance
        , public ProviderNodeInstance
    {
    public:
        const AudioBuffer* Provide() override
        {
            return nullptr;
        }

        void Reset() override
        {}
    };

    class InvalidConsumerNode final : public Node
    {
    public:
        InvalidConsumerNode()
            : Node("InvalidConsumerNode")
        {}

        [[nodiscard]] AM_INLINE std::shared_ptr<NodeInstance> CreateInstance() const override
        {
            return ampoolshared(eMemoryPoolKind_Amplimix, InvalidConsumerNodeInstance);
        }

        [[nodiscard]] AM_INLINE bool CanConsume() const override
        {
            return true;
        }

        [[nodiscard]] AM_INLINE bool CanProduce() const override
        {
            return false;
        }

        [[nodiscard]] AM_INLINE AmSize GetMaxInputCount() const override
        {
            return 1;
        }

        [[nodiscard]] AM_INLINE AmSize GetMinInputCount() const override
        {
            return 1;
        }
    };

    class EngineTestCase final : public TestCase
    {
    public:
        static void run(AmVoidPtr listener)
        {
            const auto* self = static_cast<EngineTestCase*>(listener);

            amLogDebug("Amplitude Thread started");

            while (self->IsRunning())
            {
                if (amEngine->IsInitialized() && !amEngine->IsStopping())
                    amEngine->AdvanceFrame(kFrameDelta);

                Thread::Sleep(static_cast<AmInt32>(kFrameDelta));
            }

            amLogDebug("Amplitude Thread ended");
        }

        void SetUp() override
        {
            MemoryManager::Initialize();

            amLogDebug("Test run started");

            _fileSystem = ampoolshared(eMemoryPoolKind_IO, DiskFileSystem);

            _invalidConsumerNodePlugin = Engine::RegisterExtension<InvalidConsumerNode>();

            _fileSystem->SetBasePath(AM_OS_STRING("./samples/assets"));

            Engine::AddPluginSearchPath(_fileSystem->ResolvePath(AM_OS_STRING("../")));

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
            // Deinitialize engine
            Deinitialize();

            // Unregister all default plugins
            Engine::UnregisterDefaultExtensions();

            Engine::UnregisterExtension(_invalidConsumerNodePlugin);

            amEngine->DestroyInstance();

            amLogDebug("Test run ended");

            _fileSystem.reset();

            MemoryManager::Deinitialize();
        }

        void Run() override;

        [[nodiscard]] AM_INLINE bool IsRunning() const
        {
            return _running;
        }

    protected:
        bool Deinitialize()
        {
            _running = false;

            if (_threadHandle)
            {
                Thread::Wait(_threadHandle);
                Thread::Release(_threadHandle);
            }

            bool success = true;
            if (amEngine->IsInitialized())
            {
                amEngine->SetDefaultListener(nullptr);
                amEngine->RemoveListener(1);

                success = amEngine->Deinitialize();

                // Wait for the file system to complete loading.
                amEngine->StartCloseFileSystem();
                while (!amEngine->TryFinalizeCloseFileSystem())
                    Thread::Sleep(1);
            }

            return success;
        }

        std::shared_ptr<DiskFileSystem> _fileSystem = nullptr;

    private:
        AmThreadHandle _threadHandle = nullptr;
        bool _running = false;
        std::shared_ptr<InvalidConsumerNode> _invalidConsumerNodePlugin = nullptr;
    };

    std::shared_ptr<TestCase> MakeTestCase()
    {
        return std::make_shared<EngineTestCase>();
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
