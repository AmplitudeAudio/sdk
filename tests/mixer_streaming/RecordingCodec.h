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

#ifndef _AM_TESTS_MIXER_STREAMING_RECORDING_CODEC_H
#define _AM_TESTS_MIXER_STREAMING_RECORDING_CODEC_H

#include <atomic>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/Engine.h>

#include "LightEngineTestCase.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    /// What one decoder created while tracking was on went through.
    struct RecordedDecoder
    {
        std::atomic<AmInt32> opened{ 0 };
        std::atomic<AmInt32> closed{ 0 };
        std::atomic<AmUInt64> streamCalls{ 0 };
        std::atomic<AmUInt64> completedStreamCalls{ 0 };
    };

    /// Process-wide record of the decoders the RecordingCodec created. Test fixtures reset it.
    class RecordingState
    {
    public:
        static RecordingState& Get()
        {
            static RecordingState state;
            return state;
        }

        /// Forgets everything and stops tracking.
        void Reset()
        {
            std::lock_guard lock(_mutex);
            _tracking = false;
            _streamThreads.clear();
            _seekThreads.clear();
            _decoders.clear();
            _delayMs.store(0);
        }

        /// From now on, decoders are recorded and the threads calling them noted.
        void StartTracking()
        {
            std::lock_guard lock(_mutex);
            _tracking = true;
        }

        void SetDelay(AmInt32 milliseconds)
        {
            _delayMs.store(milliseconds);
        }

        [[nodiscard]] AmInt32 GetDelay() const
        {
            return _delayMs.load();
        }

        std::shared_ptr<RecordedDecoder> NewDecoder()
        {
            std::lock_guard lock(_mutex);
            if (!_tracking)
                return nullptr;

            auto record = std::make_shared<RecordedDecoder>();
            _decoders.push_back(record);
            return record;
        }

        void NoteStream()
        {
            std::lock_guard lock(_mutex);
            if (_tracking)
                _streamThreads.insert(Thread::GetCurrentThreadId());
        }

        void NoteSeek()
        {
            std::lock_guard lock(_mutex);
            if (_tracking)
                _seekThreads.insert(Thread::GetCurrentThreadId());
        }

        [[nodiscard]] std::set<AmUInt64> GetStreamThreads() const
        {
            std::lock_guard lock(_mutex);
            return _streamThreads;
        }

        [[nodiscard]] std::set<AmUInt64> GetSeekThreads() const
        {
            std::lock_guard lock(_mutex);
            return _seekThreads;
        }

        [[nodiscard]] std::vector<std::shared_ptr<RecordedDecoder>> GetDecoders() const
        {
            std::lock_guard lock(_mutex);
            return _decoders;
        }

        /// One line per tracked decoder: opened, closed, Stream calls.
        [[nodiscard]] std::string Describe() const
        {
            std::string text;
            for (const auto& decoder : GetDecoders())
            {
                text += "[opened " + std::to_string(decoder->opened.load()) + ", closed " + std::to_string(decoder->closed.load()) +
                    ", streams " + std::to_string(decoder->streamCalls.load()) + "] ";
            }

            return text;
        }

        /// Whether every tracked decoder was closed (at least once).
        [[nodiscard]] bool AllClosed() const
        {
            for (const auto& decoder : GetDecoders())
                if (decoder->closed.load() < 1)
                    return false;

            return true;
        }

        /// Whether every tracked decoder was closed exactly once.
        [[nodiscard]] bool AllClosedOnce() const
        {
            for (const auto& decoder : GetDecoders())
                if (decoder->closed.load() != 1)
                    return false;

            return true;
        }

        /// The number of Stream calls that returned, over every tracked decoder.
        [[nodiscard]] AmUInt64 GetCompletedStreamCalls() const
        {
            AmUInt64 total = 0;
            for (const auto& decoder : GetDecoders())
                total += decoder->completedStreamCalls.load();

            return total;
        }

    private:
        mutable std::mutex _mutex;
        bool _tracking = false;
        std::set<AmUInt64> _streamThreads;
        std::set<AmUInt64> _seekThreads;
        std::vector<std::shared_ptr<RecordedDecoder>> _decoders;
        std::atomic<AmInt32> _delayMs{ 0 };
    };

    /**
     * @brief A codec that forwards to the WAV codec and records the decoders it hands out, the calls made on them and
     * the threads that made the calls. The name sorts before the real codecs, so it is found first.
     */
    class RecordingCodec final : public Codec
    {
    public:
        class RecordingDecoder final : public Decoder
        {
        public:
            RecordingDecoder(const Codec* codec, std::shared_ptr<Decoder> inner, std::shared_ptr<RecordedDecoder> record)
                : Decoder(codec)
                , _inner(std::move(inner))
                , _record(std::move(record))
            {}

            bool Open(std::shared_ptr<File> file) override
            {
                const bool opened = _inner->Open(std::move(file));
                m_format = _inner->GetFormat();
                if (_record != nullptr)
                    _record->opened.fetch_add(1);
                return opened;
            }

            bool Close() override
            {
                if (_record != nullptr)
                    _record->closed.fetch_add(1);
                return _inner->Close();
            }

            AmUInt64 Load(AudioBuffer* out) override
            {
                return _inner->Load(out);
            }

            AmUInt64 Stream(AudioBuffer* out, AmUInt64 bufferOffset, AmUInt64 seekOffset, AmUInt64 length) override
            {
                if (_record != nullptr)
                {
                    RecordingState::Get().NoteStream();
                    _record->streamCalls.fetch_add(1);
                }

                if (const AmInt32 delay = RecordingState::Get().GetDelay(); delay > 0 && _record != nullptr)
                    Thread::Sleep(delay);

                const AmUInt64 frames = _inner->Stream(out, bufferOffset, seekOffset, length);

                if (_record != nullptr)
                    _record->completedStreamCalls.fetch_add(1);

                return frames;
            }

            bool Seek(AmUInt64 offset) override
            {
                if (_record != nullptr)
                    RecordingState::Get().NoteSeek();
                return _inner->Seek(offset);
            }

        private:
            std::shared_ptr<Decoder> _inner;
            std::shared_ptr<RecordedDecoder> _record;
        };

        RecordingCodec()
            : Codec("aaa_recording")
        {}

        [[nodiscard]] std::shared_ptr<Decoder> CreateDecoder() override
        {
            return std::make_shared<RecordingDecoder>(
                this, Codec::Find("wav")->CreateDecoder(), RecordingState::Get().NewDecoder());
        }

        [[nodiscard]] std::shared_ptr<Encoder> CreateEncoder() override
        {
            return Codec::Find("wav")->CreateEncoder();
        }

        [[nodiscard]] bool CanHandleFile(std::shared_ptr<File> file) const override
        {
            return Codec::Find("wav")->CanHandleFile(std::move(file));
        }
    };

    /// The audio thread's id, noted by the after-mix callback.
    inline std::atomic<AmUInt64>& AudioThreadId()
    {
        static std::atomic<AmUInt64> id{ 0 };
        return id;
    }

    /// Peak sample of the mixes the after-mix callback saw, and how many mixes had audio.
    inline std::atomic<AmUInt32>& MixedBlocks()
    {
        static std::atomic<AmUInt32> blocks{ 0 };
        return blocks;
    }

    inline std::atomic<AmUInt32>& AudibleBlocks()
    {
        static std::atomic<AmUInt32> blocks{ 0 };
        return blocks;
    }

    inline void NoteMix(const Amplimix*, const AudioBuffer* buffer, AmUInt32 frames)
    {
        AudioThreadId().store(Thread::GetCurrentThreadId());
        MixedBlocks().fetch_add(1);

        if (buffer == nullptr)
            return;

        for (AmSize c = 0; c < buffer->GetChannelCount(); ++c)
        {
            const auto& channel = buffer->GetChannel(c);
            for (AmUInt32 i = 0; i < frames && i < buffer->GetFrameCount(); ++i)
            {
                if (channel[i] != 0.0f)
                {
                    AudibleBlocks().fetch_add(1);
                    return;
                }
            }
        }
    }

    /**
     * @brief Boots the engine on the null driver with the RecordingCodec registered, then lets tests drive frames.
     *
     * The engine thread of EngineTestCase is not used: tests call @c Frame() themselves.
     */
    class StreamingEngineTestCase : public LightEngineTestCase
    {
    public:
        void SetUp() override
        {
            LightEngineTestCase::SetUp();

            RecordingState::Get().Reset();
            AudioThreadId().store(0);
            MixedBlocks().store(0);
            AudibleBlocks().store(0);

            _codecPlugin = Engine::RegisterExtension<RecordingCodec>();

            InitializeConfig(AM_OS_STRING("tests.config.amconfig"));
            LoadSoundBank(AM_OS_STRING("tests.init.ambank"));

            amEngine->StartLoadSoundFiles();
            while (!amEngine->TryFinalizeLoadSoundFiles())
                Thread::Sleep(1);

            AM_UNUSED(amEngine->AddListener(1));
            amEngine->SetDefaultListener(1);

            amEngine->GetMixer()->SetAfterMixCallback(&NoteMix);

            // Decoders opened for playback from here on are recorded.
            RecordingState::Get().StartTracking();
        }

        void TearDown() override
        {
            if (amEngine->IsInitialized())
            {
                amEngine->Deinitialize();

                amEngine->StartCloseFileSystem();
                while (!amEngine->TryFinalizeCloseFileSystem())
                    Thread::Sleep(1);
            }

            Engine::UnregisterDefaultExtensions();
            Engine::UnregisterExtension(_codecPlugin);
            amEngine->DestroyInstance();
            _fileSystem.reset();
            MemoryManager::Deinitialize();

            RecordingState::Get().Reset();
        }

    protected:
        /// Runs one game frame and gives the audio and streaming threads a moment.
        static void Frame()
        {
            amEngine->AdvanceFrame(kAmSecond / 60.0);
            Thread::Sleep(16);
        }

        /// Runs frames until @p condition holds or @p maxFrames passed.
        template<typename Predicate>
        static bool FramesUntil(Predicate&& condition, AmUInt32 maxFrames = 600)
        {
            for (AmUInt32 i = 0; i < maxFrames; ++i)
            {
                if (condition())
                    return true;

                Frame();
            }

            return condition();
        }

        static AmUInt64 GetUnderruns()
        {
            return static_cast<EngineImpl*>(amEngine)->GetStreamUnderrunCount();
        }

        std::shared_ptr<RecordingCodec> _codecPlugin;
    };

    /// Counts the channel events it is registered for.
    struct EventCount
    {
        std::atomic<AmInt32> count{ 0 };
    };

    inline void CountEvent(ChannelEventInfo info)
    {
        static_cast<EventCount*>(info.m_userData)->count.fetch_add(1);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests

#endif // _AM_TESTS_MIXER_STREAMING_RECORDING_CODEC_H
