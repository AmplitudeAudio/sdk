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

#include <Core/EngineInternalState.h>

#include "Mixer.h"
#include "RealChannel.h"

#include "engine_config_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    static void atomix_sound_started(atomix_sound* snd)
    {
        auto* sound = static_cast<SoundInstance*>(snd->udata);
        CallLogFunc("Started sound: " AM_OS_CHAR_FMT "\n", sound->GetSound()->GetFilename());
    }

    static void atomix_sound_paused(atomix_sound* snd)
    {
        auto* sound = static_cast<SoundInstance*>(snd->udata);
        CallLogFunc("Paused sound: " AM_OS_CHAR_FMT "\n", sound->GetSound()->GetFilename());
    }

    static void atomix_sound_resumed(atomix_sound* snd)
    {
        auto* sound = static_cast<SoundInstance*>(snd->udata);
        CallLogFunc("Resumed sound: " AM_OS_CHAR_FMT "\n", sound->GetSound()->GetFilename());
    }

    static void atomix_sound_stopped(atomix_sound* snd)
    {
        auto* sound = static_cast<SoundInstance*>(snd->udata);
        CallLogFunc("Stopped sound: " AM_OS_CHAR_FMT "\n", sound->GetSound()->GetFilename());
    }

    static void atomix_sound_ended(atomix_sound* snd)
    {
        const auto* sound = static_cast<SoundInstance*>(snd->udata);
        CallLogFunc("Ended sound: " AM_OS_CHAR_FMT "\n", sound->GetSound()->GetFilename());

        if (const Engine* engine = Engine::GetInstance(); engine->GetState()->stopping)
            goto Delete;

        if (sound->GetSettings().m_kind == SoundKind::Standalone)
            goto Delete;

        RealChannel* channel = sound->GetChannel();
        const Collection* collection = sound->GetCollection();
        AMPLITUDE_ASSERT(collection != nullptr); // Should always have a collection for contained sound instances.
        const CollectionDefinition* config = collection->GetCollectionDefinition();

        if (config->play_mode() == CollectionPlayMode_PlayAll)
        {
            if (channel->Valid())
            {
                channel->MarkAsPlayed(sound->GetSound());
                if (channel->AllSoundsHasPlayed())
                {
                    channel->ClearPlayedSounds();
                    if (config->play_mode() == CollectionPlayMode_PlayAll)
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

    bool Mixer::atomix_sound_looped(atomix_sound* snd)
    {
        auto* sound = static_cast<SoundInstance*>(snd->udata);
        CallLogFunc("Looped sound: " AM_OS_CHAR_FMT "\n", sound->GetSound()->GetFilename());

        ++sound->_currentLoopCount;

        const AmUInt32 loopCount = sound->GetSettings().m_loopCount;
        if (sound->_currentLoopCount == loopCount)
        {
            sound->GetChannel()->Halt();
            return false;
        }

        return true;
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
        atomixSoundSetLoopedCallback(atomix_sound_looped);
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

        m_userData = atomixMixerNew(1.0f, kMinFadeDuration);

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

        if (Engine::GetInstance()->GetState()->stopping)
            return;

        lockAudioMutex();

        atomixMixerMix((atomix_mixer*)m_userData, (AmReal32Buffer)mixBuffer, frameCount);

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
