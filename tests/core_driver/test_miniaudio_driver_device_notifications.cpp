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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "MiniAudioEngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    static bool sMiniAudioCallbackCalled = false;
    static eDeviceNotification sMiniAudioReceivedNotification = eDeviceNotification_Opened;
    static AmString sMiniAudioReceivedDeviceName;
    static Driver* sMiniAudioReceivedDriver = nullptr;

    static void TestMiniAudioDeviceNotificationCallback(eDeviceNotification notification, const DeviceDescription& device, Driver* driver)
    {
        sMiniAudioCallbackCalled = true;
        sMiniAudioReceivedNotification = notification;
        sMiniAudioReceivedDeviceName = device.mDeviceName;
        sMiniAudioReceivedDriver = driver;
    }

    AM_TEST_CASE(MiniAudioEngineTestCase, core_driver, miniaudio_driver_device_notifications)
    {
    public:
        void SetUp() override
        {
            sMiniAudioCallbackCalled = false;
            sMiniAudioReceivedNotification = eDeviceNotification_Opened;
            sMiniAudioReceivedDeviceName.clear();
            sMiniAudioReceivedDriver = nullptr;

            RegisterDeviceNotificationCallback(TestMiniAudioDeviceNotificationCallback);

            MiniAudioEngineTestCase::SetUp();
        }

        void TearDown() override
        {
            MiniAudioEngineTestCase::TearDown();

            UnregisterDeviceNotificationCallback();
        }

        void Run() override
        {
            auto driver = amEngine->GetDriver();
            AM_EXPECT(driver != nullptr);

            // Skip MiniAudio-specific tests if the engine fell back to null driver
            // (e.g., in CI environments without audio hardware)
            if (std::string(driver->GetName()) != "miniaudio")
                return;

            AM_EXPECT(sMiniAudioCallbackCalled);
            AM_EXPECT(
                sMiniAudioReceivedNotification == eDeviceNotification_Opened ||
                sMiniAudioReceivedNotification == eDeviceNotification_Started);
            AM_EXPECT(sMiniAudioReceivedDriver == driver.get());
            AM_EXPECT(!sMiniAudioReceivedDeviceName.empty());

            sMiniAudioCallbackCalled = false;

            AM_EXPECT(Deinitialize());

            AM_EXPECT(sMiniAudioCallbackCalled);
            AM_EXPECT(sMiniAudioReceivedNotification == eDeviceNotification_Stopped);
        }
    };

    AM_REGISTER_TEST(core_driver, miniaudio_driver_device_notifications);
} // namespace SparkyStudios::Audio::Amplitude::Tests
