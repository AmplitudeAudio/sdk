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

#ifndef SPARK_AUDIO_BUS_INTERNAL_STATE_H
#define SPARK_AUDIO_BUS_INTERNAL_STATE_H

#include <vector>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <Core/ChannelInternalState.h>
#include <Utils/intrusive_list.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct BusDefinition;
    struct DuckBusDefinition;

    typedef fplutil::intrusive_list<ChannelInternalState> ChannelList;

    class BusInternalState;
    class DuckBusInternalState;

    class DuckBusInternalState
    {
    public:
        explicit DuckBusInternalState(BusInternalState* parent)
            : _parent(parent)
            , _bus(nullptr)
            , _targetGain(0.0f)
            , _fadeInDuration(0.0)
            , _fadeOutDuration(0.0)
            , _faderInFactory(nullptr)
            , _faderOutFactory(nullptr)
            , _faderIn(nullptr)
            , _faderOut(nullptr)
            , _initialized(false)
            , _transitionPercentage(0.0)
        {}

        ~DuckBusInternalState();

        bool Initialize(const DuckBusDefinition* definition);
        void Update(AmTime deltaTime);

    private:
        BusInternalState* _parent;
        bool _initialized;

        Bus _bus;
        AmReal32 _targetGain;
        AmTime _fadeInDuration;
        AmTime _fadeOutDuration;

        Fader* _faderInFactory;
        Fader* _faderOutFactory;

        FaderInstance* _faderIn;
        FaderInstance* _faderOut;

        AmTime _transitionPercentage;
    };

    class BusInternalState
    {
        friend class DuckBusInternalState;

    public:
        BusInternalState()
            : _busDefinition(nullptr)
            , _id(kAmInvalidObjectId)
            , _name()
            , _userGain(1.0f)
            , _gain(1.0f)
            , _targetUserGain(1.0f)
            , _duckGain(1.0f)
            , _playingSoundList(&ChannelInternalState::bus_node)
            , _muted(false)
            , _gainFaderFactory(nullptr)
            , _gainFader(nullptr)
        {}

        ~BusInternalState();

        void Initialize(const BusDefinition* bus_def);

        // Return the bus definition.
        [[nodiscard]] AM_INLINE(const BusDefinition*) GetBusDefinition() const
        {
            return _busDefinition;
        }

        [[nodiscard]] AM_INLINE(AmBusID) GetId() const
        {
            return _id;
        }

        [[nodiscard]] AM_INLINE(const std::string&) GetName() const
        {
            return _name;
        }

        // Return the final gain after all modifiers have been applied (parent gain,
        // duck gain, bus gain, user gain).
        [[nodiscard]] AM_INLINE(float) GetGain() const
        {
            return _muted ? 0.0f : _gain;
        }

        // Set the user gain.
        void SetUserGain(const float user_gain)
        {
            _userGain = user_gain;
            _targetUserGain = user_gain;
            _gainFader->SetState(AM_FADER_STATE_STOPPED);
        }

        // Return the user gain.
        [[nodiscard]] AM_INLINE(float) GetUserGain() const
        {
            return _userGain;
        }

        void SetMute(bool mute);

        /**
         * @brief Returns whether this bus is muted.
         *
         * @return true if this Bus is muted, false otherwise.
         */
        [[nodiscard]] bool IsMute() const;

        // Fade to the given gain over duration seconds.
        void FadeTo(float gain, AmTime duration);

        // Resets the duck gain to 1.0f. Duck gain must be reset each frame before
        // modifying it.
        AM_INLINE(void) ResetDuckGain()
        {
            _duckGain = 1.0f;
        }

        // Return the vector of child buses.
        AM_INLINE(std::vector<BusInternalState*>&) GetChildBuses()
        {
            return _childBuses;
        }

        // Return the vector of duck buses, the buses to be ducked when a sound is
        // playing on this bus.
        AM_INLINE(std::vector<DuckBusInternalState*>&) GetDuckBuses()
        {
            return _duckBuses;
        }

        // When a sound begins playing or finishes playing, the sound counter should
        // be incremented or decremented appropriately to track whether to
        // apply the duck gain.
        AM_INLINE(ChannelList&) GetPlayingSoundList()
        {
            return _playingSoundList;
        }

        [[nodiscard]] AM_INLINE(const ChannelList&) GetPlayingSoundList() const
        {
            return _playingSoundList;
        }

        // Apply appropriate duck gain to all ducked buses.
        void UpdateDuckGain(AmTime delta_time);

        // Recursively update the final gain of the bus.
        void AdvanceFrame(AmTime delta_time, float parent_gain);

    private:
        const BusDefinition* _busDefinition;

        // The bus unique ID.
        AmBusID _id;

        // The name of the bus.
        std::string _name;

        // Children of a given bus have their gain multiplied against their parent's
        // gain.
        std::vector<BusInternalState*> _childBuses;

        // When a sound is played on this bus, sounds played on these buses should be
        // ducked.
        std::vector<DuckBusInternalState*> _duckBuses;

        // The current user gain of this bus.
        float _userGain;

        // The target user gain of this bus (used for fading).
        float _targetUserGain;

        Fader* _gainFaderFactory;

        // The bus gain fader.
        FaderInstance* _gainFader;

        // The current _duckGain of this bus to be applied to all buses in
        // _duckBuses.
        float _duckGain;

        // The final gain to be applied to all sounds on this bus.
        float _gain;

        // The muted state of the bus.
        bool _muted;

        // Keeps track of how many sounds are being played on this bus.
        ChannelList _playingSoundList;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_BUS_INTERNAL_STATE_H
