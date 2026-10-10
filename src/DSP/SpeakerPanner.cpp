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
#include <Math/LinearAlgebra.h>

namespace SparkyStudios::Audio::Amplitude
{
    namespace
    {
        constexpr AmReal64 kDegreesToRadians = 3.14159265358979323846 / 180.0;

        AmReal64 Wrap360(AmReal64 degrees)
        {
            const AmReal64 wrapped = std::fmod(degrees, 360.0);
            return wrapped < 0.0 ? wrapped + 360.0 : wrapped;
        }

        /// The speakers that take part in panning (all but the LFE), sorted by azimuth from -180 to 180.
        AmUInt16 Ring(const ChannelLayout& layout, std::array<AmUInt16, kAmMaxLayoutChannelCount>& ring)
        {
            AmUInt16 count = 0;
            for (AmUInt16 c = 0; c < layout.GetChannelCount(); ++c)
                if (!layout.IsLfe(c))
                    ring[count++] = c;

            std::sort(
                ring.begin(), ring.begin() + count,
                [&](AmUInt16 a, AmUInt16 b)
                {
                    return layout.GetAzimuth(a) < layout.GetAzimuth(b);
                });

            return count;
        }

        /// VBAP between two speakers @p span degrees apart, the first at @p first; normalised so g1² + g2² = 1.
        void PanPair(AmReal64 azimuth, AmReal64 first, AmReal64 span, AmUInt16 firstChannel, AmUInt16 secondChannel, SpeakerGains& gains)
        {
            const AmReal64 t = Wrap360(azimuth - first);
            const AmReal64 g1 = std::sin((span - t) * kDegreesToRadians);
            const AmReal64 g2 = std::sin(t * kDegreesToRadians);
            const AmReal64 norm = std::sqrt(g1 * g1 + g2 * g2);

            gains[firstChannel] = static_cast<AmReal32>(g1 / norm);
            gains[secondChannel] = static_cast<AmReal32>(g2 / norm);
        }
    } // namespace

    AmReal32 WrapDegrees(AmReal32 degrees)
    {
        AmReal64 wrapped = std::fmod(static_cast<AmReal64>(degrees), 360.0);
        if (wrapped > 180.0)
            wrapped -= 360.0;
        else if (wrapped <= -180.0)
            wrapped += 360.0;

        return static_cast<AmReal32>(wrapped);
    }

    bool ListenerRelativeDirection(const AmVector3& location, const AmMatrix4& listenerView, AmReal32& azimuth, AmReal32& elevation)
    {
        const AmVector4 local = Transform(listenerView, { location.x, location.y, location.z, 1.0f });
        if (SquaredLength(local.xyz) <= kEpsilon)
        {
            azimuth = elevation = 0.0f;
            return false;
        }

        // The ears-centred frame the HRTF path uses: its azimuth is negative to the left.
        const SphericalPosition position = SphericalPosition::ForHRTF(Normalize(local.xyz));
        azimuth = WrapDegrees(-position.GetAzimuth() * AM_RadToDeg);
        elevation = position.GetElevation() * AM_RadToDeg;
        return true;
    }

    SpeakerGains SpeakerPanner::Pan(AmReal32 azimuth, AmReal32 elevation, const ChannelLayout& layout)
    {
        AM_UNUSED(elevation);

        SpeakerGains gains{};
        std::array<AmUInt16, kAmMaxLayoutChannelCount> ring{};
        const AmUInt16 count = Ring(layout, ring);

        if (count == 1)
        {
            gains[ring[0]] = 1.0f;
            return gains;
        }

        AmReal64 direction = WrapDegrees(azimuth);

        if (count == 2)
        {
            // A front pair cannot render the rear: fold to the mirror in front, on the same side, then clamp to the pair.
            if (direction > 90.0)
                direction = 180.0 - direction;
            else if (direction < -90.0)
                direction = -180.0 - direction;

            const AmReal64 right = layout.GetAzimuth(ring[0]);
            const AmReal64 left = layout.GetAzimuth(ring[1]);
            direction = std::clamp(direction, right, left);

            PanPair(direction, right, left - right, ring[0], ring[1], gains);
            return gains;
        }

        for (AmUInt16 j = 0; j < count; ++j)
        {
            const AmUInt16 next = static_cast<AmUInt16>((j + 1) % count);
            const AmReal64 first = layout.GetAzimuth(ring[j]);
            const AmReal64 span = Wrap360(layout.GetAzimuth(ring[next]) - first);

            if (Wrap360(direction - first) <= span)
            {
                PanPair(direction, first, span, ring[j], ring[next], gains);
                return gains;
            }
        }

        AMPLITUDE_ASSERT(false); // The ring closes the circle: some pair always contains the direction.
        return gains;
    }
} // namespace SparkyStudios::Audio::Amplitude
