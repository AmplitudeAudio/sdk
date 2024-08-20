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

#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    static void InterleaveStereo(
        AmSize length, const AmReal32* channel0, const AmReal32* channel1, AmUInt64 inOffset, AmReal32* out, AmUInt64 outOffset)
    {
        AmSize remaining = length;

#if defined(AM_SIMD_INTRINSICS)
        const AmSize end = GetNumSimdChunks(length);
        constexpr AmSize blockSize = GetSimdBlockSize();
        remaining = remaining - end;

        for (AmSize i = 0; i < end; i += blockSize)
        {
            auto c0 = xsimd::load_aligned(&channel0[i + inOffset]);
            auto c1 = xsimd::load_aligned(&channel1[i + inOffset]);

            const AmSize k = 2 * (i + outOffset);
            xsimd::store_aligned(&out[k], xsimd::zip_lo(c0, c1));
            xsimd::store_aligned(&out[k + blockSize], xsimd::zip_hi(c0, c1));
        }
#endif // AM_SIMD_INTRINSICS

        for (AmSize i = length - remaining; i < length; ++i)
        {
            const AmSize k = 2 * (i + outOffset);
            out[k] = channel0[i + inOffset];
            out[k + 1] = channel1[i + inOffset];
        }
    }

    void Interleave(const AudioBuffer* in, AmUInt64 inOffset, AmReal32* out, AmUInt64 outOffset, AmInt32 numSamples, AmInt32 numChannels)
    {
        if (numChannels == 1)
        {
            std::memcpy(out + outOffset, in->GetChannel(0).begin() + inOffset, numSamples * sizeof(AmReal32));
            return;
        }
        else if (numChannels == 2)
        {
            InterleaveStereo(numSamples, in->GetChannel(0).begin(), in->GetChannel(1).begin(), inOffset, out, outOffset);
            return;
        }

        for (AmInt32 j = 0; j < numChannels; ++j)
        {
            const auto& channel = in->GetChannel(j);
            for (AmInt32 i = 0; i < numSamples; ++i)
                out[(i + outOffset) * numChannels + j] = channel[i + inOffset];
        }
    }
} // namespace SparkyStudios::Audio::Amplitude
