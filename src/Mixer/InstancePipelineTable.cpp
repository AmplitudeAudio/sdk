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

#include <Mixer/InstancePipelineTable.h>

namespace SparkyStudios::Audio::Amplitude
{
    InstancePipelineTable::InstancePipelineTable(AmSize capacity)
        : _slots(capacity)
        , _size(0)
    {}

    bool InstancePipelineTable::Attach(AmChannelInstanceID id,
                                       std::shared_ptr<PipelineInstance> pipeline)
    {
        if (id == kAmInvalidObjectId || pipeline == nullptr || Find(id) != nullptr)
            return false;

        for (auto& slot : _slots)
        {
            if (slot.id != kAmInvalidObjectId)
                continue;

            slot.id = id;
            slot.pipeline = std::move(pipeline);
            ++_size;
            return true;
        }

        return false;
    }

    std::shared_ptr<PipelineInstance> InstancePipelineTable::Detach(AmChannelInstanceID id)
    {
        if (id == kAmInvalidObjectId)
            return nullptr;

        for (auto& slot : _slots)
        {
            if (slot.id != id)
                continue;

            slot.id = kAmInvalidObjectId;
            --_size;
            return std::move(slot.pipeline);
        }

        return nullptr;
    }

    PipelineInstance* InstancePipelineTable::Find(AmChannelInstanceID id) const
    {
        if (id == kAmInvalidObjectId)
            return nullptr;

        for (const auto& slot : _slots)
            if (slot.id == id)
                return slot.pipeline.get();

        return nullptr;
    }

    void InstancePipelineTable::Clear()
    {
        for (auto& slot : _slots)
        {
            slot.id = kAmInvalidObjectId;
            slot.pipeline = nullptr;
        }

        _size = 0;
    }

    AmSize InstancePipelineTable::GetCapacity() const
    {
        return _slots.size();
    }

    AmSize InstancePipelineTable::GetSize() const
    {
        return _size;
    }
} // namespace SparkyStudios::Audio::Amplitude
