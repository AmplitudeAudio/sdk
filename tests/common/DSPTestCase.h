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

#include <DSP/Resamplers/DefaultResampler.h>

#include "TestCase.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    class DSPTestCase : public TestCase
    {
    public:
        void SetUp() override
        {
            _resampler = Engine::RegisterExtension<DefaultResampler>();
        }

        void TearDown() override
        {
            Engine::UnregisterExtension(_resampler);

            amEngine->DestroyInstance();
        }

        void Run() override;

    protected:
        void GenerateSineWave(AudioBuffer& buffer, AmUInt32 sampleRate, AmReal32 dcOffset = 0.0f)
        {
            for (AmUInt16 c = 0, m = buffer.GetChannelCount(); c < m; ++c)
                for (AmUInt64 i = 0, n = buffer.GetFrameCount(); i < n; ++i)
                    buffer[c][i] =
                        std::sin(2.0f * AM_PI32 * 1000.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate)) + dcOffset;
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

    private:
        std::shared_ptr<DefaultResampler> _resampler;
    };

    std::shared_ptr<TestCase> MakeTestCase()
    {
        return amshared(DSPTestCase);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
