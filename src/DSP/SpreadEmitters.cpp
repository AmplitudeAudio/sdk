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

#include <algorithm>
#include <cmath>

#include <DSP/SpeakerPanner.h>
#include <DSP/SpreadEmitters.h>

namespace SparkyStudios::Audio::Amplitude
{
    AmReal32 SpreadEmitters::Resolve(AmReal32 spread, const ChannelLayout& source)
    {
        if (spread == kAmSpreadNative)
            return source.GetNativeWidth();

        if (!std::isfinite(spread))
            return std::isnan(spread) ? 0.0f : (spread > 0.0f ? 360.0f : 0.0f);

        return std::clamp(spread, 0.0f, 360.0f);
    }

    SpreadEmitterSet SpreadEmitters::Compute(const ChannelLayout& source, AmReal32 centreAzimuth, AmReal32 centreElevation, AmReal32 spread)
    {
        SpreadEmitterSet set;

        const ChannelMatrix fold = ChannelLayout::GetFoldDownMatrix(source, ChannelLayout::Get(eChannelLayout_Mono));
        const AmReal32 native = source.GetNativeWidth();

        // Rank of each non-LFE channel by azimuth: it picks the evenly spaced position the channel reaches at 360.
        std::array<AmUInt16, kAmMaxLayoutChannelCount> order{};
        AmUInt16 count = 0;
        for (AmUInt16 c = 0; c < source.GetChannelCount(); ++c)
            if (!source.IsLfe(c))
                order[count++] = c;

        std::sort(
            order.begin(), order.begin() + count,
            [&](AmUInt16 a, AmUInt16 b)
            {
                return source.GetAzimuth(a) < source.GetAzimuth(b);
            });

        std::array<AmReal32, kAmMaxLayoutChannelCount> even{};
        for (AmUInt16 r = 0; r < count; ++r)
            even[order[r]] = (static_cast<AmReal32>(r) - 0.5f * static_cast<AmReal32>(count - 1)) * 360.0f / static_cast<AmReal32>(count);

        const AmReal32 narrow = native > 0.0f ? std::min(spread, native) / native : 1.0f;
        const AmReal32 wide = native > 0.0f && spread > native && native < 360.0f ? (spread - native) / (360.0f - native) : 0.0f;

        for (AmUInt16 c = 0; c < source.GetChannelCount(); ++c)
        {
            if (source.IsLfe(c))
                continue;

            const AmReal32 a = source.GetAzimuth(c);
            const AmReal32 offset = wide > 0.0f ? a + (even[c] - a) * wide : a * narrow;
            const AmReal32 f = fold.gains[0][c];

            SpreadEmitter& emitter = set.emitters[set.count++];
            emitter.channel = c;
            emitter.azimuth = count > 1 ? WrapDegrees(centreAzimuth + offset) : WrapDegrees(centreAzimuth);
            emitter.elevation = centreElevation;
            emitter.weight = f + (1.0f - f) * narrow;
        }

        return set;
    }
} // namespace SparkyStudios::Audio::Amplitude
