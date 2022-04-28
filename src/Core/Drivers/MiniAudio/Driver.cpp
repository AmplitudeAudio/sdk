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

#include <Core/Drivers/MiniAudio/Driver.h>
#include <Mixer/Mixer.h>

namespace SparkyStudios::Audio::Amplitude::Drivers
{
    static void miniaudio_mixer(ma_device* pDevice, AmVoidPtr pOutput, AmConstVoidPtr pInput, ma_uint32 frameCount)
    {
        amEngine->GetMixer()->Mix(pOutput, frameCount);
    }

    static void miniaudio_device_notification(const ma_device_notification* pNotification)
    {
        auto* driver = static_cast<Driver*>(pNotification->pDevice->pUserData);
        switch (pNotification->type)
        {
        case ma_device_notification_type_started:
            CallDeviceNotificationCallback(DeviceNotification::Started, driver->GetDeviceDescription(), driver);
            break;
        case ma_device_notification_type_stopped:
            CallDeviceNotificationCallback(DeviceNotification::Stopped, driver->GetDeviceDescription(), driver);
            break;
        case ma_device_notification_type_rerouted:
            CallDeviceNotificationCallback(DeviceNotification::Rerouted, driver->GetDeviceDescription(), driver);
            break;
        case ma_device_notification_type_interruption_began:
            CallLogFunc("Device interruption began");
            break;
        case ma_device_notification_type_interruption_ended:
            CallLogFunc("Device interruption ended");
            break;
        }
    }

    MiniAudioDriver::~MiniAudioDriver()
    {
        if (_initialized)
        {
            Close();
            ma_device_uninit(&_device);
        }
    }

    bool MiniAudioDriver::Open(const DeviceDescription& device)
    {
        if (!_initialized)
        {
            const auto channelsCount = static_cast<AmInt16>(device.mRequestedOutputChannels);

            ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
            deviceConfig.noClip = true;
            deviceConfig.noPreSilencedOutputBuffer = true;
            deviceConfig.periodSizeInFrames = device.mOutputBufferSize / channelsCount;
            deviceConfig.playback.format = ma_format_from_amplitude(device.mRequestedOutputFormat);
            deviceConfig.playback.channels = channelsCount;
            deviceConfig.playback.pChannelMap =
                static_cast<ma_channel*>(amMemory->Malloc(MemoryPoolKind::Engine, channelsCount * sizeof(ma_channel)));
            deviceConfig.playback.channelMixMode = ma_channel_mix_mode_rectangular;
            deviceConfig.sampleRate = device.mRequestedOutputSampleRate;
            deviceConfig.dataCallback = miniaudio_mixer;
            deviceConfig.notificationCallback = miniaudio_device_notification;
            deviceConfig.pUserData = static_cast<void*>(this);

            ma_channel_map_init_standard(ma_standard_channel_map_vorbis, deviceConfig.playback.pChannelMap, channelsCount, channelsCount);

            _initialized = ma_device_init(nullptr, &deviceConfig, &_device) == MA_SUCCESS;
            if (!_initialized)
            {
                CallLogFunc("The miniaudio driver was not initialized successfully.");
                return false;
            }

            m_deviceDescription.mDeviceID = 0;
            m_deviceDescription.mDeviceName = std::string(_device.playback.name);
            m_deviceDescription.mDeviceOutputSampleRate = _device.playback.internalSampleRate;
            m_deviceDescription.mDeviceOutputChannels = static_cast<PlaybackOutputChannels>(_device.playback.internalChannels);
            m_deviceDescription.mDeviceOutputFormat = static_cast<PlaybackOutputFormat>(_device.playback.internalFormat);
            m_deviceDescription.mDeviceState = DeviceState::Opened;

            CallDeviceNotificationCallback(DeviceNotification::Opened, m_deviceDescription, this);

            amEngine->GetMixer()->PostInit(
                0, // TODO: Compute a proper device ID
                _device.playback.name, _device.playback.internalSampleRate,
                static_cast<PlaybackOutputChannels>(_device.playback.internalChannels),
                static_cast<PlaybackOutputFormat>(_device.playback.internalFormat));

            amMemory->Free(MemoryPoolKind::Engine, deviceConfig.playback.pChannelMap);
        }

        if (ma_device_is_started(&_device) == MA_FALSE && ma_device_start(&_device) != MA_SUCCESS)
        {
            CallLogFunc("Unable to open the audio device.");
            return false;
        }

        m_deviceDescription.mDeviceState = DeviceState::Started;
        return true;
    }

    bool MiniAudioDriver::Close()
    {
        if (!_initialized)
        {
            CallLogFunc("Cannot close an uninitialized audio device.");
            return false;
        }

        if (ma_device_is_started(&_device) == MA_TRUE && ma_device_stop(&_device) != MA_SUCCESS)
        {
            CallLogFunc("Unable to close the audio device.");
            return false;
        }

        _initialized = false;
        m_deviceDescription.mDeviceState = DeviceState::Closed;

        CallDeviceNotificationCallback(DeviceNotification::Closed, m_deviceDescription, this);
        return true;
    }
} // namespace SparkyStudios::Audio::Amplitude::Drivers
