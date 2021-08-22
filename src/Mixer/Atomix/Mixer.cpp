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
#include "atomix.h"

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/Drivers/MiniAudio/miniaudio.h>
#include <Core/EngineInternalState.h>

#include "Mixer.h"
#include "RealChannel.h"

#include "engine_config_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    static void atomix_sound_started(atomix_sound* snd)
    {
        auto* sound = static_cast<SoundInstance*>(snd->udata);
        CallLogFunc("Started sound: %s\n", sound->GetSound()->GetFilename().c_str());
    }

    static void atomix_sound_paused(atomix_sound* snd)
    {
        auto* sound = static_cast<SoundInstance*>(snd->udata);
        CallLogFunc("Paused sound: %s\n", sound->GetSound()->GetFilename().c_str());
    }

    static void atomix_sound_resumed(atomix_sound* snd)
    {
        auto* sound = static_cast<SoundInstance*>(snd->udata);
        CallLogFunc("Resumed sound: %s\n", sound->GetSound()->GetFilename().c_str());
    }

    static void atomix_sound_stopped(atomix_sound* snd)
    {
        auto* sound = static_cast<SoundInstance*>(snd->udata);
        CallLogFunc("Stopped sound: %s\n", sound->GetSound()->GetFilename().c_str());
    }

    static void atomix_sound_ended(atomix_sound* snd)
    {
        auto* sound = static_cast<SoundInstance*>(snd->udata);
        CallLogFunc("Ended sound: %s\n", sound->GetSound()->GetFilename().c_str());
        RealChannel* channel = sound->GetChannel();
        SoundCollection* collection = sound->GetSound()->GetSoundCollection();
        const SoundCollectionDefinition* config = collection->GetSoundCollectionDefinition();

        if (config->play_mode() == PlayMode_LoopAll || config->play_mode() == PlayMode_PlayAll)
        {
            if (channel->Valid())
            {
                channel->MarkAsPlayed(sound->GetSound());
                if (channel->AllSoundsHasPlayed())
                {
                    channel->ClearPlayedSounds();
                    if (config->play_mode() == PlayMode_PlayAll)
                    {
                        goto Stop;
                    }
                }

                // Play the collection again only if the channel is still playing.
                if (channel->Playing())
                {
                    channel->GetParentChannelState()->Play();
                }
            }

            // Delete the current sound instance.
            goto Delete;
        }

    Stop:
        // Stop playing the sound
        channel->GetParentChannelState()->Halt();

    Delete:
        // Delete the sound instance at the end of the playback
        delete sound;
    }

    static AmUInt64 atomix_sound_stream(atomix_sound* snd, AmUInt64 offset, AmUInt64 frames)
    {
        auto* sound = static_cast<SoundInstance*>(snd->udata);
        return sound->GetAudio(offset, frames);
    }

    static void atomix_sound_destroy(atomix_sound* snd)
    {
        auto* sound = static_cast<SoundInstance*>(snd->udata);
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
        atomixSoundSetStartedCallback(atomix_sound_started);
        atomixSoundSetPausedCallback(atomix_sound_paused);
        atomixSoundSetResumedCallback(atomix_sound_resumed);
        atomixSoundSetStoppedCallback(atomix_sound_stopped);
        atomixSoundSetStreamCallback(atomix_sound_stream);
        atomixSoundSetDestroyCallback(atomix_sound_destroy);
        atomixSoundSetEndedCallback(atomix_sound_ended);
    }

    Mixer::~Mixer()
    {
        AMPLITUDE_ASSERT(!_insideAudioThreadMutex);

        if (_initialized)
        {
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

        _initialized = true;

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