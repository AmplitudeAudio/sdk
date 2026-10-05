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
#include <chrono>
#include <cstdint>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Fidelity/ResamplerBench.h>
#include <Fidelity/Signal.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    namespace
    {
        constexpr std::uint64_t kBlock = 512;
        constexpr std::uint32_t kRate = 48000;

        /// Blocks the untimed warm-up renders, so table building and first touch stay out of the clock.
        constexpr std::uint64_t kWarmUpBlocks = 16;

        /// Renders @p outputFrames of audio in kBlock-sized blocks, from a fixed input buffer the stream never advances past.
        void RenderBlocks(ResamplerInstance& resampler, const AudioBuffer& input, AudioBuffer& output, std::uint64_t outputFrames)
        {
            for (std::uint64_t produced = 0; produced < outputFrames; produced += kBlock)
            {
                AmUInt64 inputFrames = resampler.GetInputFramesNeeded(kBlock);
                AmUInt64 wanted = kBlock;
                resampler.Process(input, inputFrames, output, wanted);
            }
        }
    } // namespace

    std::vector<ResamplerBenchRow> RunResamplerBench(double seconds)
    {
        const bool ownsMemory = !MemoryManager::IsInitialized();
        if (ownsMemory)
            MemoryManager::Initialize();

        Engine::RegisterDefaultExtensions();

        const auto total = static_cast<std::uint64_t>(seconds * kRate);

        std::vector<ResamplerBenchRow> rows;
        for (const char* preset : { "linear", "cubic", "sinc", "sinc_best" })
        {
            for (const double ratio : { 1.0, 44100.0 / 48000.0, 2.0, 4.0 })
            {
                // At a ratio of 4 a 512-frame output block reaches 2048 input frames plus one stretched kernel reach of
                // 160, so kBlock * 5 input frames covers every cell. The same buffer is fed to every block: nothing
                // copies into it inside the clock, and the resampler keeps its own phase across calls.
                AudioBuffer in(kBlock * 5, 1);
                Random random(7);
                for (std::size_t i = 0; i < in.GetFrameCount(); ++i)
                    in[0][i] = static_cast<float>(0.5 * random.Symmetric());

                AudioBuffer out(kBlock, 1);

                const std::shared_ptr<ResamplerInstance> r = Resampler::Construct(preset);
                if (r == nullptr)
                {
                    // A missing preset is a harness setup failure; a zero keeps the row count and fails loudly on it.
                    rows.push_back({ preset, ratio, 0.0 });
                    continue;
                }

                r->Initialize(1, kRate, kRate);
                r->SetRatio(ratio);

                // Building the kernel table and touching the buffers for the first time are one-time costs: warm the
                // path with a fixed, short pass whose result is discarded, so the clock only sees the steady state.
                RenderBlocks(*r, in, out, kWarmUpBlocks * kBlock);

                const auto start = std::chrono::steady_clock::now();
                RenderBlocks(*r, in, out, total);
                const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();

                rows.push_back({ preset, ratio, seconds / std::max(elapsed, 1e-9) });
            }
        }

        Engine::UnregisterDefaultExtensions();
        if (ownsMemory)
            MemoryManager::Deinitialize();

        return rows;
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity