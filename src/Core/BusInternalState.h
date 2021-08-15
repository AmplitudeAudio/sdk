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

    typedef fplutil::intrusive_list<ChannelInternalState> ChannelList;

    class BusInternalState
    {
    public:
        BusInternalState()
            : _busDefinition(nullptr)
            , _userGain(1.0f)
            , _gain(1.0f)
            , _targetUserGain(1.0f)
            , _targetUserGainStep(0.0f)
            , _duckGain(1.0f)
            , _playingSoundList(&ChannelInternalState::bus_node)
            , _transitionPercentage(0.0f)
        {}

        void Initialize(const BusDefinition* bus_def);

        // Return the GetBus definition.
        [[nodiscard]] const BusDefinition* GetBusDefinition() const
        {
            return _busDefinition;
        }

        // Return the final gain after all modifiers have been applied (parent gain,
        // duck gain, GetBus gain, user gain).
        [[nodiscard]] float GetGain() const
        {
            return _gain;
        }

        // Set the user gain.
        void SetUserGain(const float user_gain)
        {
            _userGain = user_gain;
            _targetUserGain = user_gain;
            _targetUserGainStep = 0.0f;
        }

        // Return the user gain.
        [[nodiscard]] float GetUserGain() const
        {
            return _userGain;
        }

        // Fade to the given gain over duration seconds.
        void FadeTo(float gain, AmTime duration);

        // Resets the duck gain to 1.0f. Duck gain must be reset each frame before
        // modifying it.
        void ResetDuckGain()
        {
            _duckGain = 1.0f;
        }

        // Return the vector of child buses.
        std::vector<BusInternalState*>& GetChildBuses()
        {
            return _childBuses;
        }

        // Return the vector of duck buses, the buses to be ducked when a sound is
        // playing on this GetBus.
        std::vector<BusInternalState*>& GetDuckBuses()
        {
            return _duckBuses;
        }

        // When a sound begins playing or finishes playing, the sound counter should
        // be incremented or decremented appropriately to track whether to
        // apply the duck gain.
        ChannelList& GetPlayingSoundList()
        {
            return _playingSoundList;
        }

        [[nodiscard]] const ChannelList& GetPlayingSoundList() const
        {
            return _playingSoundList;
        }

        // Apply appropriate duck gain to all ducked buses.
        void UpdateDuckGain(AmTime delta_time);

        // Recursively update the final gain of the GetBus.
        void AdvanceFrame(AmTime delta_time, float parent_gain);

    private:
        const BusDefinition* _busDefinition;

        // Children of a given GetBus have their GetGain multiplied against their parent's
        // GetGain.
        std::vector<BusInternalState*> _childBuses;

        // When a sound is played on this GetBus, sounds played on these buses should be
        // ducked.
        std::vector<BusInternalState*> _duckBuses;

        // The current user GetGain of this GetBus.
        float _userGain;

        // The target user GetGain of this GetBus (used for fading).
        float _targetUserGain;

        // How much to adjust the GetGain per second while fading.
        float _targetUserGainStep;

        // The current _duckGain of this GetBus to be applied to all buses in
        // _duckBuses.
        float _duckGain;

        // The final GetGain to be applied to all sounds on this GetBus.
        float _gain;

        // Keeps track of how many sounds are being played on this GetBus.
        ChannelList _playingSoundList;

        // If a sound is playing on this GetBus, all _duckBuses should lower in volume
        // over time. This tracks how far we are into that transition.
        float _transitionPercentage;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_BUS_INTERNAL_STATE_H
