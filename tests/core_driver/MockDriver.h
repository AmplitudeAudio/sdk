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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

namespace SparkyStudios::Audio::Amplitude::Tests
{
    class MockDriver final : public Driver
    {
    public:
        explicit MockDriver(const AmString& name)
            : Driver(name)
            , _openCalled(false)
            , _closeCalled(false)
            , _enumerateCalled(false)
            , _openResult(true)
            , _closeResult(true)
            , _enumerateResult(true)
        {}

        ~MockDriver() override = default;

        bool Open(const DeviceDescription& device) override
        {
            _openCalled = true;
            _lastDevice = device;
            return _openResult;
        }

        bool Close() override
        {
            _closeCalled = true;
            return _closeResult;
        }

        bool EnumerateDevices(std::vector<DeviceDescription>& devices) override
        {
            _enumerateCalled = true;
            devices = _mockDevices;
            return _enumerateResult;
        }

        void SetOpenResult(bool result)
        {
            _openResult = result;
        }

        void SetCloseResult(bool result)
        {
            _closeResult = result;
        }

        void SetEnumerateResult(bool result)
        {
            _enumerateResult = result;
        }

        void SetMockDevices(const std::vector<DeviceDescription>& devices)
        {
            _mockDevices = devices;
        }

        [[nodiscard]] bool WasOpenCalled() const
        {
            return _openCalled;
        }

        [[nodiscard]] bool WasCloseCalled() const
        {
            return _closeCalled;
        }

        [[nodiscard]] bool WasEnumerateCalled() const
        {
            return _enumerateCalled;
        }

        [[nodiscard]] const DeviceDescription& GetLastDevice() const
        {
            return _lastDevice;
        }

        void Reset()
        {
            _openCalled = false;
            _closeCalled = false;
            _enumerateCalled = false;
            _lastDevice = DeviceDescription{};
        }

    private:
        bool _openCalled;
        bool _closeCalled;
        bool _enumerateCalled;
        bool _openResult;
        bool _closeResult;
        bool _enumerateResult;
        DeviceDescription _lastDevice;
        std::vector<DeviceDescription> _mockDevices;
    };
} // namespace SparkyStudios::Audio::Amplitude::Tests
