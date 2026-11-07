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

#include <Mixer/Voice/VoiceTypes.h>

namespace SparkyStudios::Audio::Amplitude
{
    void VoiceEventMailbox::Post(const VoiceEvent& event)
    {
        const auto index = static_cast<AmSize>(event.kind);
        AMPLITUDE_ASSERT(index < kKinds);

        if (event.kind == eVoiceEventKind::StreamDropped)
        {
            if (_droppedCount < kMaxDroppedStreams)
            {
                _dropped[_droppedCount++] = event;
            }
            else
            {
                _overflow = true;
                _overflowEvent = event;
                _overflowEvent.stream = nullptr;
            }

            return;
        }

        if (_pending[index] && event.kind == eVoiceEventKind::Looped)
        {
            _events[index].count += event.count;
            return;
        }

        _events[index] = event;
        _pending[index] = true;
    }

    void VoiceEventMailbox::Amend(eVoiceEventKind kind, AmUInt64 sourcePosition)
    {
        const auto index = static_cast<AmSize>(kind);
        if (_pending[index])
            _events[index].sourcePosition = sourcePosition;
    }

    void VoiceEventMailbox::Publish(VoiceEventQueue& queue)
    {
        for (AmSize index = 0; index < kKinds; ++index)
        {
            if (index == static_cast<AmSize>(eVoiceEventKind::StreamDropped))
            {
                if (!PublishDropped(queue))
                    return;

                continue;
            }

            if (!_pending[index])
                continue;

            if (!queue.TryEnqueue(_events[index]))
                return;

            _pending[index] = false;
        }
    }

    bool VoiceEventMailbox::PublishDropped(VoiceEventQueue& queue)
    {
        while (_droppedHead < _droppedCount)
        {
            if (!queue.TryEnqueue(_dropped[_droppedHead]))
                return false;

            ++_droppedHead;
        }

        _droppedHead = _droppedCount = 0;

        if (_overflow)
        {
            if (!queue.TryEnqueue(_overflowEvent))
                return false;

            _overflow = false;
        }

        return true;
    }

    bool VoiceEventMailbox::IsEmpty() const
    {
        if (_droppedHead < _droppedCount || _overflow)
            return false;

        for (const bool pending : _pending)
            if (pending)
                return false;

        return true;
    }

    void VoiceEventMailbox::Clear()
    {
        _pending.fill(false);
        _droppedHead = _droppedCount = 0;
        _overflow = false;
    }
} // namespace SparkyStudios::Audio::Amplitude
