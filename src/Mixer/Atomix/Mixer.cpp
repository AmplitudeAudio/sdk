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

#define ATOMIX_IMPLEMENTATION

#include <Core/EngineInternalState.h>

#include <SparkyStudios/Audio/Amplitude/Core/Log.h>
#include <SparkyStudios/Audio/Amplitude/Core/Thread.h>

#include "Mixer.h"
#include "RealChannel.h"
#include "Sound.h"

#include "atomix.h"
#include "audio_config_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    static AmInt32 atomix_sound_stream(atomix_sound* snd, uint32_t offset, uint32_t frames)
    {
        auto* sound = static_cast<Sound*>(snd->udata);
        return sound->GetAudio(offset, frames);
    }

    static void atomix_sound_destroy(atomix_sound* snd)
    {
        auto* sound = static_cast<Sound*>(snd->udata);
        sound->Destroy();
    }

    Mixer::Mixer(EngineInternalState* parent)
        : _engineState(parent)
        , _initialized(false)
        , _audioEngineConfig(nullptr)
        , m_userData(nullptr)
        , _audioThreadMutex(nullptr)
        , _insideAudioThreadMutex(false)
    {
        atomixSoundSetStreamCallback(atomix_sound_stream);
        atomixSoundSetDestroyCallback(atomix_sound_destroy);
    }

    Mixer::~Mixer()
    {
        AMPLITUDE_ASSERT(!_insideAudioThreadMutex);

        if (_initialized)
        {
            miniaudio_uninit(this);

            free(m_userData);
            m_userData = nullptr;
            _initialized = false;

            if (_audioThreadMutex)
                Thread::DestroyMutex(_audioThreadMutex);
            _audioThreadMutex = nullptr;
        }
    }

    bool Mixer::Initialize(const EngineConfigDefinition* config)
    {
        if (_initialized)
        {
            CallLogFunc("Atomix Mixer has already been initialized.\n");
            return false;
        }

        m_userData = atomixMixerNew(1.0f, 0);

        if (!m_userData)
        {
            CallLogFunc("Could not open audio stream\n");
            return false;
        }

        _audioThreadMutex = Thread::CreateMutexAm();

        _audioEngineConfig = config;

        _initialized = miniaudio_init(this, config, ma_format_f32);

        if (!_initialized)
        {
            _audioEngineConfig = nullptr;
            CallLogFunc("Unable to load the audio playback device.");
            return false;
        }

        return _initialized;
    }

    void Mixer::Mix(AmVoidPtr mixBuffer, AmUInt32 frameCount)
    {
        AMPLITUDE_ASSERT(!_insideAudioThreadMutex);

        if (!_initialized)
            return;

        lockAudioMutex();

        atomixMixerMix((atomix_mixer*)m_userData, (AmFloat32Buffer)mixBuffer, frameCount);

        unlockAudioMutex();
    }

    void Mixer::PostInit(AmUInt32 bufferSize, AmUInt32 sampleRate, AmUInt32 channels)
    {}

    void Mixer::lockAudioMutex()
    {
        if (_audioThreadMutex)
        {
            Thread::LockMutex(_audioThreadMutex);
        }

        AMPLITUDE_ASSERT(!_insideAudioThreadMutex);
        _insideAudioThreadMutex = true;
    }

    void Mixer::unlockAudioMutex()
    {
        AMPLITUDE_ASSERT(_insideAudioThreadMutex);
        _insideAudioThreadMutex = false;

        if (_audioThreadMutex)
        {
            Thread::UnlockMutex(_audioThreadMutex);
        }
    }

} // namespace SparkyStudios::Audio::Amplitude