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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/LayerGainMixer.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, mixer_pipeline, layer_gain_mixer_handles_eight_channels)
    {
    public:
        void Run() override
        {
            AM_EXPECT_EQ(8, kAmplimixMaxOutputChannels);

            GainProcessor processors[kAmplimixMaxOutputChannels];
            for (auto& processor : processors)
                processor.Reset(0.5f);

            AudioBuffer in(64, 8);
            for (AmUInt16 c = 0; c < 8; ++c)
                for (AmUInt64 i = 0; i < 64; ++i)
                    in[c][i] = static_cast<AmReal32>(c + 1);

            AudioBuffer out(64, 8);
            MixLayerWithGain(processors, 8, 0.5f, in, out, 0, 64);

            // Layer, bus and master gains apply to every output channel alike.
            for (AmUInt16 c = 0; c < 8; ++c)
                AM_EXPECT_EQ(0.5f * static_cast<AmReal32>(c + 1), out[c][63]);
        }
    };

    AM_REGISTER_TEST(mixer_pipeline, layer_gain_mixer_handles_eight_channels);
} // namespace SparkyStudios::Audio::Amplitude::Tests
