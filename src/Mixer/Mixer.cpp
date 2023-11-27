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

#include <Core/EngineInternalState.h>
#include <Mixer/Mixer.h>
#include <Utils/Utils.h>

#define AMPLIMIX_STORE(A, C) std::atomic_store_explicit(A, C, std::memory_order_release)
#define AMPLIMIX_LOAD(A) std::atomic_load_explicit(A, std::memory_order_acquire)
#define AMPLIMIX_CSWAP(A, E, C) std::atomic_compare_exchange_strong_explicit(A, E, C, std::memory_order_acq_rel, std::memory_order_acquire)

namespace SparkyStudios::Audio::Amplitude
{
#if defined(AM_SIMD_INTRINSICS)
    constexpr AmUInt32 kProcessedFramesCount = AmAudioFrame::length;
#else
    constexpr AmUInt32 kProcessedFramesCount = 1;
#endif // AM_SIMD_INTRINSICS

    static void OnSoundDestroyed(Mixer* mixer, MixerLayer* layer);

    static void* ma_malloc(size_t sz, void*)
    {
        return amMemory->Malloc(MemoryPoolKind::Amplimix, sz);
    }

    static void* ma_realloc(void* p, size_t sz, void*)
    {
        return amMemory->Realloc(MemoryPoolKind::Amplimix, p, sz);
    }

    static void ma_free(void* p, void*)
    {
        amMemory->Free(MemoryPoolKind::Amplimix, p);
    }

    static ma_result ma_resampling_backend_get_heap_size_ls(void* pUserData, const ma_resampler_config* pConfig, size_t* pHeapSizeInBytes)
    {
        (void)pConfig;
        (void)pUserData;

        *pHeapSizeInBytes = 0;
        return MA_SUCCESS;
    }

    static ma_result ma_resampling_backend_init_ls(
        void* pUserData, const ma_resampler_config* pConfig, void* pHeap, ma_resampling_backend** ppBackend)
    {
        (void)pHeap;

        auto* pResampler = Resampler::Construct("libsamplerate");
        auto* pMixerLayer = static_cast<MixerLayer*>(pUserData);

        AmUInt64 maxFramesIn = pMixerLayer->end - pMixerLayer->start;
        pResampler->Init(pConfig->channels, pConfig->sampleRateIn, pConfig->sampleRateOut, maxFramesIn);

        *ppBackend = pResampler;

        return MA_SUCCESS;
    }

    static void ma_resampling_backend_uninit_ls(
        void* pUserData, ma_resampling_backend* pBackend, const ma_allocation_callbacks* pAllocationCallbacks)
    {
        (void)pUserData;
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
        (void)pUserData;
        auto* pResampler = static_cast<ResamplerInstance*>(pBackend);

        if (pResampler == nullptr)
            return MA_INVALID_ARGS;

        if (pResampler->GetSampleRateIn() == pResampler->GetSampleRateOut())
        {
            std::memcpy(pFramesOut, pFramesIn, *pFrameCountIn * pResampler->GetChannelCount() * sizeof(AmAudioSample));
            return MA_SUCCESS;
        }

        bool result = pResampler->Process(
            (AmAudioSampleBuffer)pFramesIn, *pFrameCountIn, static_cast<AmAudioSampleBuffer>(pFramesOut), *pFrameCountOut);

        return result ? MA_SUCCESS : MA_ERROR;
    }

    static ma_result ma_resampling_backend_set_rate_ls(
        void* pUserData, ma_resampling_backend* pBackend, ma_uint32 sampleRateIn, ma_uint32 sampleRateOut)
    {
        (void)pUserData;
        auto* pResampler = static_cast<ResamplerInstance*>(pBackend);

        if (pResampler->GetSampleRateIn() != sampleRateIn || pResampler->GetSampleRateOut() != sampleRateOut)
            pResampler->SetSampleRate(sampleRateIn, sampleRateOut);

        return MA_SUCCESS;
    }

    static ma_uint64 ma_resampling_backend_get_input_latency_ls(void* pUserData, const ma_resampling_backend* pBackend)
    {
        (void)pUserData;
        const auto* pResampler = static_cast<const ResamplerInstance*>(pBackend);

        return pResampler->GetLatencyInFrames();
    }

    static ma_uint64 ma_resampling_backend_get_output_latency_ls(void* pUserData, const ma_resampling_backend* pBackend)
    {
        (void)pUserData;
        const auto* pResampler = static_cast<const ResamplerInstance*>(pBackend);

        return pResampler->GetLatencyInFrames();
    }

    static ma_result ma_resampling_backend_get_required_input_frame_count_ls(
        void* pUserData, const ma_resampling_backend* pBackend, ma_uint64 outputFrameCount, ma_uint64* pInputFrameCount)
    {
        (void)pUserData;
        const auto* pResampler = static_cast<const ResamplerInstance*>(pBackend);

        // Sample rate is the same, so ratio is 1:1
        if (pResampler->GetSampleRateIn() == pResampler->GetSampleRateOut())
            *pInputFrameCount = outputFrameCount;
        else
            *pInputFrameCount = pResampler->GetRequiredInputFrameCount(outputFrameCount);

        return MA_SUCCESS;
    }

    static ma_result ma_resampling_backend_get_expected_output_frame_count_ls(
        void* pUserData, const ma_resampling_backend* pBackend, ma_uint64 inputFrameCount, ma_uint64* pOutputFrameCount)
    {
        (void)pUserData;
        const auto* pResampler = static_cast<const ResamplerInstance*>(pBackend);

        // Sample rate is the same, so ratio is 1:1
        if (pResampler->GetSampleRateIn() == pResampler->GetSampleRateOut())
            *pOutputFrameCount = inputFrameCount;
        else
            *pOutputFrameCount = pResampler->GetExpectedOutputFrameCount(inputFrameCount);

        return MA_SUCCESS;
    }

    static ma_result ma_resampling_backend_reset_ls(void* pUserData, ma_resampling_backend* pBackend)
    {
        (void)pUserData;
        auto* pResampler = static_cast<ResamplerInstance*>(pBackend);

        pResampler->Reset();

        return MA_SUCCESS;
    }

    static AmVec2 LRGain(float gain, float pan)
    {
        // Clamp pan to its valid range of -1.0f to 1.0f inclusive
        pan = AM_CLAMP(pan, -1.0f, 1.0f);

        // Convert gain and pan to left and right gain and store it atomically
        // This formula is explained in the following paper:
        // http://www.rs-met.com/documents/tutorials/PanRules.pdf
        const float p = static_cast<float>(M_PI) * (pan + 1.0f) / 4.0f;
        const float left = std::cos(p) * gain;
        const float right = std::sin(p) * gain;

        return { left, right };
    }

    static bool ShouldLoopSound(Mixer* mixer, MixerLayer* layer)
    {
        const auto* sound = layer->snd->sound.get();
        const AmUInt32 loopCount = sound->GetSettings().m_loopCount;

        return sound->GetCurrentLoopCount() != loopCount;
    }

    static void OnSoundStarted(Mixer* mixer, MixerLayer* layer)
    {
        const auto* sound = layer->snd->sound.get();
        CallLogFunc("Started sound: " AM_OS_CHAR_FMT "\n", sound->GetSound()->GetPath().c_str());
    }

    static void OnSoundPaused(Mixer* mixer, MixerLayer* layer)
    {
        const auto* sound = layer->snd->sound.get();
        CallLogFunc("Paused sound: " AM_OS_CHAR_FMT "\n", sound->GetSound()->GetPath().c_str());
    }

    static void OnSoundResumed(Mixer* mixer, MixerLayer* layer)
    {
        const auto* sound = layer->snd->sound.get();
        CallLogFunc("Resumed sound: " AM_OS_CHAR_FMT "\n", sound->GetSound()->GetPath().c_str());
    }

    static void OnSoundStopped(Mixer* mixer, MixerLayer* layer)
    {
        const auto* sound = layer->snd->sound.get();
        CallLogFunc("Stopped sound: " AM_OS_CHAR_FMT "\n", sound->GetSound()->GetPath().c_str());

        // Destroy the sound instance on stop
        OnSoundDestroyed(mixer, layer);
    }

    static bool OnSoundLooped(Mixer* mixer, MixerLayer* layer)
    {
        auto* sound = layer->snd->sound.get();
        CallLogFunc("Looped sound: " AM_OS_CHAR_FMT "\n", sound->GetSound()->GetPath().c_str());

        Mixer::IncrementSoundLoopCount(sound);

        return ShouldLoopSound(mixer, layer);
    }

    static AmUInt64 OnSoundStream(Mixer* mixer, MixerLayer* layer, AmUInt64 offset, AmUInt64 frames)
    {
        if (layer->snd->stream)
        {
            const auto* sound = layer->snd->sound.get();
            return sound->GetAudio(offset, frames);
        }

        return 0;
    }

    static void OnSoundEnded(Mixer* mixer, MixerLayer* layer)
    {
        auto* sound = layer->snd->sound.get();
        CallLogFunc("Ended sound: " AM_OS_CHAR_FMT "\n", sound->GetSound()->GetPath().c_str());

        RealChannel* channel = sound->GetChannel();

        // Clean up the pipeline
        mixer->GetPipeline()->Cleanup(sound);

        if (const Engine* engine = Engine::GetInstance(); engine->GetState()->stopping)
        {
            OnSoundDestroyed(mixer, layer);
        }

        if (sound->GetSettings().m_kind == SoundKind::Standalone)
        {
            // Stop playing the sound
            channel->GetParentChannelState()->Halt();
        }
        else if (sound->GetSettings().m_kind == SoundKind::Switched)
        {
            OnSoundDestroyed(mixer, layer);
        }
        else if (sound->GetSettings().m_kind == SoundKind::Contained)
        {
            const Collection* collection = sound->GetCollection();
            AMPLITUDE_ASSERT(collection != nullptr); // Should always have a collection for contained sound instances.

            if (const CollectionDefinition* config = collection->GetDefinition(); config->play_mode() == CollectionPlayMode_PlayAll)
            {
                if (channel->Valid())
                {
                    channel->MarkAsPlayed(sound->GetSound());
                    if (channel->AllSoundsHasPlayed())
                    {
                        channel->ClearPlayedSounds();
                        if (config->play_mode() == CollectionPlayMode_PlayAll)
                        {
                            // Stop playing the sound
                            channel->GetParentChannelState()->Halt();
                        }
                    }

                    // Play the collection again only if the channel is still playing.
                    if (channel->Playing())
                    {
                        channel->GetParentChannelState()->Play();
                    }
                }

                // Delete the current sound instance.
                OnSoundDestroyed(mixer, layer);
            }
        }
        else
        {
            AMPLITUDE_ASSERT(false); // Should never fall in this case.
        }
    }

    static void OnSoundDestroyed(Mixer* mixer, MixerLayer* layer)
    {
        if (layer->snd == nullptr)
            return;

        layer->snd->sound.reset();

        layer->snd = nullptr;
    }

    static void MixMono(AmUInt64 index, const AmAudioFrame& gain, const SoundChunk* in, AmAudioFrameBuffer out)
    {
#if defined(AM_SIMD_INTRINSICS)
#if defined(AM_SIMD_ARCH_FMA3)
        out[index] = simdpp::fmadd(in->buffer[index], gain, out[index]);
#else
        out[index] = simdpp::add(out[index], simdpp::mul(in->buffer[index], gain));
#endif // AM_SIMD_ARCH_FMA3
#else
        out[index] = out[index] + in->buffer[index] * gain;
#endif // AM_SIMD_INTRINSICS
    }

    static void MixStereo(
        AmUInt64 index, const AmAudioFrame& lGain, const AmAudioFrame& rGain, const SoundChunk* in, AmAudioFrameBuffer out)
    {
#if defined(AM_SIMD_INTRINSICS)
#if defined(AM_SIMD_ARCH_FMA3)
        out[index + 0] = simdpp::fmadd(in->buffer[index + 0], lGain, out[index + 0]);
        out[index + 1] = simdpp::fmadd(in->buffer[index + 1], lGain, out[index + 1]);
#else
        out[index + 0] = simdpp::add(out[index + 0], simdpp::mul(in->buffer[index + 0], lGain));
        out[index + 1] = simdpp::add(out[index + 1], simdpp::mul(in->buffer[index + 1], rGain));
#endif // AM_SIMD_ARCH_FMA3
#else
        out[index + 0] = out[index + 0] + in->buffer[index + 0] * lGain;
        out[index + 1] = out[index + 1] + in->buffer[index + 1] * rGain;
#endif // AM_SIMD_INTRINSICS
    }

    // Setup MiniAudio allocation callbacks for this frame
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

    Mixer::Mixer(float masterGain)
        : _initialized(false)
        , _commandsStack()
        , _audioThreadMutex(nullptr)
        , _insideAudioThreadMutex()
        , _masterGain()
        , _nextId(0)
        , _layers()
        , _remainingFrames(0)
        , _pipeline(nullptr)
        , _device()
    {
        AMPLIMIX_STORE(&_masterGain, masterGain);
    }

    Mixer::~Mixer()
    {
        Deinit(); // Ensures Deinit is called
    }

    bool Mixer::Init(const EngineConfigDefinition* config)
    {
        if (_initialized)
        {
            CallLogFunc("Amplimix has already been initialized.\n");
            return false;
        }

        _device.mOutputBufferSize = config->output()->buffer_size();
        _device.mRequestedOutputSampleRate = config->output()->frequency();
        _device.mRequestedOutputChannels = static_cast<PlaybackOutputChannels>(config->output()->channels());
        _device.mRequestedOutputFormat = static_cast<PlaybackOutputFormat>(config->output()->format());

        _audioThreadMutex = Thread::CreateMutex(500);

        if (const auto* pipeline = config->mixer()->pipeline(); pipeline != nullptr && pipeline->size() > 0)
        {
            _pipeline = amnew(ProcessorPipeline);

            for (flatbuffers::uoffset_t i = 0, l = pipeline->size(); i < l; ++i)
            {
                switch (config->mixer()->pipeline_type()->Get(i))
                {
                case AudioMixerPipelineItem_AudioProcessorMixer:
                    {
                        const auto* p = pipeline->GetAs<AudioProcessorMixer>(i);
                        SoundProcessorInstance* dryProcessor = SoundProcessor::Construct(p->dry_processor()->str());
                        SoundProcessorInstance* wetProcessor = SoundProcessor::Construct(p->wet_processor()->str());

                        if (dryProcessor == nullptr)
                        {
                            CallLogFunc(
                                "[WARNING] Unable to find a registered sound processor with name: %s\n", p->dry_processor()->c_str());
                            continue;
                        }

                        if (wetProcessor == nullptr)
                        {
                            CallLogFunc(
                                "[WARNING] Unable to find a registered sound processor with name: %s\n", p->wet_processor()->c_str());
                            continue;
                        }

                        auto* mixer = amnew(ProcessorMixer);
                        mixer->SetDryProcessor(dryProcessor, p->dry());
                        mixer->SetWetProcessor(wetProcessor, p->wet());

                        _pipeline->Append(mixer);
                    }
                    break;

                case AudioMixerPipelineItem_AudioSoundProcessor:
                    {
                        const auto* p = pipeline->GetAs<AudioSoundProcessor>(i);
                        SoundProcessorInstance* soundProcessor = SoundProcessor::Construct(p->processor()->str());
                        if (soundProcessor == nullptr)
                        {
                            CallLogFunc("[WARNING] Unable to find a registered sound processor with name: %s\n", p->processor()->c_str());
                            continue;
                        }

                        _pipeline->Append(soundProcessor);
                    }
                    break;

                default:
                    AMPLITUDE_ASSERT(false);
                    break;
                }
            }
        }

        _initialized = true;

        return true;
    }

    void Mixer::Deinit()
    {
        if (!_initialized)
            return;

        AMPLITUDE_ASSERT(!IsInsideThreadMutex());

        _initialized = false;

        if (_audioThreadMutex)
            Thread::DestroyMutex(_audioThreadMutex);

        _audioThreadMutex = nullptr;

        amdelete(ProcessorPipeline, _pipeline);
        _pipeline = nullptr;

        for (auto& layer : _layers)
            layer.Reset();
    }

    void Mixer::UpdateDevice(
        AmObjectID deviceID,
        std::string deviceName,
        AmUInt32 deviceOutputSampleRate,
        PlaybackOutputChannels deviceOutputChannels,
        PlaybackOutputFormat deviceOutputFormat)
    {
        _device.mDeviceID = deviceID;
        _device.mDeviceName = std::move(deviceName);
        _device.mDeviceOutputSampleRate = deviceOutputSampleRate;
        _device.mDeviceOutputChannels = deviceOutputChannels;
        _device.mDeviceOutputFormat = deviceOutputFormat;
    }

    bool Mixer::IsInitialized() const
    {
        return _initialized;
    }

    AmUInt64 Mixer::Mix(AmVoidPtr mixBuffer, AmUInt64 frameCount)
    {
        if (!_initialized || amEngine->GetState() == nullptr || amEngine->GetState()->stopping || amEngine->GetState()->paused)
            return 0;

        LockAudioMutex();

        const auto numChannels = static_cast<AmUInt16>(_device.mRequestedOutputChannels);
#if defined(AM_SIMD_INTRINSICS)
        const auto lower = simdpp::make_float<AmAudioFrame>(AM_AUDIO_SAMPLE_MIN),
                   upper = simdpp::make_float<AmAudioFrame>(AM_AUDIO_SAMPLE_MAX);
#else
        const AmAudioFrame lower = AM_AUDIO_SAMPLE_MIN, upper = AM_AUDIO_SAMPLE_MAX;
#endif // AM_SIMD_INTRINSICS

        auto buffer = static_cast<AmAudioSampleBuffer>(mixBuffer);
        std::memset(buffer, 0, frameCount * numChannels * sizeof(AmAudioSample));

        // output remaining frames in buffer before mixing new ones
        const AmUInt64 frames = frameCount;

        // dynamically sized buffer
        SoundChunk* align = SoundChunk::CreateChunk(frames, numChannels, MemoryPoolKind::Amplimix);

#if defined(AM_SIMD_INTRINSICS)
        // aSize in Vc::int16_v and multiple of kProcessedFramesCountHalf
        const AmUInt64 aSize = frames / align->samplesPerVector;

        // determine remaining number of frames
        _remainingFrames = aSize * align->samplesPerVector - frames;
#else
        // aSize in AmInt16
        const AmUInt64 aSize = frames * numChannels;

        _remainingFrames = 0; // Should not have remaining frames without SIMD optimization
#endif // AM_SIMD_INTRINSICS

        // begin actual mixing
        bool hasMixedAtLeastOneLayer = false;
        for (auto&& layer : _layers)
        {
            if (ShouldMix(&layer))
            {
                UpdatePitch(&layer);

                hasMixedAtLeastOneLayer = true;
                MixLayer(&layer, align->buffer, aSize, frames);

                // If we have mixed more frames than required, move back the cursor
                if (_remainingFrames)
                {
                    AmUInt64 cursor = AMPLIMIX_LOAD(&layer.cursor);
                    cursor -= _remainingFrames;
                    AMPLIMIX_STORE(&layer.cursor, cursor);
                }
            }
        }

        if (!hasMixedAtLeastOneLayer)
            goto Cleanup;

        // perform clipping using min and max
        for (AmUInt32 i = 0; i < aSize; i++)
        {
#if defined(AM_SIMD_INTRINSICS)
            align->buffer[i] = simdpp::min(simdpp::max(align->buffer[i], lower), upper);
#else
            align->buffer[i] = std::min(std::max(align->buffer[i], lower), upper);
#endif // AM_SIMD_INTRINSICS
        }

        // copy frames, leaving possible remainder
        std::memcpy(buffer, reinterpret_cast<AmAudioSampleBuffer>(align->buffer), frames * numChannels * sizeof(AmAudioSample));

    Cleanup:
        SoundChunk::DestroyChunk(align);

        UnlockAudioMutex();

        ExecuteCommands();

        return frameCount;
    }

    AmUInt32 Mixer::Play(
        SoundData* sound, PlayStateFlag flag, AmReal32 gain, AmReal32 pan, AmReal32 pitch, AmReal32 speed, AmUInt32 id, AmUInt32 layer)
    {
        return PlayAdvanced(sound, flag, gain, pan, pitch, speed, 0, sound->length, id, layer);
    }

    AmUInt32 Mixer::PlayAdvanced(
        SoundData* sound,
        PlayStateFlag flag,
        AmReal32 gain,
        AmReal32 pan,
        AmReal32 pitch,
        AmReal32 speed,
        AmUInt64 startFrame,
        AmUInt64 endFrame,
        AmUInt32 id,
        AmUInt32 layer)
    {
        if (flag <= PLAY_STATE_FLAG_MIN || flag >= PLAY_STATE_FLAG_MAX)
            return 0; // invalid flag

        if (endFrame - startFrame < kProcessedFramesCount || endFrame < kProcessedFramesCount)
            return 0; // invalid frame range

        // define a layer id
        layer = layer == 0 ? ++_nextId : layer;

        // skip 0 as it is special
        if (id == 0)
            id = kAmplimixLayersCount;

        LockAudioMutex();

        // get layer for next sound handle id
        auto* lay = GetLayer(layer);

        // check if corresponding layer is free
        if (AMPLIMIX_LOAD(&lay->flag) == PLAY_STATE_FLAG_MIN)
        {
            // fill in non-atomic layer data along with truncating start and end
            lay->id = id;
            lay->snd = sound;

#if defined(AM_SIMD_INTRINSICS)
            lay->start = startFrame & ~(kProcessedFramesCount - 1);
            lay->end = endFrame & ~(kProcessedFramesCount - 1);
#else
            lay->start = startFrame;
            lay->end = endFrame;
#endif // AM_SIMD_INTRINSICS

            // convert gain and pan to left and right gain and store it atomically
            AMPLIMIX_STORE(&lay->gain, LRGain(gain, pan));
            // store the pitch
            AMPLIMIX_STORE(&lay->pitch, pitch);
            // store the playback speed
            AMPLIMIX_STORE(&lay->userPlaySpeed, speed);
            // atomically set cursor to start position based on given argument
            AMPLIMIX_STORE(&lay->cursor, lay->start);
            // store flag last, releasing the layer to the mixer thread
            AMPLIMIX_STORE(&lay->flag, flag);

            // Initialize the resampler
            ma_data_converter_uninit(&lay->dataConverter, &gAllocationCallbacks);

            const auto soundChannels = static_cast<AmUInt32>(sound->format.GetNumChannels());
            const auto reqChannels = static_cast<AmUInt32>(_device.mRequestedOutputChannels);

            const AmUInt32 soundSampleRate = sound->format.GetSampleRate();
            const AmUInt32 reqSampleRate = _device.mRequestedOutputSampleRate;

            ma_data_converter_config converterConfig = ma_data_converter_config_init_default();

            converterConfig.formatIn = ma_format_f32;
            converterConfig.formatOut = ma_format_from_amplitude(_device.mDeviceOutputFormat);

            converterConfig.channelsIn = soundChannels;
            converterConfig.channelsOut = reqChannels;
            converterConfig.channelMixMode = ma_channel_mix_mode_rectangular;

            converterConfig.sampleRateIn = soundSampleRate;
            converterConfig.sampleRateOut = reqSampleRate;

            converterConfig.resampling.algorithm = ma_resample_algorithm_custom;
            converterConfig.resampling.pBackendUserData = lay;
            converterConfig.resampling.pBackendVTable = &gResamplerVTable;

            converterConfig.allowDynamicSampleRate = true;
            converterConfig.calculateLFEFromSpatialChannels = true;
            converterConfig.ditherMode = ma_dither_mode_rectangle;

            ma_channel_map_init_standard(
                ma_standard_channel_map_default, converterConfig.pChannelMapIn, soundChannels, converterConfig.channelsIn);

            ma_channel_map_init_standard(
                ma_standard_channel_map_default, converterConfig.pChannelMapOut, reqChannels, converterConfig.channelsOut);

            if (ma_data_converter_init(&converterConfig, &gAllocationCallbacks, &lay->dataConverter) != MA_SUCCESS)
            {
                ma_data_converter_uninit(&lay->dataConverter, &gAllocationCallbacks);

                CallLogFunc("[ERROR] Cannot process frames. Unable to initialize the samples data converter.");

                return 0;
            }
        }

        UnlockAudioMutex();
        return layer;
    }

    bool Mixer::SetGainPan(AmUInt32 id, AmUInt32 layer, float gain, float pan)
    {
        auto* lay = GetLayer(layer);

        // check id and state flag to make sure the id is valid
        if ((id == lay->id) && (AMPLIMIX_LOAD(&lay->flag) > PLAY_STATE_FLAG_STOP))
        {
            if (lay->snd != nullptr && lay->snd->format.GetNumChannels() == 1)
                pan = 0.0f;

            // convert gain and pan to left and right gain and store it atomically
            AMPLIMIX_STORE(&lay->gain, LRGain(gain, pan));
            // return success
            return true;
        }

        // return failure
        return false;
    }

    bool Mixer::SetPitch(AmUInt32 id, AmUInt32 layer, AmReal32 pitch)
    {
        auto* lay = GetLayer(layer);

        // check id and state flag to make sure the id is valid
        if ((id == lay->id) && (AMPLIMIX_LOAD(&lay->flag) > PLAY_STATE_FLAG_STOP))
        {
            // store the pitch value atomically
            AMPLIMIX_STORE(&lay->pitch, pitch);
            // return success
            return true;
        }

        // return failure
        return false;
    }

    bool Mixer::SetCursor(AmUInt32 id, AmUInt32 layer, AmUInt64 cursor)
    {
        auto* lay = GetLayer(layer);

        // check id and state flag to make sure the id is valid
        if ((id == lay->id) && (AMPLIMIX_LOAD(&lay->flag) > PLAY_STATE_FLAG_STOP))
        {
#if defined(AM_SIMD_INTRINSICS)
            // clamp cursor and truncate to multiple of 16 before storing
            AMPLIMIX_STORE(&lay->cursor, AM_CLAMP(cursor, lay->start, lay->end) & ~(kProcessedFramesCount - 1));
#else
            // clamp cursor and store it
            AMPLIMIX_STORE(&lay->cursor, AM_CLAMP(cursor, lay->start, lay->end));
#endif // AM_SIMD_INTRINSICS

            // return success
            return true;
        }

        // return failure
        return false;
    }

    bool Mixer::SetPlayState(AmUInt32 id, AmUInt32 layer, PlayStateFlag flag)
    {
        // return failure if given flag invalid
        if (flag >= PLAY_STATE_FLAG_MAX)
            return false;

        LockAudioMutex();

        // get layer based on the lowest bits of id
        auto* lay = GetLayer(layer);

        // check id and state flag to make sure the id is valid
        if (PlayStateFlag prev; (id == lay->id) && ((prev = AMPLIMIX_LOAD(&lay->flag)) >= PLAY_STATE_FLAG_STOP))
        {
            // return success if already in desired state
            if (prev == flag)
            {
                UnlockAudioMutex();
                return true;
            }

            // run appropriate callback
            if (prev == PLAY_STATE_FLAG_STOP && (flag == PLAY_STATE_FLAG_PLAY || flag == PLAY_STATE_FLAG_LOOP))
                OnSoundStarted(this, lay);
            else if ((prev == PLAY_STATE_FLAG_PLAY || prev == PLAY_STATE_FLAG_LOOP) && flag == PLAY_STATE_FLAG_HALT)
                OnSoundPaused(this, lay);
            else if (prev == PLAY_STATE_FLAG_HALT && (flag == PLAY_STATE_FLAG_PLAY || flag == PLAY_STATE_FLAG_LOOP))
                OnSoundResumed(this, lay);
            else if ((prev == PLAY_STATE_FLAG_PLAY || prev == PLAY_STATE_FLAG_LOOP) && flag == PLAY_STATE_FLAG_STOP)
                OnSoundStopped(this, lay);

            // swap if flag has not changed and return if successful
            if (AMPLIMIX_CSWAP(&lay->flag, &prev, flag))
            {
                UnlockAudioMutex();
                return true;
            }
        }

        // return failure
        UnlockAudioMutex();
        return false;
    }

    PlayStateFlag Mixer::GetPlayState(AmUInt32 id, AmUInt32 layer)
    {
        // get layer based on the lowest bits of id
        auto* lay = GetLayer(layer);

        // check id and state flag to make sure the id is valid
        if (PlayStateFlag flag; (id == lay->id) && ((flag = AMPLIMIX_LOAD(&lay->flag)) > PLAY_STATE_FLAG_STOP))
        {
            // return the found flag
            return flag;
        }

        // return failure
        return PLAY_STATE_FLAG_MIN;
    }

    bool Mixer::SetPlaySpeed(AmUInt32 id, AmUInt32 layer, AmReal32 speed)
    {
        auto* lay = GetLayer(layer);

        // check id and state flag to make sure the id is valid
        if ((id == lay->id) && (AMPLIMIX_LOAD(&lay->flag) > PLAY_STATE_FLAG_STOP))
        {
            // convert gain and pan to left and right gain and store it atomically
            AMPLIMIX_STORE(&lay->userPlaySpeed, speed);
            // return success
            return true;
        }

        // return failure
        return false;
    }

    void Mixer::SetMasterGain(float gain)
    {
        AMPLIMIX_STORE(&_masterGain, gain);
    }

    void Mixer::StopAll()
    {
        LockAudioMutex();

        // go through all active layers and set their states to the stop state
        for (auto&& lay : _layers)
        {
            // check if active and set to stop if true
            if (AMPLIMIX_LOAD(&lay.flag) > PLAY_STATE_FLAG_STOP)
                AMPLIMIX_STORE(&lay.flag, PLAY_STATE_FLAG_STOP);
        }

        UnlockAudioMutex();
    }

    void Mixer::HaltAll()
    {
        LockAudioMutex();

        // go through all playing layers and set their states to halt
        for (auto&& lay : _layers)
        {
            // check if playing or looping and try to swap
            if (PlayStateFlag flag; (flag = AMPLIMIX_LOAD(&lay.flag)) > PLAY_STATE_FLAG_HALT)
                AMPLIMIX_CSWAP(&lay.flag, &flag, PLAY_STATE_FLAG_HALT);
        }

        UnlockAudioMutex();
    }

    void Mixer::PlayAll()
    {
        LockAudioMutex();

        // go through all halted layers and set their states to play
        for (auto&& lay : _layers)
        {
            // need to reset each time
            PlayStateFlag flag = PLAY_STATE_FLAG_HALT;
            // swap the flag to play if it is on halt
            AMPLIMIX_CSWAP(&lay.flag, &flag, PLAY_STATE_FLAG_PLAY);
        }

        UnlockAudioMutex();
    }

    bool Mixer::IsInsideThreadMutex() const
    {
        if (_insideAudioThreadMutex.find(Thread::GetCurrentThreadId()) != _insideAudioThreadMutex.end())
            return _insideAudioThreadMutex.at(Thread::GetCurrentThreadId());

        return false;
    }

    void Mixer::PushCommand(const MixerCommand& command)
    {
        _commandsStack.push(command);
    }

    const ProcessorPipeline* Mixer::GetPipeline() const
    {
        return _pipeline;
    }

    ProcessorPipeline* Mixer::GetPipeline()
    {
        return _pipeline;
    }

    void Mixer::IncrementSoundLoopCount(SoundInstance* sound)
    {
        ++sound->_currentLoopCount;
    }

    const DeviceDescription& Mixer::GetDeviceDescription() const
    {
        return _device;
    }

    void Mixer::ExecuteCommands()
    {
        while (!_commandsStack.empty())
        {
            if (const auto& command = _commandsStack.front(); command.callback)
                AM_UNUSED(command.callback());

            _commandsStack.pop();
        }
    }

    void Mixer::MixLayer(MixerLayer* layer, AmAudioFrameBuffer buffer, AmUInt64 bufferSize, AmUInt64 samples)
    {
        if (layer->snd == nullptr)
        {
            AMPLITUDE_ASSERT(false); // This should technically never appear
            return;
        }

        if (_pipeline == nullptr)
        {
            CallLogFunc("[WARNING] No active pipeline is set, this means no sound will be rendered. You should configure the Amplimix "
                        "pipeline in your engine configuration file.\n");
            return;
        }

        const auto reqChannels = static_cast<AmUInt16>(_device.mRequestedOutputChannels);

        // load flag value atomically first
        PlayStateFlag flag = AMPLIMIX_LOAD(&layer->flag);

        // atomically load cursor
        AmUInt64 cursor = AMPLIMIX_LOAD(&layer->cursor);

        // atomically load left and right gain
        const AmVec2 g = AMPLIMIX_LOAD(&layer->gain);
        const float gain = AMPLIMIX_LOAD(&_masterGain);
#if defined(AM_SIMD_INTRINSICS)
        const auto mxGain = simdpp::make_float<AmAudioFrame>(g.X * gain, g.Y * gain);
        const auto& lGain = mxGain;
        const auto& rGain = mxGain;
#else
        const auto lGain = g.X * gain;
        const auto rGain = g.Y * gain;
#endif // AM_SIMD_INTRINSICS

        // loop state
        const bool loop = (flag == PLAY_STATE_FLAG_LOOP);

        const AmUInt16 soundChannels = layer->snd->format.GetNumChannels();
        const AmUInt32 baseSampleRate = layer->snd->format.GetSampleRate();
        const AmReal32 sampleRateRatio = AMPLIMIX_LOAD(&layer->sampleRateRatio);

        AmUInt64 outSamples = samples;
        AmUInt64 inSamples = samples;

        AmUInt64 producedSamples = 0;
        AmUInt64 consumedSamples = 0;

        if (sampleRateRatio != 1.0f)
            ma_data_converter_get_required_input_frame_count(&layer->dataConverter, outSamples, &inSamples);

#if defined(AM_SIMD_INTRINSICS)
        inSamples = AM_VALUE_ALIGN(inSamples, kProcessedFramesCount);
#endif // AM_SIMD_INTRINSICS

        SoundChunk* in = SoundChunk::CreateChunk(inSamples, soundChannels, MemoryPoolKind::Amplimix);
        SoundChunk* out = SoundChunk::CreateChunk(AM_MAX(inSamples, outSamples), reqChannels, MemoryPoolKind::Amplimix);

        while (true)
        {
            const AmUInt64 currentCursor = cursor + consumedSamples;

            // if this sound is streaming, and we have a stream event callback
            if (layer->snd->stream)
            {
                // mix sound per chunk of streamed data
                AmUInt64 c = inSamples;
                while (c > 0 && flag != PLAY_STATE_FLAG_MIN)
                {
                    // update flag value
                    flag = AMPLIMIX_LOAD(&layer->flag);

                    if (flag == PLAY_STATE_FLAG_MIN)
                        break;

                    const AmUInt64 chunkSize = AM_MIN(layer->snd->chunk->frames, c);
                    /* */ AmUInt64 readLen = chunkSize;

#if defined(AM_SIMD_INTRINSICS)
                    readLen = AM_VALUE_ALIGN(readLen, kProcessedFramesCount);
#endif // AM_SIMD_INTRINSICS

                    readLen = OnSoundStream(this, layer, (currentCursor + (inSamples - c)) % layer->snd->length, readLen);
                    readLen = AM_MIN(readLen, chunkSize);

                    // having 0 here mainly means that we have reached
                    // the end of the stream and the audio is not looping.
                    if (readLen == 0)
                        break;

                    memcpy(
                        reinterpret_cast<AmAudioSampleBuffer>(in->buffer) + ((inSamples - c) * soundChannels),
                        reinterpret_cast<AmAudioSampleBuffer>(layer->snd->chunk->buffer), readLen * layer->snd->format.GetFrameSize());

                    c -= readLen;
                }
            }
            else
            {
                // Compute offset
                const AmUInt64 offset = (currentCursor % layer->snd->length) * soundChannels;
                const AmUInt64 remaining = layer->snd->chunk->frames - currentCursor;

                if (currentCursor < layer->snd->chunk->frames && remaining < inSamples)
                {
                    const AmUInt64 size = remaining * layer->snd->format.GetFrameSize();

                    memcpy(
                        reinterpret_cast<AmAudioSampleBuffer>(in->buffer),
                        reinterpret_cast<AmAudioSampleBuffer>(layer->snd->chunk->buffer) + offset, size);

                    memcpy(
                        reinterpret_cast<AmAudioSampleBuffer>(in->buffer) + (remaining * soundChannels),
                        reinterpret_cast<AmAudioSampleBuffer>(layer->snd->chunk->buffer), in->size - size);
                }
                else
                {
                    memcpy(
                        reinterpret_cast<AmAudioSampleBuffer>(in->buffer),
                        reinterpret_cast<AmAudioSampleBuffer>(layer->snd->chunk->buffer) + offset, in->size);
                }
            }

            AmUInt64 currentOutSamples = outSamples - producedSamples;

            if (ma_data_converter_process_pcm_frames(
                    &layer->dataConverter, reinterpret_cast<AmAudioSampleBuffer>(in->buffer), &inSamples,
                    reinterpret_cast<AmAudioSampleBuffer>(out->buffer) + (producedSamples * reqChannels), &currentOutSamples) != MA_SUCCESS)
            {
                SoundChunk::DestroyChunk(out);
                SoundChunk::DestroyChunk(in);

                CallLogFunc("[ERROR] Cannot process frames. Unable to convert the audio input.");

                return;
            }

            producedSamples += currentOutSamples;
            consumedSamples += inSamples;

            if (producedSamples >= outSamples)
                break;
        }

        if (flag >= PLAY_STATE_FLAG_PLAY)
        {
            // Cache cursor
            AmUInt64 oldCursor = cursor;

            const auto sampleRate = static_cast<AmUInt32>(std::ceil(layer->snd->format.GetSampleRate() / sampleRateRatio));

            switch (layer->snd->format.GetInterleaveType())
            {
            case AM_SAMPLE_INTERLEAVED:
                _pipeline->ProcessInterleaved(
                    reinterpret_cast<AmAudioSampleBuffer>(out->buffer), reinterpret_cast<AmAudioSampleBuffer>(out->buffer), samples,
                    out->size, reqChannels, sampleRate, layer->snd->sound.get());
                break;
            case AM_SAMPLE_NON_INTERLEAVED:
                _pipeline->Process(
                    reinterpret_cast<AmAudioSampleBuffer>(out->buffer), reinterpret_cast<AmAudioSampleBuffer>(out->buffer), samples,
                    out->size, reqChannels, sampleRate, layer->snd->sound.get());
                break;
            default:
                CallLogFunc("[WARNING] A bad sound data interleave type was encountered.\n");
                break;
            }

            /* */ AmReal32 position = cursor;
            const AmUInt64 start = layer->start;
            const AmUInt64 end = layer->end;

            const auto step = static_cast<AmReal32>(inSamples) / static_cast<AmReal32>(outSamples);

            // regular playback
            for (AmUInt64 i = 0; i < bufferSize; i += reqChannels)
            {
                position = AM_CLAMP(position, start, end);

                // check if cursor at end
                if (std::ceil(position) == end)
                {
                    // quit unless looping
                    if (!loop)
                        break;

                    // call the onLoop callback
                    if (OnSoundLooped(this, layer))
                    {
                        // wrap around if allowed looping again
                        position = start;
                    }
                    else
                    {
                        ma_data_converter_reset(&layer->dataConverter);

                        // stop playback
                        break;
                    }
                }

                switch (_device.mRequestedOutputChannels)
                {
                case PlaybackOutputChannels::Mono:
                    // lGain is always equal to rGain on mono
                    MixMono(i, lGain, out, buffer);
                    break;

                case PlaybackOutputChannels::Stereo:
                    MixStereo(i, lGain, rGain, out, buffer);
                    break;

                default:
                    CallLogFunc("The mixer cannot handle the requested output channels");
                    break;
                }

                position += step * kProcessedFramesCount;
            }

            cursor += consumedSamples;

#if defined(AM_SIMD_INTRINSICS)
            cursor = AM_VALUE_ALIGN(cursor, kProcessedFramesCount);
#endif // AM_SIMD_INTRINSICS

            cursor = AM_CLAMP(cursor, layer->start, layer->end);

            // swap back cursor if unchanged
            if (!AMPLIMIX_CSWAP(&layer->cursor, &oldCursor, cursor))
                cursor = oldCursor;

            // clear flag if PLAY_STATE_FLAG_PLAY and the cursor has reached the end
            if ((flag == PLAY_STATE_FLAG_PLAY) && (cursor == layer->end))
                AMPLIMIX_CSWAP(&layer->flag, &flag, PLAY_STATE_FLAG_MIN);
        }

        SoundChunk::DestroyChunk(out);
        SoundChunk::DestroyChunk(in);

        // run callback if reached the end
        if (cursor == layer->end)
        {
            // We are in the audio thread mutex here
            const MixerCommandCallback callback = [=]() -> bool
            {
                // stop playback unless looping
                if (!loop)
                {
                    OnSoundEnded(this, layer);
                }
                else
                {
                    // call the onLoop callback
                    if (ShouldLoopSound(this, layer))
                    {
                        // wrap around if allowed looping again
                        AMPLIMIX_CSWAP(&layer->cursor, &layer->end, layer->start);
                    }
                    else
                    {
                        // stop playback
                        OnSoundEnded(this, layer);
                    }
                }

                return true;
            };

            // Postpone call outside the audio thread mutex
            PushCommand({ callback });
        }
    }

    MixerLayer* Mixer::GetLayer(AmUInt32 layer)
    {
        // get layer based on the lowest bits of layer id
        return &_layers[layer & kAmplimixLayersMask];
    }

    bool Mixer::ShouldMix(MixerLayer* layer)
    {
        if (layer->snd == nullptr)
            return false;

        // load flag value
        PlayStateFlag flag = AMPLIMIX_LOAD(&layer->flag);

        // return if flag is not cleared
        return (flag > PLAY_STATE_FLAG_HALT);
    }

    void Mixer::UpdatePitch(MixerLayer* layer)
    {
        const AmReal32 pitch = AMPLIMIX_LOAD(&layer->pitch);
        const AmReal32 speed = AMPLIMIX_LOAD(&layer->userPlaySpeed);

        const AmReal32 oldSpeed = AMPLIMIX_LOAD(&layer->playSpeed);
        const AmReal32 playSpeed = pitch > 0 ? pitch * speed : 0.001f;

        if (playSpeed != oldSpeed)
        {
            const AmReal32 basePitch =
                static_cast<AmReal32>(layer->snd->format.GetSampleRate()) / static_cast<AmReal32>(_device.mRequestedOutputSampleRate);
            const AmReal32 sampleRateRatio = basePitch * playSpeed;

            AMPLIMIX_STORE(&layer->playSpeed, playSpeed);
            AMPLIMIX_STORE(&layer->sampleRateRatio, sampleRateRatio);

            ma_data_converter_set_rate_ratio(&layer->dataConverter, sampleRateRatio);
        }
    }

    void Mixer::LockAudioMutex()
    {
        if (_audioThreadMutex)
        {
            Thread::LockMutex(_audioThreadMutex);
        }

        _insideAudioThreadMutex[Thread::GetCurrentThreadId()] = true;
    }

    void Mixer::UnlockAudioMutex()
    {
        AMPLITUDE_ASSERT(IsInsideThreadMutex());

        if (_audioThreadMutex)
        {
            Thread::UnlockMutex(_audioThreadMutex);
        }

        _insideAudioThreadMutex[Thread::GetCurrentThreadId()] = false;
    }

    void MixerLayer::Reset()
    {
        ma_data_converter_uninit(&dataConverter, &gAllocationCallbacks);
    }
} // namespace SparkyStudios::Audio::Amplitude
