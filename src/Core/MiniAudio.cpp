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

#include <Utils/stb_vorbis.h>

#include <Core/Drivers/MiniAudio/miniaudio.h>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Core/Engine.h>

#include "Mixer.h"
#include "engine_config_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    ma_device gDevice;

    void miniaudio_mixer(ma_device* pDevice, AmVoidPtr pOutput, const void* pInput, ma_uint32 frameCount)
    {
        auto* mixer = (Mixer*)pDevice->pUserData;
        mixer->Mix(pOutput, frameCount);
    }

    void miniaudio_uninit(Mixer* mixer)
    {
        ma_device_uninit(&gDevice);
    }

    bool miniaudio_init(Mixer* mixer, const EngineConfigDefinition* audioEngineConfig, ma_format format)
    {
        ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
        deviceConfig.periodSizeInFrames = audioEngineConfig->output()->buffer_size();
        deviceConfig.playback.format = format;
        deviceConfig.playback.channels = audioEngineConfig->output()->channels();
        deviceConfig.sampleRate = audioEngineConfig->output()->frequency();
        deviceConfig.dataCallback = miniaudio_mixer;
        deviceConfig.pUserData = (void*)mixer;

        if (ma_device_init(nullptr, &deviceConfig, &gDevice) != MA_SUCCESS)
        {
            return false;
        }

        mixer->PostInit(
            gDevice.playback.internalPeriodSizeInFrames, gDevice.playback.internalSampleRate, gDevice.playback.internalChannels);

        ma_device_start(&gDevice);

        return true;
    }
}; // namespace SparkyStudios::Audio::Amplitude
