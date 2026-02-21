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

#include "EngineTestCase.h"
#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    static bool sCallbackCalled = false;
    static eDeviceNotification sReceivedNotification = eDeviceNotification_Opened;
    static AmString sReceivedDeviceName;

    static void TestDeviceNotificationCallback(eDeviceNotification notification, const DeviceDescription& device, Driver* driver)
    {
        sCallbackCalled = true;
        sReceivedNotification = notification;
        sReceivedDeviceName = device.mDeviceName;
    }

    AM_TEST_CASE(ComponentTestCase, core_driver, can_register_device_notification_callback)
    {
    public:
        void Run() override
        {
            sCallbackCalled = false;
            sReceivedNotification = eDeviceNotification_Opened;

            RegisterDeviceNotificationCallback(TestDeviceNotificationCallback);

            AM_EXPECT_NOT(sCallbackCalled);

            UnregisterDeviceNotificationCallback();
        }
    };

    AM_TEST_CASE(EngineTestCase, core_driver, device_notification_callback_is_called_on_open)
    {
    public:
        void SetUp() override
        {
            sCallbackCalled = false;
            sReceivedNotification = eDeviceNotification_Opened;
            sReceivedDeviceName.clear();

            RegisterDeviceNotificationCallback(TestDeviceNotificationCallback);

            EngineTestCase::SetUp();
        }

        void TearDown() override
        {
            EngineTestCase::TearDown();

            UnregisterDeviceNotificationCallback();
        }

        void Run() override
        {
            AM_EXPECT(sCallbackCalled);
            AM_EXPECT(sReceivedNotification == eDeviceNotification_Opened || sReceivedNotification == eDeviceNotification_Started);
        }
    };

    AM_REGISTER_TEST(core_driver, device_notification_callback_is_called_on_open);
    AM_REGISTER_TEST(core_driver, can_register_device_notification_callback);
} // namespace SparkyStudios::Audio::Amplitude::Tests
