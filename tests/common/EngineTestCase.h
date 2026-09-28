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

#include <atomic>

#include "PlatformTestCase.h"
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

    class EngineTestCase : public TestCase
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

            // Use platform abstraction for file system creation
            _fileSystem = CreatePlatformFileSystem();

            _invalidConsumerNodePlugin = Engine::RegisterExtension<InvalidConsumerNode>();

            // Use platform-appropriate assets path
            _fileSystem->SetBasePath(GetPlatformAssetsBasePath());

            // Plugin search paths only work on desktop platforms
            if (SupportsPluginLoading())
            {
                Engine::AddPluginSearchPath(_fileSystem->ResolvePath(AM_OS_STRING("../")));
            }

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

            _running.store(true, std::memory_order_release);

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

        [[nodiscard]] AM_INLINE bool IsRunning() const
        {
            return _running.load(std::memory_order_acquire);
        }

    protected:
        /**
         * @brief The frame cap used by @c WaitUntil() (about 5 seconds at 60 frames per second).
         */
        static constexpr AmUInt64 kMaxWaitFrames = 300;

        /**
         * @brief Advances engine frames until @p condition holds, or until @p maxFrames frames elapsed.
         *
         * Audio-thread effects (mixer commands such as seeks, cursor write-backs, mixer-side state, sound-end
         * callbacks) land on the audio callback period, which is unrelated to game frames: wait for the effect
         * itself instead of a fixed number of frames.
         *
         * @param[in] condition Returns @c true once the awaited effect is visible.
         * @param[in] maxFrames The maximum number of frames to wait.
         *
         * @return @c true if @p condition held before the cap was reached.
         */
        template<typename Predicate>
        [[nodiscard]] bool WaitUntil(Predicate&& condition, AmUInt64 maxFrames = kMaxWaitFrames)
        {
            for (AmUInt64 frame = 0; frame < maxFrames; ++frame)
            {
                if (condition())
                    return true;

                amEngine->WaitUntilFrames(1);
            }

            return condition();
        }

        bool Deinitialize()
        {
            _running.store(false, std::memory_order_release);

            // Join the engine thread before the engine goes away: it calls into amEngine every iteration.
            if (_threadHandle)
                Thread::Wait(_threadHandle);

            bool success = true;
            if (amEngine->IsInitialized())
            {
                success = amEngine->Deinitialize();

                // Wait for the file system to complete loading.
                amEngine->StartCloseFileSystem();
                while (!amEngine->TryFinalizeCloseFileSystem())
                    Thread::Sleep(1);
            }

            return success;
        }

        std::shared_ptr<FileSystem> _fileSystem = nullptr;

    private:
        AmThreadHandle _threadHandle = nullptr;
        std::atomic<bool> _running{ false };
        std::shared_ptr<InvalidConsumerNode> _invalidConsumerNodePlugin = nullptr;
    };
} // namespace SparkyStudios::Audio::Amplitude::Tests
