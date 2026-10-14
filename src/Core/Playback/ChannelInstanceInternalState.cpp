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

#include <Core/Playback/ChannelInstanceInternalState.h>

#include <Core/Playback/ChannelInternalState.h>

namespace SparkyStudios::Audio::Amplitude
{
    ChannelInstanceInternalState::ChannelInstanceInternalState()
        : _instanceId(kAmInvalidObjectId)
        , _parentChannel(nullptr)
        , _location(kVector3Zero)
        , _previousLocation(kVector3Zero)
        , _orientation(Orientation::Zero())
        , _hasOrientation(false)
        , _obstruction(0.0f)
        , _hasObstruction(false)
        , _occlusion(0.0f)
        , _hasOcclusion(false)
        , _room()
        , _weight(1.0f)
        , _cursor(0)
        , _loopCount(0)
        , _finished(false)
        , _computedGain(1.0f)
        , _pannedGain(kVector2One)
        , _dopplerFactors()
    {}

    ChannelInstanceInternalState::ChannelInstanceInternalState(ChannelInternalState* parentChannel)
        : _instanceId(kAmInvalidObjectId)
        , _parentChannel(parentChannel)
        , _location(kVector3Zero)
        , _previousLocation(kVector3Zero)
        , _orientation(Orientation::Zero())
        , _hasOrientation(false)
        , _obstruction(0.0f)
        , _hasObstruction(false)
        , _occlusion(0.0f)
        , _hasOcclusion(false)
        , _room()
        , _weight(1.0f)
        , _cursor(0)
        , _loopCount(0)
        , _finished(false)
        , _computedGain(1.0f)
        , _pannedGain(kVector2One)
        , _dopplerFactors()
    {}

    void ChannelInstanceInternalState::Reset()
    {
        _instanceId = kAmInvalidObjectId;
        _location = kVector3Zero;
        _previousLocation = kVector3Zero;
        _orientation = Orientation::Zero();
        _hasOrientation = false;
        _obstruction = 0.0f;
        _hasObstruction = false;
        _occlusion = 0.0f;
        _hasOcclusion = false;
        _room.Clear();
        _weight = 1.0f;
        _cursor = 0;
        _loopCount = 0;
        _finished = false;
        _computedGain = 1.0f;
        _pannedGain = kVector2One;
        _dopplerFactors.clear();
        _generation = 0;
    }

    void ChannelInstanceInternalState::Invalidate()
    {
        _instanceId = kAmInvalidObjectId;
        _generation = 0;
    }

    void ChannelInstanceInternalState::SetLocation(const AmVector3& location)
    {
        _previousLocation = _location;
        _location = location;

        if (_parentChannel != nullptr)
            _parentChannel->InvalidateInstanceSnapshot();
    }

    const Orientation& ChannelInstanceInternalState::GetOrientation() const
    {
        if (_hasOrientation || _parentChannel == nullptr)
            return _orientation;

        const Entity& entity = _parentChannel->GetEntity();
        return entity.Valid() ? entity.GetOrientation() : _orientation;
    }

    void ChannelInstanceInternalState::SetOrientation(const Orientation& orientation)
    {
        _orientation = orientation;
        _hasOrientation = true;

        if (_parentChannel != nullptr)
            _parentChannel->InvalidateInstanceSnapshot();
    }

    void ChannelInstanceInternalState::ResetOrientation()
    {
        _orientation = Orientation::Zero();
        _hasOrientation = false;

        if (_parentChannel != nullptr)
            _parentChannel->InvalidateInstanceSnapshot();
    }

    AmReal32 ChannelInstanceInternalState::GetObstruction() const
    {
        if (_hasObstruction || _parentChannel == nullptr)
            return _obstruction;

        const Entity& entity = _parentChannel->GetEntity();
        return entity.Valid() ? entity.GetObstruction() : _obstruction;
    }

    void ChannelInstanceInternalState::SetObstruction(AmReal32 obstruction)
    {
        _obstruction = obstruction;
        _hasObstruction = true;

        if (_parentChannel != nullptr)
            _parentChannel->InvalidateInstanceSnapshot();
    }

    void ChannelInstanceInternalState::ResetObstruction()
    {
        _obstruction = 0.0f;
        _hasObstruction = false;

        if (_parentChannel != nullptr)
            _parentChannel->InvalidateInstanceSnapshot();
    }

    AmReal32 ChannelInstanceInternalState::GetOcclusion() const
    {
        if (_hasOcclusion || _parentChannel == nullptr)
            return _occlusion;

        const Entity& entity = _parentChannel->GetEntity();
        return entity.Valid() ? entity.GetOcclusion() : _occlusion;
    }

    void ChannelInstanceInternalState::SetOcclusion(AmReal32 occlusion)
    {
        _occlusion = occlusion;
        _hasOcclusion = true;

        if (_parentChannel != nullptr)
            _parentChannel->InvalidateInstanceSnapshot();
    }

    void ChannelInstanceInternalState::ResetOcclusion()
    {
        _occlusion = 0.0f;
        _hasOcclusion = false;

        if (_parentChannel != nullptr)
            _parentChannel->InvalidateInstanceSnapshot();
    }

    void ChannelInstanceInternalState::SetRoom(const Room& room)
    {
        _room = room;

        if (_parentChannel != nullptr)
            _parentChannel->InvalidateInstanceSnapshot();
    }

    void ChannelInstanceInternalState::SetWeight(AmReal32 weight)
    {
        _weight = AM_MAX(weight, 0.0f);

        if (_parentChannel != nullptr)
            _parentChannel->InvalidateInstanceSnapshot();
    }

    void ChannelInstanceInternalState::AdvanceCursor(AmUInt64 frames, AmUInt64 soundLength, bool loop)
    {
        if (_finished)
            return;

        _cursor += frames;

        if (_cursor >= soundLength)
        {
            if (loop)
            {
                _cursor = _cursor % soundLength;
                _loopCount++;
            }
            else
            {
                _cursor = soundLength;
                _finished = true;
            }
        }
    }

    AmReal32 ChannelInstanceInternalState::GetDopplerFactor(AmListenerID listener) const
    {
        if (const auto it = _dopplerFactors.find(listener); it != _dopplerFactors.end())
            return it->second;

        return 1.0f;
    }

    void ChannelInstanceInternalState::SetDopplerFactor(AmListenerID listener, AmReal32 factor)
    {
        _dopplerFactors[listener] = factor;
    }
} // namespace SparkyStudios::Audio::Amplitude
