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

#ifndef _AM_IMPLEMENTATION_DSP_CHANNEL_MAP_H
#define _AM_IMPLEMENTATION_DSP_CHANNEL_MAP_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Gets the matrix that plays a @p from source on a @p to output without spatialization.
     *
     * Identity when the layouts match; the ITU fold-down when @p to has fewer channels; a mono source goes to the
     * centre, or to L and R at -3 dB when there is no centre; otherwise each source speaker goes to the nearest output
     * speaker (the LFE to the LFE) and nothing is synthesised.
     */
    ChannelMatrix BuildChannelMap(const ChannelLayout& from, const ChannelLayout& to);

    /**
     * @brief Writes @p frames frames of every channel of @p output as @p matrix applied to @p input.
     *
     * Output channels the matrix does not reach are cleared.
     */
    void ApplyChannelMap(const ChannelMatrix& matrix, const AudioBuffer& input, AudioBuffer& output, AmUInt64 frames);
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_DSP_CHANNEL_MAP_H
