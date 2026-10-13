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

#include <cmath>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <DSP/ChannelMap.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_panning, stereo_asset_2d_equals_mono_fold_down)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frames = 512;
            const auto& mono = ChannelLayout::Get(eChannelLayout_Mono);
            const auto& stereo = ChannelLayout::Get(eChannelLayout_Stereo);

            // The same content on both channels: what F8 compares.
            AudioBuffer asset(frames, 2);
            for (AmUInt64 i = 0; i < frames; ++i)
                asset[0][i] = asset[1][i] = 0.5f * std::sin(0.05f * static_cast<AmReal32>(i));

            // The asset played 2D on a stereo output.
            AudioBuffer direct(frames, 2);
            ApplyChannelMap(BuildChannelMap(stereo, stereo), asset, direct, frames);

            // Its mono fold-down, played 2D (centred) on the same output.
            AudioBuffer folded(frames, 1);
            ApplyChannelMap(ChannelLayout::GetFoldDownMatrix(stereo, mono), asset, folded, frames);
            AudioBuffer centred(frames, 2);
            ApplyChannelMap(BuildChannelMap(mono, stereo), folded, centred, frames);

            for (AmUInt16 c = 0; c < 2; ++c)
                for (AmUInt64 i = 0; i < frames; ++i)
                    AM_EXPECT(std::abs(direct[c][i] - centred[c][i]) < 1e-6f);
        }
    };

    AM_REGISTER_TEST(dsp_panning, stereo_asset_2d_equals_mono_fold_down);
} // namespace SparkyStudios::Audio::Amplitude::Tests
