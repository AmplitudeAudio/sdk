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

#include <Core/Codecs/WAV/Codec.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // The streaming thread decodes into a fixed-size chunk, often asking for fewer frames than the chunk holds. Every channel must
    // land at its own channel of the destination buffer, not at an offset derived from the number of frames requested.
    AM_TEST_CASE(DSPTestCase, core_codec_wav, stream_into_wider_buffer_keeps_channels_apart)
    {
    public:
        void Run() override
        {
            WAVCodec codec;

            constexpr AmSize numSamples = 2048;
            constexpr AmSize numChannels = 2;
            constexpr AmSize sampleRate = 44100;
            constexpr AmSize requested = 1000;
            constexpr AmSize chunkFrames = 4096;

            AudioBuffer inputBuffer(numSamples, numChannels);
            for (AmSize i = 0; i < numSamples; ++i)
            {
                inputBuffer[0][i] = 0.25f;
                inputBuffer[1][i] = -0.5f;
            }

            auto encodedFile = std::make_shared<MemoryFile>();
            encodedFile->Open(numSamples * numChannels * sizeof(AmReal32) * 2);

            auto encoder = codec.CreateEncoder();
            SoundFormat format;
            format.SetAll(sampleRate, numChannels, 32, numSamples, numChannels * sizeof(AmAudioSample), eAudioSampleFormat_Float32);
            encoder->SetFormat(format);
            AM_EXPECT(encoder->Open(encodedFile));
            AM_EXPECT_EQ(encoder->Write(&inputBuffer, 0, numSamples), numSamples);
            encoder->Close();
            encodedFile->Seek(0, eFileSeekOrigin_Start);

            auto decoder = codec.CreateDecoder();
            AM_EXPECT(decoder->Open(encodedFile));

            AudioBuffer chunk(chunkFrames, numChannels);
            AM_EXPECT_EQ(decoder->Stream(&chunk, 0, 0, requested), requested);

            bool leftIntact = true;
            bool rightIntact = true;
            for (AmSize i = 0; i < requested; ++i)
            {
                leftIntact = leftIntact && std::abs(chunk[0][i] - 0.25f) < 1e-3f;
                rightIntact = rightIntact && std::abs(chunk[1][i] + 0.5f) < 1e-3f;
            }

            AM_EXPECT(leftIntact);
            AM_EXPECT(rightIntact);

            decoder->Close();
        }
    };

    AM_REGISTER_TEST(core_codec_wav, stream_into_wider_buffer_keeps_channels_apart);
} // namespace SparkyStudios::Audio::Amplitude::Tests
