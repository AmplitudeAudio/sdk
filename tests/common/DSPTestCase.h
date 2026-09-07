// Copyright (c) 2021-present Sparky Studios. All rights reserved.
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

#include <vector>

#include <DSP/Resamplers/DefaultResampler.h>

#include "ComponentTestCase.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    class DSPTestCase : public ComponentTestCase
    {
    public:
        void SetUp() override
        {
            ComponentTestCase::SetUp();
            _resampler = Engine::RegisterExtension<DefaultResampler>();
        }

        void TearDown() override
        {
            Engine::UnregisterExtension(_resampler);
            ComponentTestCase::TearDown();
        }

    protected:
        void GenerateSineWave(AudioBuffer& buffer, AmUInt32 sampleRate, AmReal32 dcOffset = 0.0f)
        {
            for (AmUInt16 c = 0, m = buffer.GetChannelCount(); c < m; ++c)
                for (AmUInt64 i = 0, n = buffer.GetFrameCount(); i < n; ++i)
                    buffer[c][i] =
                        std::sin(2.0f * AM_PI32 * 1000.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate)) + dcOffset;
        }

        /**
         * @brief Drives a resampler instance through rate pairs that are hostile to fixed-size filter buffers.
         *
         * Any resampler, bundled or plugin-provided, must survive this sweep: the interface contract requires
         * Initialize() and SetSampleRate() to accept every positive rate pair without reading or writing out of
         * bounds, approximating the ratio when it cannot be represented exactly.
         *
         * @param[in] instance The resampler instance to exercise.
         * @param[in] channelCount The channel count to configure.
         *
         * @return @c true when every conversion produced a plausible frame count.
         */
        static bool RunResamplerConformanceSweep(ResamplerInstance& instance, AmUInt16 channelCount = 1)
        {
            constexpr AmUInt32 sourceRates[] = { 8000, 11025, 22050, 22254, 32000, 32075, 44056, 44100, 48000, 96000, 192000 };
            constexpr AmUInt32 targetRates[] = { 44100, 48000, 96000 };
            constexpr AmUInt64 inputFrames = 256;

            for (const AmUInt32 sourceSampleRate : sourceRates)
            {
                for (const AmUInt32 targetSampleRate : targetRates)
                {
                    instance.Initialize(channelCount, sourceSampleRate, targetSampleRate);

                    AudioBuffer inputBuffer(inputFrames, channelCount);
                    const AmUInt64 expectedOutputFrames = instance.GetExpectedOutputFrames(inputFrames);

                    if (expectedOutputFrames == 0)
                        return false;

                    AudioBuffer outputBuffer(expectedOutputFrames, channelCount);

                    AmUInt64 processedInputFrames = inputFrames;
                    AmUInt64 processedOutputFrames = expectedOutputFrames;

                    if (!instance.Process(inputBuffer, processedInputFrames, outputBuffer, processedOutputFrames))
                        return false;

                    if (processedOutputFrames > expectedOutputFrames)
                        return false;
                }
            }

            // Pitch path: ratios expressed in thousandths, as AmplimixImpl::UpdatePitch does.
            for (AmUInt32 s = 1; s <= 4000; ++s)
                instance.SetSampleRate(s, 1000);

            // Degenerate input the mixer can produce in release builds.
            instance.SetSampleRate(0, 1000);

            return true;
        }

        bool EnsureHasNonZeroOutput(const AudioBuffer& buffer)
        {
            bool hasOutput = false;
            for (AmUInt16 c = 0, m = buffer.GetChannelCount(); c < m && !hasOutput; ++c)
                for (AmUInt64 i = 0, n = buffer.GetFrameCount(); i < n && !hasOutput; ++i)
                    hasOutput = std::abs(buffer[c][i]) > kEpsilon;

            return hasOutput;
        }

        bool EnsureHasZeroOutput(const AudioBuffer& buffer)
        {
            return !EnsureHasNonZeroOutput(buffer);
        }

        bool EnsureBufferEqual(const AudioBuffer& buffer1, const AudioBuffer& buffer2)
        {
            if (buffer1.GetChannelCount() != buffer2.GetChannelCount() || buffer1.GetFrameCount() != buffer2.GetFrameCount())
                return false;

            for (AmUInt16 c = 0, m = buffer1.GetChannelCount(); c < m; ++c)
                for (AmUInt64 i = 0, n = buffer1.GetFrameCount(); i < n; ++i)
                    if (std::abs(buffer1[c][i] - buffer2[c][i]) > kEpsilon)
                        return false;

            return true;
        }

        // --- Signal generation helpers ---

        void GenerateSineWaveAtFrequency(
            AudioBuffer& buffer, AmUInt32 sampleRate, AmReal32 frequency, AmReal32 amplitude = 1.0f)
        {
            for (AmUInt16 c = 0, m = buffer.GetChannelCount(); c < m; ++c)
                for (AmUInt64 i = 0, n = buffer.GetFrameCount(); i < n; ++i)
                    buffer[c][i] = amplitude *
                        std::sin(2.0f * AM_PI32 * frequency * static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate));
        }

        void GenerateMultiTone(
            AudioBuffer& buffer, AmUInt32 sampleRate, const std::vector<AmReal32>& frequencies, AmReal32 amplitude = 0.3f)
        {
            for (AmUInt16 c = 0, m = buffer.GetChannelCount(); c < m; ++c)
            {
                for (AmUInt64 i = 0, n = buffer.GetFrameCount(); i < n; ++i)
                {
                    AmReal32 sample = 0.0f;
                    for (AmReal32 freq : frequencies)
                        sample += amplitude *
                            std::sin(2.0f * AM_PI32 * freq * static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate));
                    buffer[c][i] = sample;
                }
            }
        }

        void GenerateImpulse(AudioBuffer& buffer, AmReal32 amplitude = 1.0f, AmUInt64 position = 0)
        {
            for (AmUInt16 c = 0, m = buffer.GetChannelCount(); c < m; ++c)
                for (AmUInt64 i = 0, n = buffer.GetFrameCount(); i < n; ++i)
                    buffer[c][i] = (i == position) ? amplitude : 0.0f;
        }

        // --- Signal analysis helpers ---

        AmReal32 CalculateRMS(const AudioBuffer& buffer)
        {
            AmReal64 sum = 0.0;
            AmUInt64 count = 0;
            for (AmUInt16 c = 0, m = buffer.GetChannelCount(); c < m; ++c)
            {
                for (AmUInt64 i = 0, n = buffer.GetFrameCount(); i < n; ++i)
                {
                    sum += static_cast<AmReal64>(buffer[c][i]) * static_cast<AmReal64>(buffer[c][i]);
                    ++count;
                }
            }
            return static_cast<AmReal32>(std::sqrt(sum / static_cast<AmReal64>(count)));
        }

        AmReal32 CalculatePeak(const AudioBuffer& buffer)
        {
            AmReal32 peak = 0.0f;
            for (AmUInt16 c = 0, m = buffer.GetChannelCount(); c < m; ++c)
                for (AmUInt64 i = 0, n = buffer.GetFrameCount(); i < n; ++i)
                    peak = std::max(peak, std::abs(buffer[c][i]));
            return peak;
        }

        AmReal32 CalculateDCOffset(const AudioBuffer& buffer)
        {
            AmReal64 sum = 0.0;
            AmUInt64 count = 0;
            for (AmUInt16 c = 0, m = buffer.GetChannelCount(); c < m; ++c)
            {
                for (AmUInt64 i = 0, n = buffer.GetFrameCount(); i < n; ++i)
                {
                    sum += static_cast<AmReal64>(buffer[c][i]);
                    ++count;
                }
            }
            return static_cast<AmReal32>(sum / static_cast<AmReal64>(count));
        }

        // --- Signal assertion helpers ---

        void ExpectRMSNear(AmReal32 expected, const AudioBuffer& buffer, AmReal32 tolerance = 0.01f)
        {
            AmReal32 actual = CalculateRMS(buffer);
            AM_EXPECT(std::abs(expected - actual) <= tolerance);
        }

        void ExpectPeakNear(AmReal32 expected, const AudioBuffer& buffer, AmReal32 tolerance = 0.01f)
        {
            AmReal32 actual = CalculatePeak(buffer);
            AM_EXPECT(std::abs(expected - actual) <= tolerance);
        }

        void ExpectNoDCOffset(const AudioBuffer& buffer, AmReal32 tolerance = 0.01f)
        {
            AM_EXPECT(std::abs(CalculateDCOffset(buffer)) <= tolerance);
        }

        void ExpectSilent(const AudioBuffer& buffer, AmReal32 threshold = 0.001f)
        {
            AM_EXPECT(CalculateRMS(buffer) < threshold);
        }

        void ExpectNotSilent(const AudioBuffer& buffer, AmReal32 threshold = 0.001f)
        {
            AM_EXPECT(CalculateRMS(buffer) >= threshold);
        }

        void ExpectGainApplied(
            const AudioBuffer& input, const AudioBuffer& output, AmReal32 expectedGain, AmReal32 tolerance = 0.1f)
        {
            AmReal32 inputRMS = CalculateRMS(input);
            if (inputRMS < 1e-6f)
                return;
            AmReal32 outputRMS = CalculateRMS(output);
            AmReal32 actualGain = outputRMS / inputRMS;
            AM_EXPECT(std::abs(expectedGain - actualGain) <= tolerance);
        }

    private:
        std::shared_ptr<DefaultResampler> _resampler;
    };
} // namespace SparkyStudios::Audio::Amplitude::Tests
