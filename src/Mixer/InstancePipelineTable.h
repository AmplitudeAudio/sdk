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

#ifndef _AM_IMPLEMENTATION_MIXER_INSTANCE_PIPELINE_TABLE_H
#define _AM_IMPLEMENTATION_MIXER_INSTANCE_PIPELINE_TABLE_H

#include <memory>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Pipeline.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Fixed-capacity map from a channel instance ID to the pipeline that renders that
     * instance.
     *
     * All slots are allocated by the constructor, on the game thread. @c Attach, @c Detach, @c
     * Find and @c Clear only fill, empty and scan existing slots, so they never allocate.
     * Lookups are linear: the capacity is small (engine config @c max_instance_pipelines, 32
     * by default).
     */
    class InstancePipelineTable
    {
    public:
        explicit InstancePipelineTable(AmSize capacity);

        /**
         * @brief Stores @p pipeline for @p id in a free slot.
         *
         * @return @c false if @p id is invalid, @p pipeline is null, @p id is already present,
         * or the table is full.
         */
        bool Attach(AmChannelInstanceID id, std::shared_ptr<PipelineInstance> pipeline);

        /**
         * @brief Removes the pipeline stored for @p id.
         *
         * @return The removed pipeline, or @c nullptr if @p id was not present.
         */
        std::shared_ptr<PipelineInstance> Detach(AmChannelInstanceID id);

        /**
         * @brief Gets the pipeline stored for @p id, or @c nullptr if absent.
         */
        [[nodiscard]] PipelineInstance* Find(AmChannelInstanceID id) const;

        /**
         * @brief Empties every slot.
         */
        void Clear();

        [[nodiscard]] AmSize GetCapacity() const;

        [[nodiscard]] AmSize GetSize() const;

    private:
        struct Slot
        {
            AmChannelInstanceID id = kAmInvalidObjectId;
            std::shared_ptr<PipelineInstance> pipeline;
        };

        std::vector<Slot> _slots;
        AmSize _size;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_INSTANCE_PIPELINE_TABLE_H
