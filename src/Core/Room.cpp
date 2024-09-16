// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#include <SparkyStudios/Audio/Amplitude/Core/Room.h>

#include <Core/EngineInternalState.h>
#include <Core/RoomInternalState.h>

namespace SparkyStudios::Audio::Amplitude
{
    Room::Room()
        : _state(nullptr)
    {}

    Room::Room(RoomInternalState* state)
        : _state(state)
    {}

    void Room::Clear()
    {
        _state = nullptr;
    }

    bool Room::Valid() const
    {
        return _state != nullptr && _state->GetId() != kAmInvalidObjectId && _state->node.in_list();
    }

    AmRoomID Room::GetId() const
    {
        return _state != nullptr ? _state->GetId() : kAmInvalidObjectId;
    }

    void Room::SetLocation(const AmVec3& location) const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->SetLocation(location);
    }

    const AmVec3& Room::GetLocation() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetLocation();
    }

    void Room::SetOrientation(const Orientation& orientation) const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->SetOrientation(orientation);
    }

    const Orientation& Room::GetOrientation() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetOrientation();
    }

    AmVec3 Room::GetDirection() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetDirection();
    }

    AmVec3 Room::GetUp() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetUp();
    }

    void Room::SetDimensions(AmVec3 dimensions) const
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetDimensions(dimensions);
    }

    void Room::SetShape(const BoxShape& shape) const
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetShape(shape);
    }

    const BoxShape& Room::GetShape() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetShape();
    }

    void Room::SetWallMaterial(RoomWall wall, const RoomMaterial& material) const
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetWallMaterial(wall, material);
    }

    void Room::SetAllWallMaterials(const RoomMaterial& material) const
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetWallMaterial(RoomWall::Left, material);
        _state->SetWallMaterial(RoomWall::Right, material);
        _state->SetWallMaterial(RoomWall::Floor, material);
        _state->SetWallMaterial(RoomWall::Ceiling, material);
        _state->SetWallMaterial(RoomWall::Front, material);
        _state->SetWallMaterial(RoomWall::Back, material);
    }

    void Room::SetWallMaterials(
        const RoomMaterial& leftWallMaterial,
        const RoomMaterial& rightWallMaterial,
        const RoomMaterial& floorMaterial,
        const RoomMaterial& ceilingMaterial,
        const RoomMaterial& frontWallMaterial,
        const RoomMaterial& backWallMaterial) const
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetWallMaterial(RoomWall::Left, leftWallMaterial);
        _state->SetWallMaterial(RoomWall::Right, rightWallMaterial);
        _state->SetWallMaterial(RoomWall::Floor, floorMaterial);
        _state->SetWallMaterial(RoomWall::Ceiling, ceilingMaterial);
        _state->SetWallMaterial(RoomWall::Front, frontWallMaterial);
        _state->SetWallMaterial(RoomWall::Back, backWallMaterial);
    }

    const RoomMaterial& Room::GetWallMaterial(RoomWall wall) const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetWallMaterial(wall);
    }

    void Room::SetGain(AmReal32 gain) const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->SetGain(gain);
    }

    AmReal32 Room::GetGain() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetGain();
    }

    AmReal32 Room::GetVolume() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetVolume();
    }

    void Room::Update() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->Update();
    }

    RoomInternalState* Room::GetState() const
    {
        return _state;
    }
} // namespace SparkyStudios::Audio::Amplitude
