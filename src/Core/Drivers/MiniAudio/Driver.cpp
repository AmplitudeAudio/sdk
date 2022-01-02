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

#include "engine_config_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude::Drivers
{
    static void miniaudio_mixer(ma_device* pDevice, AmVoidPtr pOutput, AmConstVoidPtr pInput, ma_uint32 frameCount)
    {
        auto* mixer = static_cast<Mixer*>(pDevice->pUserData);
        mixer->Mix(pOutput, frameCount);
    }

    static ma_format ma_format_from_amplitude(PlaybackOutputFormat format)
    {
        switch (format)
        {
        default:
        case PlaybackOutputFormat_Default:
            return ma_format_unknown;
        case PlaybackOutputFormat_UInt8:
            return ma_format_u8;
        case PlaybackOutputFormat_Int16:
            return ma_format_s16;
        case PlaybackOutputFormat_Int24:
            return ma_format_s24;
        case PlaybackOutputFormat_Int32:
            return ma_format_s32;
        case PlaybackOutputFormat_Float32:
            return ma_format_f32;
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

    bool MiniAudioDriver::Open(const EngineConfigDefinition* config)
    {
        if (!_initialized)
        {
            ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
            deviceConfig.periodSizeInFrames = config->output()->buffer_size();
            deviceConfig.playback.format = ma_format_from_amplitude(config->output()->format());
            deviceConfig.playback.channels = config->output()->channels();
            deviceConfig.sampleRate = config->output()->frequency();
            deviceConfig.dataCallback = miniaudio_mixer;
            deviceConfig.pUserData = static_cast<void*>(m_mixer);

            _initialized = ma_device_init(nullptr, &deviceConfig, &_device) == MA_SUCCESS;
            if (!_initialized)
            {
                CallLogFunc("The miniaudio driver was not initialized successfully.");
                return false;
            }

            m_mixer->PostInit(
                _device.playback.internalPeriodSizeInFrames, _device.playback.internalSampleRate,
                static_cast<AmUInt16>(_device.playback.internalChannels));
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
