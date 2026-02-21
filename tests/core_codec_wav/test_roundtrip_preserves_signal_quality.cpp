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
    AM_TEST_CASE(DSPTestCase, core_codec_wav, roundtrip_preserves_signal_quality)
    {
    public:
        void Run() override
        {
            WAVCodec codec;

            constexpr AmSize numSamples = 4096;
            constexpr AmSize numChannels = 1;
            constexpr AmSize sampleRate = 48000;

            AudioBuffer inputBuffer(numSamples, numChannels);
            GenerateSineWaveAtFrequency(inputBuffer, sampleRate, 440.0f, 0.5f);

            AmReal32 inputRMS = CalculateRMS(inputBuffer);

            // Test 1: 16-bit PCM round-trip (quantized but close)
            {
                constexpr AmSize estimatedSize = numSamples * numChannels * sizeof(AmReal32) * 2;
                auto encodedFile = std::make_shared<MemoryFile>();
                encodedFile->Open(estimatedSize);

                auto encoder = codec.CreateEncoder();
                SoundFormat format;
                format.SetAll(sampleRate, numChannels, 16, numSamples, numChannels * sizeof(AmAudioSample), eAudioSampleFormat_Int16);
                encoder->SetFormat(format);

                AM_EXPECT(encoder->Open(encodedFile));
                AmUInt64 written = encoder->Write(&inputBuffer, 0, numSamples);
                AM_EXPECT_EQ(written, numSamples);
                encoder->Close();

                encodedFile->Seek(0, eFileSeekOrigin_Start);
                auto decoder = codec.CreateDecoder();
                AM_EXPECT(decoder->Open(encodedFile));

                AudioBuffer outputBuffer(numSamples, numChannels);
                AmUInt64 loaded = decoder->Load(&outputBuffer);
                AM_EXPECT_EQ(loaded, numSamples);
                decoder->Close();

                AmReal32 outputRMS = CalculateRMS(outputBuffer);
                AM_EXPECT(outputRMS > 0.01f);

                // 16-bit quantization: RMS ratio should be very close to 1.0
                AmReal32 rmsRatio = outputRMS / inputRMS;
                AM_EXPECT(rmsRatio > 0.95f);
                AM_EXPECT(rmsRatio < 1.05f);

                // Max per-sample error bounded by 16-bit quantization (~1/32768)
                AmReal32 maxError = 0.0f;
                for (AmUInt64 i = 0; i < numSamples; ++i)
                    maxError = std::max(maxError, std::abs(outputBuffer[0][i] - inputBuffer[0][i]));
                AM_EXPECT(maxError < 0.001f);
            }

            // Test 2: 32-bit float round-trip (lossless)
            {
                constexpr AmSize estimatedSize = numSamples * numChannels * sizeof(AmReal32) * 2;
                auto encodedFile = std::make_shared<MemoryFile>();
                encodedFile->Open(estimatedSize);

                auto encoder = codec.CreateEncoder();
                SoundFormat format;
                format.SetAll(sampleRate, numChannels, 32, numSamples, numChannels * sizeof(AmAudioSample), eAudioSampleFormat_Float32);
                encoder->SetFormat(format);

                AM_EXPECT(encoder->Open(encodedFile));
                AmUInt64 written = encoder->Write(&inputBuffer, 0, numSamples);
                AM_EXPECT_EQ(written, numSamples);
                encoder->Close();

                encodedFile->Seek(0, eFileSeekOrigin_Start);
                auto decoder = codec.CreateDecoder();
                AM_EXPECT(decoder->Open(encodedFile));

                AudioBuffer outputBuffer(numSamples, numChannels);
                AmUInt64 loaded = decoder->Load(&outputBuffer);
                AM_EXPECT_EQ(loaded, numSamples);
                decoder->Close();

                // 32-bit float: should be bit-perfect
                AM_EXPECT(EnsureBufferEqual(inputBuffer, outputBuffer));
            }
        }
    };

    AM_REGISTER_TEST(core_codec_wav, roundtrip_preserves_signal_quality);
} // namespace SparkyStudios::Audio::Amplitude::Tests
