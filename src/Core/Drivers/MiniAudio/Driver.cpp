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

namespace SparkyStudios::Audio::Amplitude
{
    static void* ma_malloc(size_t sz, void* pUserData)
    {
        AM_UNUSED(pUserData);
        return ampoolmalloc(MemoryPoolKind::Amplimix, sz);
    }

    static void* ma_realloc(void* p, size_t sz, void* pUserData)
    {
        AM_UNUSED(pUserData);
        return ampoolrealloc(MemoryPoolKind::Amplimix, p, sz);
    }

    static void ma_free(void* p, void* pUserData)
    {
        AM_UNUSED(pUserData);
        ampoolfree(MemoryPoolKind::Amplimix, p);
    }

    static void miniaudio_log(void* pUserData, ma_uint32 level, const char* pMessage)
    {
        AM_UNUSED(pUserData);
        AM_UNUSED(level);

        // TODO: Add support for log levels.
        CallLogFunc(pMessage);
    }

    static void miniaudio_mixer(ma_device* pDevice, AmVoidPtr pOutput, AmConstVoidPtr pInput, ma_uint32 frameCount)
    {
        AM_UNUSED(pDevice);
        AM_UNUSED(pInput);

        amEngine->GetMixer()->Mix(pOutput, frameCount);
    }

    void miniaudio_device_notification(const ma_device_notification* pNotification)
    {
        auto* driver = static_cast<MiniAudioDriver*>(pNotification->pDevice->pUserData);

        driver->m_deviceDescription.mDeviceID = 0; // TODO: Compute a proper device ID
        driver->m_deviceDescription.mDeviceName = AmString(pNotification->pDevice->playback.name);
        driver->m_deviceDescription.mDeviceOutputSampleRate = pNotification->pDevice->playback.internalSampleRate;
        driver->m_deviceDescription.mDeviceOutputChannels =
            static_cast<PlaybackOutputChannels>(pNotification->pDevice->playback.internalChannels);
        driver->m_deviceDescription.mDeviceOutputFormat =
            static_cast<PlaybackOutputFormat>(pNotification->pDevice->playback.internalFormat);

        if (auto* mixer = amEngine->GetMixer(); mixer != nullptr)
        {
            mixer->UpdateDevice(
                driver->m_deviceDescription.mDeviceID, driver->m_deviceDescription.mDeviceName,
                driver->m_deviceDescription.mDeviceOutputSampleRate, driver->m_deviceDescription.mDeviceOutputChannels,
                driver->m_deviceDescription.mDeviceOutputFormat);
        }

        switch (pNotification->type)
        {
        case ma_device_notification_type_started:
            driver->m_deviceDescription.mDeviceState = DeviceState::Started;
            CallDeviceNotificationCallback(DeviceNotification::Started, driver->GetDeviceDescription(), driver);
            break;
        case ma_device_notification_type_stopped:
            driver->m_deviceDescription.mDeviceState = DeviceState::Stopped;
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

    static ma_result ma_resampling_backend_get_heap_size_ls(void* pUserData, const ma_resampler_config* pConfig, size_t* pHeapSizeInBytes)
    {
        AM_UNUSED(pConfig);
        AM_UNUSED(pUserData);

        *pHeapSizeInBytes = 0;
        return MA_SUCCESS;
    }

    static ma_result ma_resampling_backend_init_ls(
        void* pUserData, const ma_resampler_config* pConfig, void* pHeap, ma_resampling_backend** ppBackend)
    {
        AM_UNUSED(pHeap);

        auto* pResampler = Resampler::Construct("libsamplerate");
        const auto* pDeviceDescription = static_cast<DeviceDescription*>(pUserData);

        const AmUInt64 maxFramesIn = pDeviceDescription->mOutputBufferSize / pConfig->channels;
        pResampler->Init(pConfig->channels, pConfig->sampleRateIn, pConfig->sampleRateOut, maxFramesIn);

        *ppBackend = pResampler;

        return MA_SUCCESS;
    }

    static void ma_resampling_backend_uninit_ls(
        void* pUserData, ma_resampling_backend* pBackend, const ma_allocation_callbacks* pAllocationCallbacks)
    {
        AM_UNUSED(pUserData);
        AM_UNUSED(pAllocationCallbacks);

        auto* pResampler = static_cast<ResamplerInstance*>(pBackend);
        pResampler->Clear();

        Resampler::Destruct("libsamplerate", pResampler);
    }

    static ma_result ma_resampling_backend_process_ls(
        void* pUserData,
        ma_resampling_backend* pBackend,
        const void* pFramesIn,
        ma_uint64* pFrameCountIn,
        void* pFramesOut,
        ma_uint64* pFrameCountOut)
    {
        AM_UNUSED(pUserData);

        auto* pResampler = static_cast<ResamplerInstance*>(pBackend);

        if (pResampler == nullptr)
            return MA_INVALID_ARGS;

        if (pResampler->GetSampleRateIn() == pResampler->GetSampleRateOut())
        {
            std::memcpy(pFramesOut, pFramesIn, *pFrameCountIn * pResampler->GetChannelCount() * sizeof(AmAudioSample));
            return MA_SUCCESS;
        }

        const bool result = pResampler->Process(
            static_cast<AmConstAudioSampleBuffer>(pFramesIn), *pFrameCountIn, static_cast<AmAudioSampleBuffer>(pFramesOut),
            *pFrameCountOut);

        return result ? MA_SUCCESS : MA_ERROR;
    }

    static ma_result ma_resampling_backend_set_rate_ls(
        void* pUserData, ma_resampling_backend* pBackend, ma_uint32 sampleRateIn, ma_uint32 sampleRateOut)
    {
        AM_UNUSED(pUserData);

        if (auto* pResampler = static_cast<ResamplerInstance*>(pBackend);
            pResampler->GetSampleRateIn() != sampleRateIn || pResampler->GetSampleRateOut() != sampleRateOut)
            pResampler->SetSampleRate(sampleRateIn, sampleRateOut);

        return MA_SUCCESS;
    }

    static ma_uint64 ma_resampling_backend_get_input_latency_ls(void* pUserData, const ma_resampling_backend* pBackend)
    {
        AM_UNUSED(pUserData);

        const auto* pResampler = static_cast<const ResamplerInstance*>(pBackend);

        return pResampler->GetLatencyInFrames();
    }

    static ma_uint64 ma_resampling_backend_get_output_latency_ls(void* pUserData, const ma_resampling_backend* pBackend)
    {
        AM_UNUSED(pUserData);

        const auto* pResampler = static_cast<const ResamplerInstance*>(pBackend);

        return pResampler->GetLatencyInFrames();
    }

    static ma_result ma_resampling_backend_get_required_input_frame_count_ls(
        void* pUserData, const ma_resampling_backend* pBackend, ma_uint64 outputFrameCount, ma_uint64* pInputFrameCount)
    {
        AM_UNUSED(pUserData);

        // Sample rate is the same, so ratio is 1:1
        if (const auto* pResampler = static_cast<const ResamplerInstance*>(pBackend);
            pResampler->GetSampleRateIn() == pResampler->GetSampleRateOut())
            *pInputFrameCount = outputFrameCount;
        else
            *pInputFrameCount = pResampler->GetRequiredInputFrameCount(outputFrameCount);

        return MA_SUCCESS;
    }

    static ma_result ma_resampling_backend_get_expected_output_frame_count_ls(
        void* pUserData, const ma_resampling_backend* pBackend, ma_uint64 inputFrameCount, ma_uint64* pOutputFrameCount)
    {
        AM_UNUSED(pUserData);

        // Sample rate is the same, so ratio is 1:1
        if (const auto* pResampler = static_cast<const ResamplerInstance*>(pBackend);
            pResampler->GetSampleRateIn() == pResampler->GetSampleRateOut())
            *pOutputFrameCount = inputFrameCount;
        else
            *pOutputFrameCount = pResampler->GetExpectedOutputFrameCount(inputFrameCount);

        return MA_SUCCESS;
    }

    static ma_result ma_resampling_backend_reset_ls(void* pUserData, ma_resampling_backend* pBackend)
    {
        AM_UNUSED(pUserData);

        auto* pResampler = static_cast<ResamplerInstance*>(pBackend);
        pResampler->Reset();

        return MA_SUCCESS;
    }

    static ma_allocation_callbacks gAllocationCallbacks = { nullptr, ma_malloc, ma_realloc, ma_free };

    static ma_resampling_backend_vtable gResamplerVTable = { ma_resampling_backend_get_heap_size_ls,
                                                             ma_resampling_backend_init_ls,
                                                             ma_resampling_backend_uninit_ls,
                                                             ma_resampling_backend_process_ls,
                                                             ma_resampling_backend_set_rate_ls,
                                                             ma_resampling_backend_get_input_latency_ls,
                                                             ma_resampling_backend_get_output_latency_ls,
                                                             ma_resampling_backend_get_required_input_frame_count_ls,
                                                             ma_resampling_backend_get_expected_output_frame_count_ls,
                                                             ma_resampling_backend_reset_ls };

    MiniAudioDriver::MiniAudioDriver()
        : Driver("miniaudio")
        , _initialized(false)
        , _device()
        , _logCallback()
        , _log()
        , _context()
        , _devices()
    {
        _logCallback = ma_log_callback_init(miniaudio_log, nullptr);
        if (ma_log_init(nullptr, &_log) != MA_SUCCESS)
        {
            CallLogFunc("Failed to initialize miniaudio log\n");
            return;
        }

        ma_log_register_callback(&_log, _logCallback);

        ma_context_config config = ma_context_config_init();
        config.pLog = &_log;
        config.pUserData = this;
        config.threadPriority = ma_thread_priority_realtime;

        if (ma_context_init(nullptr, 0, &config, &_context) != MA_SUCCESS)
        {
            CallLogFunc("Failed to initialize miniaudio context\n");
            return;
        }
    }

    MiniAudioDriver::~MiniAudioDriver()
    {
        if (_initialized)
            Close();

        ma_log_unregister_callback(&_log, _logCallback);
        ma_log_uninit(&_log);

        ma_context_uninit(&_context);
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
                static_cast<ma_channel*>(ampoolmalloc(MemoryPoolKind::Engine, channelsCount * sizeof(ma_channel)));
            deviceConfig.playback.channelMixMode = ma_channel_mix_mode_rectangular;
            deviceConfig.sampleRate = device.mRequestedOutputSampleRate;
            deviceConfig.dataCallback = miniaudio_mixer;
            deviceConfig.notificationCallback = miniaudio_device_notification;
            deviceConfig.pUserData = static_cast<void*>(this);
            deviceConfig.resampling.algorithm = ma_resample_algorithm_custom;
            deviceConfig.resampling.pBackendVTable = &gResamplerVTable;
            deviceConfig.resampling.pBackendUserData = static_cast<void*>(&m_deviceDescription);

            ma_channel_map_init_standard(ma_standard_channel_map_vorbis, deviceConfig.playback.pChannelMap, channelsCount, channelsCount);

            _initialized = ma_device_init(&_context, &deviceConfig, &_device) == MA_SUCCESS;
            if (!_initialized)
            {
                CallLogFunc("The miniaudio driver was not initialized successfully.");
                return false;
            }

            m_deviceDescription = device;

            m_deviceDescription.mDeviceID = 0; // TODO: Compute a proper device ID
            m_deviceDescription.mDeviceName = AmString(_device.playback.name);
            m_deviceDescription.mDeviceOutputSampleRate = _device.playback.internalSampleRate;
            m_deviceDescription.mDeviceOutputChannels = static_cast<PlaybackOutputChannels>(_device.playback.internalChannels);
            m_deviceDescription.mDeviceOutputFormat = static_cast<PlaybackOutputFormat>(_device.playback.internalFormat);
            m_deviceDescription.mDeviceState = DeviceState::Opened;

            CallDeviceNotificationCallback(DeviceNotification::Opened, m_deviceDescription, this);

            amEngine->GetMixer()->UpdateDevice(
                m_deviceDescription.mDeviceID, m_deviceDescription.mDeviceName, m_deviceDescription.mDeviceOutputSampleRate,
                m_deviceDescription.mDeviceOutputChannels, m_deviceDescription.mDeviceOutputFormat);

            ampoolfree(MemoryPoolKind::Engine, deviceConfig.playback.pChannelMap);
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

        m_deviceDescription.mDeviceState = DeviceState::Closed;

        ma_device_uninit(&_device);
        _initialized = false;

        return true;
    }

    bool MiniAudioDriver::EnumerateDevices(std::vector<DeviceDescription>& devices)
    {
        if (_devices.empty())
        {
            ma_device_info* pPlaybackInfos;
            ma_uint32 playbackCount;
            ma_device_info* pCaptureInfos;
            ma_uint32 captureCount;

            if (ma_context_get_devices(&_context, &pPlaybackInfos, &playbackCount, &pCaptureInfos, &captureCount) != MA_SUCCESS)
            {
                CallLogFunc("Unable to enumerate the audio devices.\n");
                return false;
            }

            _devices.resize(playbackCount);
            for (ma_uint32 iDevice = 0; iDevice < playbackCount; iDevice++)
            {
                _devices[iDevice].mDeviceID = iDevice;
                _devices[iDevice].mDeviceName = AmString(pPlaybackInfos[iDevice].name);
                _devices[iDevice].mDeviceOutputSampleRate = pPlaybackInfos[iDevice].nativeDataFormats[0].sampleRate;
                _devices[iDevice].mDeviceOutputChannels =
                    static_cast<PlaybackOutputChannels>(pPlaybackInfos[iDevice].nativeDataFormats[0].channels);
                _devices[iDevice].mDeviceOutputFormat =
                    static_cast<PlaybackOutputFormat>(pPlaybackInfos[iDevice].nativeDataFormats[0].format);
                _devices[iDevice].mDeviceState = DeviceState::Closed;
            }
        }

        devices = _devices;

        return true;
    }
} // namespace SparkyStudios::Audio::Amplitude
