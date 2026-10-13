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
#include <cstdio>
#include <limits>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Fidelity/ResamplerBench.h>
#include <Fidelity/Signal.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    namespace
    {
        constexpr std::uint64_t kBlock = 512;
        constexpr std::uint32_t kRate = 48000;

        /// Blocks the untimed warm-up renders, so the first touch of the buffers stays out of the clock.
        constexpr std::uint64_t kWarmUpBlocks = 16;

        /// Renders @p outputFrames of audio in kBlock-sized blocks, from a fixed input buffer the stream never advances
        /// past. Returns false as soon as a block produces less than it owes: the input buffer is too small to feed the
        /// ratio, and the loop would otherwise credit the missing frames as if it had produced them.
        bool RenderBlocks(ResamplerInstance& resampler, const AudioBuffer& input, AudioBuffer& output, std::uint64_t outputFrames)
        {
            for (std::uint64_t rendered = 0; rendered < outputFrames; rendered += kBlock)
            {
                AmUInt64 inputFrames = resampler.GetInputFramesNeeded(kBlock);
                AmUInt64 produced = kBlock;
                resampler.Process(input, inputFrames, output, produced);

                // Only the last block can legitimately stop short: it asks for a full kBlock but owes the remainder.
                if (produced < AM_MIN(kBlock, outputFrames - rendered))
                    return false;
            }

            return true;
        }
    } // namespace

    std::vector<ResamplerBenchRow> RunResamplerBench(double seconds)
    {
        // Owns the extension registry for its duration: it registers the defaults on the way in and unregisters them on
        // the way out, so the registry is left empty rather than as it was found. RenderSession::Render() does the same.
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
                    // A missing preset is a harness setup failure; a NaN keeps the row count, prints as `nan` rather than
                    // as a plausible zero, and fails any check on the throughput.
                    rows.push_back({ preset, ratio, std::numeric_limits<double>::quiet_NaN() });
                    continue;
                }

                r->Initialize(1, kRate, kRate);
                r->SetRatio(ratio);

                // Touching the buffers, the i-cache and the branch predictors are one-time costs: warm the path with a
                // fixed, short pass whose result is discarded, so the clock only sees the steady state. The kernel table
                // is not one of them -- it is built inside Resampler::Construct(), before this pass.
                const bool warmed = RenderBlocks(*r, in, out, kWarmUpBlocks * kBlock);

                const auto start = std::chrono::steady_clock::now();
                const bool starved = !warmed || !RenderBlocks(*r, in, out, total);
                const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();

                if (starved)
                {
                    // The input buffer is sized for ratios up to 4. A starved block would read as too fast: report NaN.
                    std::fprintf(
                        stderr,
                        "resampler bench: '%s' at ratio %.9g is starved: a %llu-frame input buffer cannot feed "
                        "%llu-frame output blocks, so the cell under-rendered and its throughput reads too high.\n",
                        preset, ratio, static_cast<unsigned long long>(in.GetFrameCount()), static_cast<unsigned long long>(kBlock));
                    rows.push_back({ preset, ratio, std::numeric_limits<double>::quiet_NaN() });
                    continue;
                }

                rows.push_back({ preset, ratio, seconds / std::max(elapsed, 1e-9) });
            }
        }

        Engine::UnregisterDefaultExtensions();
        if (ownsMemory)
            MemoryManager::Deinitialize();

        return rows;
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
