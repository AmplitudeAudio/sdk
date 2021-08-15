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

#include <Core/ChannelInternalState.h>

#ifdef _WIN32
#if !defined(_USE_MATH_DEFINES)
#define _USE_MATH_DEFINES
#endif // !defined(_USE_MATH_DEFINES)
#endif // _WIN32

#include <algorithm>
#include <cmath>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Sound/SoundCollection.h>

#include <Core/BusInternalState.h>
#include <Utils/intrusive_list.h>

#include "Sound.h"

#include "sound_collection_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    bool ChannelInternalState::IsStream() const
    {
        return _collection->GetSoundCollectionDefinition()->stream() != 0;
    }

    // Removes this channel state from all lists.
    void ChannelInternalState::Remove()
    {
        free_node.remove();
        priority_node.remove();
        bus_node.remove();
    }

    void ChannelInternalState::SetSoundCollection(SoundCollection* collection)
    {
        if (_collection && _collection->GetBus())
        {
            bus_node.remove();
        }
        _collection = collection;
        if (_collection && _collection->GetBus())
        {
            _collection->GetBus()->GetPlayingSoundList().push_front(*this);
        }
    }

    bool ChannelInternalState::Play(SoundCollection* collection)
    {
        _collection = collection;
        _sound = collection->Select();
        _channelState = ChannelStatePlaying;
        return !_realChannel.Valid() || _realChannel.Play(_collection, _sound);
    }

    bool ChannelInternalState::Playing() const
    {
        return _channelState == ChannelStatePlaying;
    }

    bool ChannelInternalState::Stopped() const
    {
        return _channelState == ChannelStateStopped;
    }

    bool ChannelInternalState::Paused() const
    {
        return _channelState == ChannelStatePaused;
    }

    void ChannelInternalState::Halt()
    {
        // If this channel loops, we may want to resume it later. If this is a one
        // shot sound that does not loop, just halt it now.
        // TODO(amablue): What we really want is for one shot sounds to change to the
        // stopped state when the sound would have finished. However, SDL mixer does
        // not give good visibility into the length of loaded audio, which makes this
        // difficult. b/20697050
        if (!_collection->GetSoundCollectionDefinition()->loop())
        {
            _channelState = ChannelStateStopped;
        }

        if (_realChannel.Valid())
        {
            _realChannel.Halt();
        }

        _channelState = ChannelStateStopped;
    }

    void ChannelInternalState::Pause()
    {
        if (_realChannel.Valid())
        {
            _realChannel.Pause();
        }

        _channelState = ChannelStatePaused;
    }

    void ChannelInternalState::Resume()
    {
        if (_realChannel.Valid())
        {
            _realChannel.Resume();
        }

        _channelState = ChannelStatePlaying;
    }

    void ChannelInternalState::FadeOut(int milliseconds)
    {
        if (_realChannel.Valid())
        {
            _realChannel.FadeOut(milliseconds);
        }

        _channelState = ChannelStateFadingOut;
    }

    void ChannelInternalState::SetPan(const hmm_vec2& pan)
    {
        if (_realChannel.Valid())
        {
            _realChannel.SetPan(pan);
        }
    }

    void ChannelInternalState::Devirtualize(ChannelInternalState* other)
    {
        AMPLITUDE_ASSERT(!_realChannel.Valid());
        AMPLITUDE_ASSERT(other->_realChannel.Valid());

        // Transfer the real channel id to this channel.
        std::swap(_realChannel, other->_realChannel);

        if (Playing())
        {
            // Resume playing the audio.
            _realChannel.Play(_collection, _sound);
        }
        else if (Paused())
        {
            // The audio needs to be playing to pause it.
            _realChannel.Play(_collection, _sound);
            _realChannel.Pause();
        }
    }

    float ChannelInternalState::Priority() const
    {
        AMPLITUDE_ASSERT(_collection);
        return GetGain() * _collection->GetSoundCollectionDefinition()->priority();
    }

    void ChannelInternalState::UpdateState()
    {
        switch (_channelState)
        {
        case ChannelStatePaused:
        case ChannelStateStopped:
            {
                break;
            }
        case ChannelStatePlaying:
            if (_realChannel.Valid() && !_realChannel.Playing())
            {
                _channelState = ChannelStateStopped;
            }
            break;
        case ChannelStateFadingOut:
            {
                if (!_realChannel.Valid() || !_realChannel.Playing())
                {
                    _channelState = ChannelStateStopped;
                }
                break;
            }
        default:
            {
                AMPLITUDE_ASSERT(false);
            }
        }
    }

} // namespace SparkyStudios::Audio::Amplitude
