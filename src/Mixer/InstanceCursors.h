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

#ifndef _AM_IMPLEMENTATION_MIXER_INSTANCE_CURSORS_H
#define _AM_IMPLEMENTATION_MIXER_INSTANCE_CURSORS_H

#include <vector>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <Core/Playback/ChannelInstanceInternalState.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Gives the instances of a fresh snapshot the cursors the layer advanced since the previous one.
     *
     * A snapshot only carries the initial cursor of an instance. Once the layer plays an instance, the cursor is its
     * own, so an instance that was already there must keep it. An instance that is new keeps the cursor it came with.
     *
     * Runs on the audio thread: no allocation, one pass over each list.
     *
     * @param[in] previous The instances the layer had, with the cursors it advanced.
     * @param[in,out] current The instances of the new snapshot.
     *
     * @note Both lists must be in ascending @c instanceId order, which is how a channel publishes them: instances are
     * appended with increasing ids, and removal keeps the order. An instance that cannot be matched because of a
     * broken order keeps its initial cursor.
     */
    AM_INLINE void CarryOverInstanceCursors(const std::vector<ChannelInstanceData>& previous, std::vector<ChannelInstanceData>& current)
    {
        AmSize p = 0;
        for (auto& instance : current)
        {
            while (p < previous.size() && previous[p].instanceId < instance.instanceId)
                ++p;

            if (p == previous.size())
                break;

            if (previous[p].instanceId == instance.instanceId)
                instance.cursor = previous[p].cursor;
        }
    }
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_INSTANCE_CURSORS_H
