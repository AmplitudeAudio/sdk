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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/Drivers/MiniAudio/Driver.h>
#include <Mixer/Mixer.h>

namespace SparkyStudios::Audio::Amplitude::Drivers
{
    static void miniaudio_mixer(ma_device* pDevice, AmVoidPtr pOutput, AmConstVoidPtr pInput, ma_uint32 frameCount)
    {
        auto* mixer = static_cast<Mixer*>(pDevice->pUserData);
        mixer->Mix(pOutput, frameCount);
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
            deviceConfig.pUserData = static_cast<void*>(m_mixer);

            ma_channel_map_init_standard(ma_standard_channel_map_vorbis, deviceConfig.playback.pChannelMap, channelsCount, channelsCount);

            _initialized = ma_device_init(nullptr, &deviceConfig, &_device) == MA_SUCCESS;
            if (!_initialized)
            {
                CallLogFunc("The miniaudio driver was not initialized successfully.");
                return false;
            }

            m_mixer->PostInit(
                0, // TODO: Compute a proper device ID
                _device.playback.name, _device.playback.internalSampleRate,
                static_cast<PlaybackOutputChannels>(_device.playback.internalChannels),
                static_cast<PlaybackOutputFormat>(_device.playback.internalFormat));
        }

        if (ma_device_is_started(&_device) == MA_FALSE && ma_device_start(&_device) != MA_SUCCESS)
        {
            CallLogFunc("Unable to open the audio device.");
            return false;
        }

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

        return true;
    }
} // namespace SparkyStudios::Audio::Amplitude::Drivers
