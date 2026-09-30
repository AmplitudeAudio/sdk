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

#include <algorithm>
#include <cmath>

#include <Mixer/Voice/VirtualCursor.h>
#include <Mixer/Voice/Voice.h>

namespace SparkyStudios::Audio::Amplitude
{
    namespace
    {
        constexpr AmReal64 kHalfPi = 1.57079632679489661923;
    } // namespace

    bool Voice::Initialize(const VoiceSettings& settings)
    {
        if (!settings.source.IsValid() || settings.outputRate == 0 || settings.maxBlockFrames == 0)
            return false;

        _settings = settings;
        _speed = settings.speed > 0.0 && std::isfinite(settings.speed) ? settings.speed : 1.0;
        _settings.speed = _speed;

        for (auto& slot : _slots)
            if (!InitializeSlot(slot, settings.startPosition))
                return false;

        _primary = 0;
        _envelope.Initialize(settings.curve, settings.outputRate);
        _envelope.SetGain(settings.fadeIn > 0.0 ? 0.0f : 1.0f);
        _gains = AudioBuffer(settings.maxBlockFrames, 1);
        _scratch = AudioBuffer(settings.maxBlockFrames, 1);

        _crossfadeFrames = AM_MAX(MillisecondsToFrames(kSeekCrossfade, settings.outputRate), 1ULL);
        _crossfadeRemaining = 0;
        _crossfadePosition = 0;
        _commandCount = _segmentCount = _seekCount = 0;
        _mailbox.Clear();

        _state = eVoiceState::Scheduled;
        _started = false;
        _startFrame = settings.startFrame;
        _fading = _positionPending = _audible = _sourceDone = false;
        _fadeTarget = eVoiceFadeTarget::None;
        _lateCommands = 0;
        _evictedCommands = 0;

        _publishedState.store(_state, std::memory_order_release);
        _publishedPosition.store(_slots[0].stream.GetSourcePosition(_slots[0].reader), std::memory_order_release);
        _publishedLate.store(0, std::memory_order_release);
        _publishedEvicted.store(0, std::memory_order_release);
        return true;
    }

    bool Voice::InitializeSlot(VoiceStreamSlot& slot, AmUInt64 position) const
    {
        slot.reader.Initialize(_settings.source, _settings.regionStart, _settings.regionEnd, _settings.loop, _settings.loopCount);
        slot.reader.Seek(position);

        if (!slot.stream.Initialize(
                _settings.resamplerName, _settings.source.sampleRate, _settings.outputRate, _settings.source.channels,
                _settings.maxBlockFrames))
            return false;

        slot.stream.SetSpeed(_settings.speed);
        return true;
    }

    AmUInt64 Voice::SortKey(const VoiceCommand& command)
    {
        return command.frame == kVoiceAsap ? 0 : command.frame;
    }

    void Voice::Enqueue(const VoiceCommand& command)
    {
        const AmUInt64 key = SortKey(command);

        // Commands are never dropped: when full, evict the latest-stamped pending command to make room, then insert
        // the new one in sorted position (stable).
        AmSize count = _commandCount;
        if (count == kMaxPendingCommands)
        {
            --count;
            ++_evictedCommands;
        }

        AmSize index = count;
        while (index > 0 && SortKey(_commands[index - 1]) > key)
        {
            _commands[index] = _commands[index - 1];
            --index;
        }

        _commands[index] = command;
        _commandCount = count + 1;
    }

    void Voice::SetSpeed(AmReal64 speed)
    {
        if (!std::isfinite(speed) || speed <= 0.0)
            speed = 1.0;

        if (speed == _speed)
            return;

        _speed = speed;
        for (auto& slot : _slots)
            slot.stream.SetSpeed(speed);
    }

    AmUInt64 Voice::EffectiveFrame(const VoiceCommand& command) const
    {
        return command.frame == kVoiceAsap ? _blockClock : AM_MAX(command.frame, _blockClock);
    }

    AmUInt64 Voice::NextCommandFrame() const
    {
        return _commandCount > 0 ? EffectiveFrame(_commands[0]) : kVoiceAsap;
    }

    AmUInt64 Voice::NextTransitionFrame() const
    {
        AmUInt64 next = kVoiceAsap;

        if (_state == eVoiceState::Scheduled && _startFrame != kVoiceAsap)
            next = AM_MAX(_startFrame, _blockClock);

        if (_fading)
            next = AM_MIN(next, _fadeEnd);

        return next;
    }

    bool Voice::IsSounding() const
    {
        return _state == eVoiceState::Playing || _state == eVoiceState::FadingOut || _state == eVoiceState::Ending;
    }

    void Voice::BeginBlock(AmUInt64 blockClock, AmUInt64 frames)
    {
        AMPLITUDE_ASSERT(frames <= _gains.GetFrameCount());

        _blockClock = blockClock;
        _blockFrames = frames;
        _segmentCount = 0;
        _seekCount = 0;
        _audible = false;

        if (_state == eVoiceState::Scheduled)
        {
            if (_startFrame == kVoiceAsap)
            {
                _startFrame = blockClock;
            }
            else if (_startFrame < blockClock)
            {
                ++_lateCommands;
                _startFrame = blockClock;
            }
        }

        const AmUInt64 blockEnd = blockClock + frames;
        AmUInt64 at = blockClock;

        while (true)
        {
            const AmUInt64 next = AM_MAX(AM_MIN(AM_MIN(NextCommandFrame(), NextTransitionFrame()), blockEnd), at);
            EmitSegment(at - blockClock, next - blockClock);
            at = next;

            if (at >= blockEnd)
                break;

            HandleTransitions(at);
            ApplyCommandsAt(at);
        }
    }

    void Voice::EmitSegment(AmUInt64 begin, AmUInt64 end)
    {
        if (end <= begin)
            return;

        _envelope.Render(_gains[0], begin, end - begin);

        const bool play = IsSounding();
        _audible |= play;

        if (_segmentCount > 0 && _segments[_segmentCount - 1].play == play && _segments[_segmentCount - 1].end == begin)
        {
            _segments[_segmentCount - 1].end = end;
            return;
        }

        if (_segmentCount < kMaxSegments)
            _segments[_segmentCount++] = { begin, end, play };
        else
            _segments[_segmentCount - 1].end = end;
    }

    void Voice::HandleTransitions(AmUInt64 frame)
    {
        if (_state == eVoiceState::Scheduled && _startFrame != kVoiceAsap && _startFrame <= frame)
            StartPlaying(frame);

        if (_fading && _fadeEnd <= frame)
            FinishFade(frame);
    }

    void Voice::ApplyCommandsAt(AmUInt64 frame)
    {
        AmSize applied = 0;
        while (applied < _commandCount && EffectiveFrame(_commands[applied]) <= frame)
        {
            const VoiceCommand& command = _commands[applied];
            if (command.frame != kVoiceAsap && command.frame < _blockClock)
                ++_lateCommands;

            Apply(command, frame);
            ++applied;
        }

        std::move(_commands.begin() + applied, _commands.begin() + _commandCount, _commands.begin());
        _commandCount -= applied;
    }

    void Voice::Apply(const VoiceCommand& command, AmUInt64 frame)
    {
        switch (command.kind)
        {
        case eVoiceCommandKind::Stop:
        case eVoiceCommandKind::Release:
            {
                const bool release = command.kind == eVoiceCommandKind::Release;
                if (_state == eVoiceState::Finished || _state == eVoiceState::Idle)
                    break;

                if (_state == eVoiceState::Scheduled || _state == eVoiceState::Paused)
                {
                    if (release)
                    {
                        Post(eVoiceEventKind::FadedOut, frame, eVoiceFadeTarget::Released);
                        _positionPending = true;
                    }

                    Finish(frame);
                    break;
                }

                const AmTime duration = release && command.duration <= 0.0 ? kStealFade : command.duration;
                BeginFade(0.0f, duration, frame, release ? eVoiceFadeTarget::Released : eVoiceFadeTarget::Stopped);
                _state = eVoiceState::FadingOut;
                break;
            }

        case eVoiceCommandKind::Pause:
            if (_state == eVoiceState::Playing || _state == eVoiceState::Ending)
            {
                BeginFade(0.0f, command.duration, frame, eVoiceFadeTarget::Paused);
                _state = eVoiceState::FadingOut;
            }
            else if (_state == eVoiceState::Scheduled)
            {
                _state = eVoiceState::Paused;
                _envelope.SetGain(0.0f);
                Post(eVoiceEventKind::FadedOut, frame, eVoiceFadeTarget::Paused);
                _positionPending = true;
            }
            break;

        case eVoiceCommandKind::Resume:
            if (_state == eVoiceState::Paused || (_state == eVoiceState::FadingOut && _fadeTarget == eVoiceFadeTarget::Paused))
            {
                if (!_started)
                {
                    _started = true;
                    Post(eVoiceEventKind::Started, frame);
                }

                _state = eVoiceState::Playing;
                BeginFade(1.0f, command.duration, frame, eVoiceFadeTarget::None);
            }
            break;

        case eVoiceCommandKind::Seek:
            if (IsSounding())
            {
                if (_seekCount < _seeks.size())
                    _seeks[_seekCount++] = { frame - _blockClock, command.position };

                if (_state == eVoiceState::Ending)
                    _state = eVoiceState::Playing;
            }
            else if (_state == eVoiceState::Paused || _state == eVoiceState::Scheduled)
            {
                // Cancel any crossfade left over from a seek that armed before the voice stopped sounding: promote
                // the incoming stream first if it had already become dominant, per the same rule RenderPrimary uses.
                if (_crossfadeRemaining > 0)
                {
                    if (_crossfadePosition >= _crossfadeFrames / 2)
                        _primary = 1 - _primary;

                    _crossfadeRemaining = 0;
                }

                VoiceStreamSlot& slot = _slots[_primary];
                slot.reader.Seek(command.position);
                slot.stream.Reset();
            }
            break;
        }
    }

    void Voice::StartPlaying(AmUInt64 frame)
    {
        _state = eVoiceState::Playing;
        _started = true;

        // Resuming a virtual cursor: startPosition is the source frame that was heard at startPositionClock, so
        // advance the primary reader to where the cursor would be by this actual start frame.
        if (_settings.startPositionClock != kVoiceAsap && frame > _settings.startPositionClock)
        {
            VoiceStreamSlot& slot = _slots[_primary];
            const AmReal64 rate = static_cast<AmReal64>(_settings.source.sampleRate) * _speed / static_cast<AmReal64>(_settings.outputRate);

            VirtualCursor cursor;
            cursor.Anchor(
                _settings.startPosition, _settings.startPositionClock, rate, slot.reader.GetRegionStart(), slot.reader.GetRegionEnd(),
                slot.reader.IsLooping());

            slot.reader.Seek(cursor.PositionAt(frame));
            slot.stream.Reset();
        }

        Post(eVoiceEventKind::Started, frame);

        if (_settings.fadeIn > 0.0)
            BeginFade(1.0f, _settings.fadeIn, frame, eVoiceFadeTarget::None);
    }

    void Voice::BeginFade(AmReal32 target, AmTime duration, AmUInt64 frame, eVoiceFadeTarget fadeTarget)
    {
        const AmUInt64 length = _envelope.FadeTo(target, duration);
        _fading = true;
        _fadeEnd = frame + length;
        _fadeTarget = fadeTarget;
    }

    void Voice::FinishFade(AmUInt64 frame)
    {
        _fading = false;
        const eVoiceFadeTarget target = _fadeTarget;
        _fadeTarget = eVoiceFadeTarget::None;

        switch (target)
        {
        case eVoiceFadeTarget::None:
            break;
        case eVoiceFadeTarget::Paused:
            _state = eVoiceState::Paused;
            Post(eVoiceEventKind::FadedOut, frame, eVoiceFadeTarget::Paused);
            _positionPending = true;
            break;
        case eVoiceFadeTarget::Stopped:
            Post(eVoiceEventKind::FadedOut, frame, eVoiceFadeTarget::Stopped);
            Finish(frame);
            break;
        case eVoiceFadeTarget::Released:
            Post(eVoiceEventKind::FadedOut, frame, eVoiceFadeTarget::Released);
            _positionPending = true;
            Finish(frame);
            break;
        }
    }

    void Voice::Finish(AmUInt64 frame)
    {
        _state = eVoiceState::Finished;
        _fading = false;
        Post(eVoiceEventKind::Finished, frame);
    }

    void Voice::Post(eVoiceEventKind kind, AmUInt64 frame, eVoiceFadeTarget target, AmUInt32 count)
    {
        _mailbox.Post(VoiceEvent{ _settings.layer, _settings.id, kind, target, frame, 0, count });
    }

    void Voice::RenderPrimary(AudioBuffer& mono)
    {
        auto& out = mono[0];
        AmSize seek = 0;

        for (AmSize s = 0; s < _segmentCount; ++s)
        {
            const Segment& segment = _segments[s];
            AmUInt64 a = segment.begin;

            while (a < segment.end)
            {
                if (!segment.play || _sourceDone)
                {
                    std::fill(out.begin() + a, out.begin() + segment.end, 0.0f);
                    break;
                }

                AmUInt64 b = segment.end;
                if (seek < _seekCount)
                {
                    if (_seeks[seek].offset <= a)
                    {
                        ArmSeek(_seeks[seek++].position);
                        continue;
                    }

                    b = AM_MIN(b, _seeks[seek].offset);
                }

                RenderPlayRange(mono, a, b);
                a = b;
            }
        }

        // A seek that landed where nothing plays (the source already finished) repositions directly.
        for (; seek < _seekCount; ++seek)
        {
            VoiceStreamSlot& slot = _slots[_primary];
            slot.reader.Seek(_seeks[seek].position);
            slot.stream.Reset();
        }
    }

    void Voice::RenderPlayRange(AudioBuffer& mono, AmUInt64 begin, AmUInt64 end)
    {
        auto& out = mono[0];
        AmUInt64 a = begin;

        if (_crossfadeRemaining > 0)
        {
            const AmUInt64 n = AM_MIN(end - a, _crossfadeRemaining);
            VoiceStreamSlot& from = _slots[_primary];
            VoiceStreamSlot& to = _slots[1 - _primary];

            AM_UNUSED(from.stream.Pull(from.reader, _scratch[0], a, n));
            const auto report = to.stream.Pull(to.reader, out, a, n);

            for (AmUInt64 i = 0; i < n; ++i)
            {
                const AmReal64 t = (static_cast<AmReal64>(_crossfadePosition + i) + 0.5) / static_cast<AmReal64>(_crossfadeFrames);
                const AmReal64 angle = t * kHalfPi;
                out[a + i] = static_cast<AmReal32>(_scratch[0][a + i] * std::cos(angle) + out[a + i] * std::sin(angle));
            }

            _crossfadePosition += n;
            _crossfadeRemaining -= n;
            if (_crossfadeRemaining == 0)
                _primary = 1 - _primary;

            HandlePrimaryReport(report, a);
            a += n;
        }

        if (a < end && !_sourceDone)
        {
            VoiceStreamSlot& slot = _slots[_primary];
            HandlePrimaryReport(slot.stream.Pull(slot.reader, out, a, end - a), a);
        }
        else if (a < end)
        {
            std::fill(out.begin() + a, out.begin() + end, 0.0f);
        }
    }

    void Voice::ArmSeek(AmUInt64 position)
    {
        // A seek during a crossfade promotes the incoming stream to outgoing only once it dominates (at or past the
        // midpoint). Before that the outgoing stream is the louder one: keep it and re-target the incoming slot, which
        // avoids a cut.
        if (_crossfadeRemaining > 0 && _crossfadePosition >= _crossfadeFrames / 2)
            _primary = 1 - _primary;

        VoiceStreamSlot& to = _slots[1 - _primary];
        to.reader = _slots[_primary].reader;
        to.reader.Seek(position);
        to.stream.Reset();
        to.stream.SetSpeed(_speed);

        _crossfadeRemaining = _crossfadeFrames;
        _crossfadePosition = 0;
        _sourceDone = false;
    }

    void Voice::HandlePrimaryReport(const ResampleStream::PullReport& report, AmUInt64 offset)
    {
        const AmUInt64 base = _blockClock + offset;

        if (report.wraps > 0)
            Post(eVoiceEventKind::Looped, base + report.firstWrapFrame, eVoiceFadeTarget::None, report.wraps);

        if (report.ended)
        {
            Post(eVoiceEventKind::Ended, base + report.endFrame);
            if (_state == eVoiceState::Playing)
                _state = eVoiceState::Ending;
        }

        if (report.error)
            Post(eVoiceEventKind::Error, base);

        if ((report.finished || report.error) && !_sourceDone)
        {
            _sourceDone = true;

            if (_state != eVoiceState::Finished)
            {
                // The source ran out while a release fade was still running: a virtual cursor anchors on
                // FadedOut{Released}, so post it (position pending, amended in EndBlock) before Finished.
                if (_state == eVoiceState::FadingOut && _fadeTarget == eVoiceFadeTarget::Released)
                {
                    Post(eVoiceEventKind::FadedOut, base + report.finishedFrame, eVoiceFadeTarget::Released);
                    _positionPending = true;
                }

                Finish(base + report.finishedFrame);
            }
        }
    }

    ResampleStream::PullReport Voice::Render(VoiceStreamSlot& slot, AudioBuffer& mono)
    {
        ResampleStream::PullReport total;
        auto& out = mono[0];

        for (AmSize s = 0; s < _segmentCount; ++s)
        {
            const Segment& segment = _segments[s];
            if (!segment.play || _sourceDone)
            {
                std::fill(out.begin() + segment.begin, out.begin() + segment.end, 0.0f);
                continue;
            }

            const auto report = slot.stream.Pull(slot.reader, out, segment.begin, segment.end - segment.begin);
            total.produced += report.produced;
            total.error |= report.error;
            total.starved |= report.starved;

            if (report.ended && !total.ended)
            {
                total.ended = true;
                total.endFrame = segment.begin + report.endFrame;
            }

            if (report.finished && !total.finished)
            {
                total.finished = true;
                total.finishedFrame = segment.begin + report.finishedFrame;
            }
        }

        return total;
    }

    void Voice::ApplyGain(AudioBuffer& output) const
    {
        const auto& gains = _gains[0];
        for (AmSize c = 0; c < output.GetChannelCount(); ++c)
        {
            auto& channel = output[c];
            for (AmUInt64 i = 0; i < _blockFrames; ++i)
                channel[i] *= gains[i];
        }
    }

    void Voice::NotifySourcesFinished(AmUInt64 offset)
    {
        if (_state == eVoiceState::Finished)
            return;

        _sourceDone = true;
        Post(eVoiceEventKind::Ended, _blockClock + offset);
        Finish(_blockClock + offset);
    }

    void Voice::EndBlock()
    {
        // While a crossfade is active, the incoming stream is the position the voice is transitioning to; report it
        // instead of the outgoing (still primary) stream's.
        const VoiceStreamSlot& slot = _crossfadeRemaining > 0 ? _slots[1 - _primary] : _slots[_primary];
        const AmUInt64 position = slot.stream.GetSourcePosition(slot.reader);

        if (_positionPending)
        {
            _mailbox.Amend(eVoiceEventKind::FadedOut, position);
            _positionPending = false;
        }

        _publishedPosition.store(position, std::memory_order_release);
        _publishedLate.store(_lateCommands, std::memory_order_release);
        _publishedEvicted.store(_evictedCommands, std::memory_order_release);
        _publishedState.store(_state, std::memory_order_release);
    }
} // namespace SparkyStudios::Audio::Amplitude
