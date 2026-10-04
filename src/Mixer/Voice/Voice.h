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

#ifndef _AM_IMPLEMENTATION_MIXER_VOICE_VOICE_H
#define _AM_IMPLEMENTATION_MIXER_VOICE_VOICE_H

#include <array>
#include <atomic>
#include <memory>

#include <SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Fader.h>

#include <Mixer/Voice/ResampleStream.h>
#include <Mixer/Voice/SourceReader.h>
#include <Mixer/Voice/TransportEnvelope.h>
#include <Mixer/Voice/VoiceTypes.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief One source position read and resampled: a reader and its stream.
     */
    struct VoiceStreamSlot
    {
        SourceReader reader;
        ResampleStream stream;
    };

    /**
     * @brief Everything a voice needs, fixed at creation on the game thread.
     */
    struct VoiceSettings
    {
        VoiceSource source;
        AmUInt64 regionStart = 0;
        AmUInt64 regionEnd = 0;           ///< 0 means the source length.
        bool loop = false;
        AmUInt32 loopCount = 0;           ///< Total plays when looping; 0 loops forever.
        AmUInt64 startPosition = 0;       ///< First source frame heard.
        AmUInt64 startFrame = kVoiceAsap; ///< Audio-clock frame of the first sample.
        AmUInt64 startPositionClock = kVoiceAsap; ///< Audio-clock frame at which startPosition would have been heard;
                                                   ///< the voice advances it to its real start frame.
        AmTime fadeIn = 0.0;              ///< Fade-in at start in milliseconds; 0 starts at unity.
        AmUInt32 outputRate = 48000;
        AmUInt64 maxBlockFrames = 1024;   ///< Largest block BeginBlock() may be given.
        AmString resamplerName = "default";
        std::shared_ptr<FaderInstance> curve; ///< Transport fade curve; null is linear.
        AmReal64 speed = 1.0;             ///< Initial pitch times playback speed.
        AmUInt32 layer = 0;               ///< Stamped on events.
        AmUInt32 id = 0;                  ///< Stamped on events.
    };

    /**
     * @brief Renders one sound on the audio thread: sample-accurate transport, exact blocks, audio-rate fades.
     *
     * @c Initialize() and @c InitializeSlot() run on the game thread and allocate. Everything else runs on the audio thread
     * and never allocates, locks or logs. The published getters are safe from any thread.
     */
    class Voice
    {
    public:
        static constexpr AmSize kMaxPendingCommands = 32;
        static constexpr AmSize kMaxSegments = 2 * kMaxPendingCommands + 8;

        bool Initialize(const VoiceSettings& settings);
        bool InitializeSlot(VoiceStreamSlot& slot, AmUInt64 position) const;

        /**
         * @brief Queues a command, ordered by frame. Commands are never dropped: when full, the latest-stamped pending
         * command is evicted to make room and @c GetEvictedCommandCount() counts it.
         */
        void Enqueue(const VoiceCommand& command);

        void SetSpeed(AmReal64 speed);

        /**
         * @brief Applies due commands and transitions, splits the block into segments and computes its gain curve.
         */
        void BeginBlock(AmUInt64 blockClock, AmUInt64 frames);

        /**
         * @brief Renders the primary stream into channel 0 of @p mono, handling seeks and the source end.
         */
        void RenderPrimary(AudioBuffer& mono);

        /**
         * @brief Renders an extra stream (separate-mode instance) with this block's segments. Does not change state.
         */
        ResampleStream::PullReport Render(VoiceStreamSlot& slot, AudioBuffer& mono);

        /**
         * @brief Multiplies every channel of @p output by this block's gain curve. May be called once per pipeline output.
         */
        void ApplyGain(AudioBuffer& output) const;

        /**
         * @brief Separate mode: every instance stream finished at @p offset in this block.
         */
        void NotifySourcesFinished(AmUInt64 offset);

        /**
         * @brief Publishes state, position and speed for other threads.
         */
        void EndBlock();

        [[nodiscard]] bool IsAudible() const
        {
            return _audible;
        }

        [[nodiscard]] eVoiceState GetState() const
        {
            return _state;
        }

        [[nodiscard]] AmReal64 GetSpeed() const
        {
            return _speed;
        }

        [[nodiscard]] AmUInt32 GetOutputRate() const
        {
            return _settings.outputRate;
        }

        [[nodiscard]] eVoiceState GetPublishedState() const
        {
            return _publishedState.load(std::memory_order_acquire);
        }

        [[nodiscard]] AmUInt64 GetPublishedPosition() const
        {
            return _publishedPosition.load(std::memory_order_acquire);
        }

        [[nodiscard]] AmUInt32 GetLateCommandCount() const
        {
            return _publishedLate.load(std::memory_order_acquire);
        }

        /**
         * @brief Number of pending commands evicted from a full queue to make room for a newer @c Enqueue() (thread-safe).
         */
        [[nodiscard]] AmUInt32 GetEvictedCommandCount() const
        {
            return _publishedEvicted.load(std::memory_order_acquire);
        }

        VoiceEventMailbox& GetMailbox()
        {
            return _mailbox;
        }

        /**
         * @brief Discards the resampler's group delay on an extra stream (separate-mode instance), so it lines up with the
         * primary path. Audio thread; call once after the stream is positioned.
         */
        void PrimeStream(VoiceStreamSlot& slot);

        VoiceStreamSlot& GetPrimarySlot()
        {
            return _slots[_primary];
        }

    private:
        struct Segment
        {
            AmUInt64 begin;
            AmUInt64 end;
            bool play;
        };

        struct SeekOp
        {
            AmUInt64 offset;
            AmUInt64 position;
        };

        [[nodiscard]] static AmUInt64 SortKey(const VoiceCommand& command);
        [[nodiscard]] AmUInt64 EffectiveFrame(const VoiceCommand& command) const;
        [[nodiscard]] AmUInt64 NextCommandFrame() const;
        [[nodiscard]] AmUInt64 NextTransitionFrame() const;
        [[nodiscard]] bool IsSounding() const;

        void EmitSegment(AmUInt64 begin, AmUInt64 end);
        void HandleTransitions(AmUInt64 frame);
        void ApplyCommandsAt(AmUInt64 frame);
        void Apply(const VoiceCommand& command, AmUInt64 frame);
        void StartPlaying(AmUInt64 frame);
        void BeginFade(AmReal32 target, AmTime duration, AmUInt64 frame, eVoiceFadeTarget fadeTarget);
        void FinishFade(AmUInt64 frame);
        void Finish(AmUInt64 frame);
        void Post(eVoiceEventKind kind, AmUInt64 frame, eVoiceFadeTarget target = eVoiceFadeTarget::None, AmUInt32 count = 1);
        void ArmSeek(AmUInt64 position, AmUInt64 offset);
        ResampleStream::PullReport Prime(VoiceStreamSlot& slot);
        void MergeIncomingReport(const ResampleStream::PullReport& report);
        void RenderPlayRange(AudioBuffer& mono, AmUInt64 begin, AmUInt64 end);
        void HandlePrimaryReport(const ResampleStream::PullReport& report, AmUInt64 offset);

        VoiceSettings _settings;
        std::array<VoiceStreamSlot, 2> _slots;
        AmSize _primary = 0;
        TransportEnvelope _envelope;
        AudioBuffer _gains;
        AudioBuffer _scratch;

        std::array<VoiceCommand, kMaxPendingCommands> _commands{};
        AmSize _commandCount = 0;
        std::array<Segment, kMaxSegments> _segments{};
        AmSize _segmentCount = 0;
        std::array<SeekOp, kMaxPendingCommands> _seeks{};
        AmSize _seekCount = 0;
        VoiceEventMailbox _mailbox;

        eVoiceState _state = eVoiceState::Idle;
        bool _started = false;
        AmUInt64 _startFrame = kVoiceAsap;
        AmUInt64 _blockClock = 0;
        AmUInt64 _blockFrames = 0;
        bool _fading = false;
        AmUInt64 _fadeEnd = 0;
        eVoiceFadeTarget _fadeTarget = eVoiceFadeTarget::None;
        bool _positionPending = false;
        bool _primePending = false;
        bool _audible = false;
        bool _sourceDone = false;
        AmUInt64 _crossfadeFrames = 1;
        AmUInt64 _crossfadeRemaining = 0;
        AmUInt64 _crossfadePosition = 0;
        /// What the incoming stream reported (priming, pulls) during the crossfade: it only counts once it is primary.
        ResampleStream::PullReport _incomingReport;
        AmReal64 _speed = 1.0;
        AmUInt32 _lateCommands = 0;
        AmUInt32 _evictedCommands = 0;

        std::atomic<eVoiceState> _publishedState{ eVoiceState::Idle };
        std::atomic<AmUInt64> _publishedPosition{ 0 };
        std::atomic<AmUInt32> _publishedLate{ 0 };
        std::atomic<AmUInt32> _publishedEvicted{ 0 };
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_VOICE_VOICE_H
