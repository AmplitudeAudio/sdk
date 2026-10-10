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
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/Voice/Voice.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"
#include "VoiceTestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // Every channel of a multichannel voice is rendered exactly as a mono voice of that channel would be: through the
    // resampler, a start position (pre-roll), loop seams and a seek crossfade.
    AM_TEST_CASE(DSPTestCase, mixer_voice, voice_multichannel_matches_mono_voices)
    {
    public:
        void Run() override
        {
            constexpr AmUInt16 channels = 6;
            constexpr AmUInt64 length = 3000;
            constexpr AmUInt64 block = 256;
            constexpr AmUInt64 total = 6 * block * 4;

            AudioBuffer multi(length, channels);
            for (AmUInt16 c = 0; c < channels; ++c)
                for (AmUInt64 i = 0; i < length; ++i)
                    multi[c][i] = 0.5f * std::sin(0.01f * static_cast<AmReal32>((c + 1) * i));

            const auto render = [&](const AudioBuffer& source)
            {
                VoiceSettings settings = MakeVoiceSettings(source, 44100, block, true);
                settings.startPosition = 700;

                Voice voice;
                AM_EXPECT(voice.Initialize(settings));

                std::vector<std::vector<AmReal32>> out(source.GetChannelCount());
                AudioBuffer buffer(block, source.GetChannelCount());
                for (AmUInt64 done = 0; done < total; done += block)
                {
                    if (done == 8 * block)
                        voice.Enqueue(MakeCommand(eVoiceCommandKind::Seek, done + 37, 0.0, 2100));

                    voice.BeginBlock(done, block);
                    buffer.Clear();
                    voice.RenderPrimary(buffer);
                    voice.EndBlock();

                    for (AmUInt16 c = 0; c < source.GetChannelCount(); ++c)
                        out[c].insert(out[c].end(), buffer[c].begin(), buffer[c].begin() + block);
                }

                return out;
            };

            const auto rendered = render(multi);

            for (AmUInt16 c = 0; c < channels; ++c)
            {
                AudioBuffer mono(length, 1);
                std::copy_n(multi[c].begin(), length, mono[0].begin());

                const auto expected = render(mono);
                for (AmUInt64 i = 0; i < total; ++i)
                    AM_EXPECT_EQ(expected[0][i], rendered[c][i]);
            }
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_multichannel_matches_mono_voices);
} // namespace SparkyStudios::Audio::Amplitude::Tests
