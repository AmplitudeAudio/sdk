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

#include <catch2/catch_test_macros.hpp>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/EntityInternalState.h>

using namespace SparkyStudios::Audio::Amplitude;

class FailingDriver final : public Driver
{
public:
    FailingDriver()
        : Driver("failing")
    {}

    bool Open(const DeviceDescription& device) override
    {
        return false; // Always fail
    }

    bool Close() override
    {
        return false; // Always fail
    }

    bool EnumerateDevices(std::vector<DeviceDescription>& devices) override
    {
        return false; // Always fail
    }
};

TEST_CASE("Engine Tests", "[engine][core][amplitude]")
{
    DiskFileSystem fileSystem;
    fileSystem.SetBasePath(AM_OS_STRING("./samples/assets"));

    SECTION("engine instance is never nullptr")
    {
        REQUIRE_FALSE(amEngine == nullptr);
    }

    SECTION("cannot initialize with an unknown config")
    {
        REQUIRE(amEngine->Deinitialize());
        REQUIRE_FALSE(amEngine->Initialize(AM_OS_STRING("unexistant.config.amconfig")));
        REQUIRE(amEngine->Deinitialize());
    }

    GIVEN("an unknow driver")
    {
        THEN("it can be initialized with a default driver")
        {
            REQUIRE(amEngine->Deinitialize());
            REQUIRE(amEngine->Initialize(AM_OS_STRING("tests.invalid.unknown_driver.config.amconfig")));
            REQUIRE(amEngine->GetDriver()->GetName() != "unknown");
            REQUIRE(amEngine->Deinitialize());
        }

        THEN("it cannot be initialized without a default driver")
        {
            REQUIRE(amEngine->Deinitialize());
            Engine::UnregisterDefaultPlugins();
            REQUIRE_FALSE(amEngine->Initialize(AM_OS_STRING("tests.invalid.unknown_driver.config.amconfig")));
            REQUIRE(amEngine->Deinitialize());
            Engine::RegisterDefaultPlugins();
        }
    }

    GIVEN("an empty driver config")
    {
        THEN("it can be initialized with a default driver")
        {
            REQUIRE(amEngine->Deinitialize());
            REQUIRE(amEngine->Initialize(AM_OS_STRING("tests.invalid.unset_driver.config.amconfig")));
            REQUIRE(amEngine->GetDriver()->GetName() != "unknown");
            REQUIRE(amEngine->Deinitialize());
        }

        THEN("it cannot be initialized without a default driver")
        {
            REQUIRE(amEngine->Deinitialize());
            Engine::UnregisterDefaultPlugins();
            REQUIRE_FALSE(amEngine->Initialize(AM_OS_STRING("tests.invalid.unset_driver.config.amconfig")));
            REQUIRE(amEngine->Deinitialize());
            Engine::RegisterDefaultPlugins();
        }
    }

    GIVEN("a failing driver")
    {
        FailingDriver driver;

        THEN("it cannot be initialized with a failing driver")
        {
            REQUIRE(amEngine->Deinitialize());
            REQUIRE_FALSE(amEngine->Initialize(AM_OS_STRING("tests.invalid.failing_driver.config.amconfig")));
            REQUIRE(amEngine->Deinitialize());
        }
    }

    SECTION("cannot initialize with an empty pipeline")
    {
        REQUIRE(amEngine->Deinitialize());
        REQUIRE_FALSE(amEngine->Initialize(AM_OS_STRING("tests.invalid.empty_pipeline.config.amconfig")));
        REQUIRE(amEngine->Deinitialize());
    }

    SECTION("cannot initialize with an unknown buses file")
    {
        REQUIRE(amEngine->Deinitialize());
        REQUIRE_FALSE(amEngine->Initialize(AM_OS_STRING("tests.invalid.unknown_buses.config.amconfig")));
        REQUIRE(amEngine->Deinitialize());
    }

    SECTION("cannot initialize with an invalid buses file (missing child buses)")
    {
        REQUIRE(amEngine->Deinitialize());
        REQUIRE_FALSE(amEngine->Initialize(AM_OS_STRING("tests.invalid.missing_child_bus.config.amconfig")));
        REQUIRE(amEngine->Deinitialize());
    }

    SECTION("cannot initialize with an invalid buses file (missing duck buses)")
    {
        REQUIRE(amEngine->Deinitialize());
        REQUIRE_FALSE(amEngine->Initialize(AM_OS_STRING("tests.invalid.missing_duck_bus.config.amconfig")));
        REQUIRE(amEngine->Deinitialize());
    }

    GIVEN("a wrong master bus config")
    {
        THEN("it can load if only the ID is invalid")
        {
            REQUIRE(amEngine->Deinitialize());
            REQUIRE(amEngine->Initialize(AM_OS_STRING("tests.invalid.wrong_master_bus_id.config.amconfig")));
            REQUIRE(amEngine->Deinitialize());
        }

        THEN("it can load if only the name is invalid")
        {
            REQUIRE(amEngine->Deinitialize());
            REQUIRE(amEngine->Initialize(AM_OS_STRING("tests.invalid.wrong_master_bus_name.config.amconfig")));
            REQUIRE(amEngine->Deinitialize());
        }

        THEN("it cannot load if both the ID and the name are invalid")
        {
            REQUIRE(amEngine->Deinitialize());
            REQUIRE_FALSE(amEngine->Initialize(AM_OS_STRING("tests.invalid.wrong_master_bus.config.amconfig")));
            REQUIRE(amEngine->Deinitialize());
        }
    }

    GIVEN("an initialized engine")
    {
        REQUIRE(amEngine->Initialize(AM_OS_STRING("tests.config.amconfig")));
        REQUIRE(amEngine->IsInitialized());

        THEN("it can be paused and resumed")
        {
            amEngine->Pause(true);
            REQUIRE(amEngine->IsPaused());

            amEngine->Pause(false);
            REQUIRE_FALSE(amEngine->IsPaused());
        }

        THEN("it can access buses")
        {
            REQUIRE(amEngine->FindBus("master").Valid());
            REQUIRE(amEngine->FindBus(1).Valid());

            REQUIRE_FALSE(amEngine->FindBus("invalid").Valid());
            REQUIRE_FALSE(amEngine->FindBus(120198434).Valid());
        }

        THEN("it cannot unload an unloaded soundbank")
        {
            // REQUIRE_THROWS(amEngine->UnloadSoundBank("init.ambank"));
        }

        WHEN("engine has loaded a soundbank")
        {
            REQUIRE(amEngine->LoadSoundBank(AM_OS_STRING("init.ambank")));

            THEN("it can load sound files")
            {
                REQUIRE(amEngine->TryFinalizeLoadSoundFiles());
                amEngine->StartLoadSoundFiles();

                // Wait for the sound files to complete loading
                while (!amEngine->TryFinalizeLoadSoundFiles())
                    Thread::Sleep(1);

                REQUIRE(amEngine->TryFinalizeLoadSoundFiles());
            }

            THEN("it can register entities")
            {
                Entity e1 = amEngine->AddEntity(1);
                Entity e2 = amEngine->AddEntity(2);

                REQUIRE(e1.Valid());
                REQUIRE(e2.Valid());

                Entity e3 = amEngine->AddEntity(1);
                REQUIRE(e3.Valid());
                REQUIRE(e3.GetState() == e1.GetState());

                Entity e4 = amEngine->GetEntity(2);
                REQUIRE(e4.Valid());
                REQUIRE(e4.GetState() == e2.GetState());

                Entity e5 = amEngine->GetEntity(3);
                REQUIRE_FALSE(e5.Valid());

                amEngine->RemoveEntity(1);
                amEngine->RemoveEntity(&e2);
                amEngine->RemoveEntity(3);

                REQUIRE_FALSE(e1.Valid());
                REQUIRE_FALSE(e2.Valid());
                REQUIRE_FALSE(e3.Valid());
                REQUIRE_FALSE(e4.Valid());
                REQUIRE_FALSE(e5.Valid());

                amEngine->UnloadSoundBank("init.ambank");
            }

            THEN("it can register listeners")
            {
                Listener l1 = amEngine->AddListener(1);
                Listener l2 = amEngine->AddListener(2);

                REQUIRE(l1.Valid());
                REQUIRE(l2.Valid());

                Listener l3 = amEngine->AddListener(1);
                REQUIRE(l3.Valid());
                REQUIRE(l3.GetState() == l1.GetState());

                Listener l4 = amEngine->GetListener(2);
                REQUIRE(l4.Valid());
                REQUIRE(l4.GetState() == l2.GetState());

                Listener l5 = amEngine->GetListener(3);
                REQUIRE_FALSE(l5.Valid());

                amEngine->RemoveListener(1);
                amEngine->RemoveListener(&l2);
                amEngine->RemoveListener(3);

                REQUIRE_FALSE(l1.Valid());
                REQUIRE_FALSE(l2.Valid());
                REQUIRE_FALSE(l3.Valid());
                REQUIRE_FALSE(l4.Valid());
                REQUIRE_FALSE(l5.Valid());

                amEngine->UnloadSoundBank("init.ambank");
            }

            THEN("it can register environments")
            {
                Environment e1 = amEngine->AddEnvironment(1);
                Environment e2 = amEngine->AddEnvironment(2);

                REQUIRE(e1.Valid());
                REQUIRE(e2.Valid());

                Environment e3 = amEngine->AddEnvironment(1);
                REQUIRE(e3.Valid());
                REQUIRE(e3.GetState() == e1.GetState());

                Environment e4 = amEngine->GetEnvironment(2);
                REQUIRE(e4.Valid());
                REQUIRE(e4.GetState() == e2.GetState());

                Environment e5 = amEngine->GetEnvironment(3);
                REQUIRE_FALSE(e5.Valid());

                amEngine->RemoveEnvironment(1);
                amEngine->RemoveEnvironment(&e2);
                amEngine->RemoveEnvironment(3);

                REQUIRE_FALSE(e1.Valid());
                REQUIRE_FALSE(e2.Valid());
                REQUIRE_FALSE(e3.Valid());
                REQUIRE_FALSE(e4.Valid());
                REQUIRE_FALSE(e5.Valid());

                amEngine->UnloadSoundBank("init.ambank");
            }

            THEN("it can access sound assets by names")
            {
                REQUIRE(amEngine->GetSoundHandle("symphony") != nullptr);
                REQUIRE(amEngine->GetSoundHandle("AMB_Forest") != nullptr);
                REQUIRE(amEngine->GetSoundHandle("throw_01") != nullptr);

                amEngine->UnloadSoundBank("init.ambank");
            }

            THEN("it can access sound assets by IDs")
            {
                REQUIRE(amEngine->GetSoundHandle(101) != nullptr);
                REQUIRE(amEngine->GetSoundHandle(100) != nullptr);
                REQUIRE(amEngine->GetSoundHandle(1) != nullptr);

                amEngine->UnloadSoundBank("init.ambank");
            }

            THEN("it accesses the same sound assets when fetching by name or ID")
            {
                for (AmUInt32 i = 0; i < 8; ++i)
                {
                    const auto id = i + 1;
                    const auto name = "throw_0" + std::to_string(id);

                    REQUIRE(amEngine->GetSoundHandle(name) == amEngine->GetSoundHandle(id));
                }

                amEngine->UnloadSoundBank("init.ambank");
            }

            THEN("it can load the same soundbank again")
            {
                REQUIRE(amEngine->LoadSoundBank(AM_OS_STRING("init.ambank")));
                amEngine->UnloadSoundBank("init.ambank");

                amEngine->UnloadSoundBank("init.ambank");
            }

            THEN("it can load other soundbanks")
            {
                REQUIRE(amEngine->LoadSoundBank(AM_OS_STRING("sample_01.ambank")));
                REQUIRE(amEngine->LoadSoundBank(AM_OS_STRING("sample_02.ambank")));

                amEngine->UnloadSoundBanks();
            }

            THEN("engine can play a sound")
            {
                SoundHandle throw01 = amEngine->GetSoundHandle("throw_01");

                Channel channel = amEngine->Play(throw01);
                REQUIRE(channel.Valid());
                REQUIRE(channel.Playing());

                Thread::Sleep(1000); // wait for the sound to finish playing
                REQUIRE_FALSE(channel.Playing());

                amEngine->UnloadSoundBank("init.ambank");
            }

            THEN("engine can play a collection")
            {
                CollectionHandle throw_collection = amEngine->GetCollectionHandle("throw_collection_1");

                Channel channel = amEngine->Play(throw_collection);
                REQUIRE(channel.Valid());
                REQUIRE(channel.Playing());

                Thread::Sleep(8000); // wait for the sound to finish playing
                REQUIRE_FALSE(channel.Playing());

                amEngine->UnloadSoundBank("init.ambank");
            }

            THEN("engine can play a switch container")
            {
                Entity entity = amEngine->AddEntity(100);
                SwitchContainerHandle footsteps = amEngine->GetSwitchContainerHandle("footsteps");

                Channel channel = amEngine->Play(footsteps);
                REQUIRE_FALSE(channel.Valid()); // switch container is entity scoped

                channel = amEngine->Play(footsteps, entity);
                REQUIRE(channel.Valid());
                REQUIRE(channel.Playing());

                Thread::Sleep(1000); // wait for the sound to finish playing
                REQUIRE_FALSE(channel.Playing());

                amEngine->UnloadSoundBank("init.ambank");
            }

            GIVEN("a playing channel")
            {
                AmVec3 location = { 10.0f, 20.0f, 30.0f };
                AmReal32 userGain = 0.36f;
                Channel channel = amEngine->Play(101, location, userGain);

                REQUIRE(channel.Valid());
                REQUIRE(channel.Playing());

                THEN("a playing channel cannot be resumed")
                {
                    channel.Resume();
                    REQUIRE_FALSE(channel.GetPlaybackState() == ChannelPlaybackState::Paused);
                    REQUIRE_FALSE(channel.GetPlaybackState() == ChannelPlaybackState::FadingOut);
                    REQUIRE(channel.Playing());
                }

                THEN("it can be paused")
                {
                    channel.Pause();
                    REQUIRE(channel.GetPlaybackState() == ChannelPlaybackState::FadingOut);
                    REQUIRE_FALSE(channel.Playing());
                    Thread::Sleep(kAmSecond); // wait for sixty frames
                    REQUIRE(channel.GetPlaybackState() == ChannelPlaybackState::Paused);

                    channel.Resume();
                    REQUIRE(channel.GetPlaybackState() == ChannelPlaybackState::FadingIn);
                    Thread::Sleep(kAmSecond); // wait for sixty frames
                    REQUIRE(channel.GetPlaybackState() == ChannelPlaybackState::Playing);
                    REQUIRE(channel.Playing());

                    channel.Stop(0);
                }

                THEN("it can be stopped")
                {
                    channel.Stop();
                    REQUIRE(channel.GetPlaybackState() == ChannelPlaybackState::FadingOut);
                    REQUIRE_FALSE(channel.Playing());
                    Thread::Sleep(kAmSecond); // wait for sixty frames
                    REQUIRE(channel.GetPlaybackState() == ChannelPlaybackState::Stopped);

                    channel.Resume();
                    Thread::Sleep(kAmSecond); // wait for sixty frames
                    REQUIRE_FALSE(channel.GetPlaybackState() == ChannelPlaybackState::FadingIn);
                    REQUIRE(channel.GetPlaybackState() == ChannelPlaybackState::Stopped);

                    channel.Stop(0);
                }

                THEN("it can be cleared")
                {
                    channel.Stop(0);

                    channel.Clear();
                    REQUIRE_FALSE(channel.Valid());
                }

                THEN("it returns the correct location")
                {
                    AmVec3 result = channel.GetLocation();
                    REQUIRE(AM_EqV3(result, location));

                    WHEN("location is updated")
                    {
                        AmVec3 newLocation = { 100.0f, 200.0f, 300.0f };
                        channel.SetLocation(newLocation);

                        THEN("it returns the new location")
                        {
                            AmVec3 result2 = channel.GetLocation();
                            REQUIRE(AM_EqV3(result2, newLocation));
                        }
                    }

                    channel.Stop(0);
                }

                THEN("it returns the correct gain")
                {
                    AmReal32 result = channel.GetGain();
                    REQUIRE(result == userGain);

                    WHEN("gain is updated")
                    {
                        AmReal32 newGain = 0.5f;
                        channel.SetGain(newGain);

                        THEN("it returns the new gain")
                        {
                            AmReal32 result2 = channel.GetGain();
                            REQUIRE(result2 == newGain);
                        }
                    }

                    channel.Stop(0);
                }

                if (channel.Valid())
                    channel.Stop(0);

                amEngine->UnloadSoundBank("init.ambank");
            }
        }
    }
}
