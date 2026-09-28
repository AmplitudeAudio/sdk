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

#include <Fidelity/Analysis/Null.h>
#include <Fidelity/AssetGenerator.h>
#include <Fidelity/Scenarios/Common.h>
#include <Fidelity/Scenarios/Playback.h>
#include <Fidelity/Targets.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    namespace
    {
        void NullCompare(const Capture& a, const Capture& b, Measurement& out)
        {
            NullOptions options;
            options.matchGain = false;
            const NullResult r = AnalyzeNull(ChannelSignal(a, 0), ChannelSignal(b, 0), options);

            out.Add("null.residualPeakDbfs", r.residualPeakDbfs, "dBFS", Better::Lower, Targets::kMaxNullResidualDbfs);
            out.Add("null.residualRmsDbfs", r.residualRmsDbfs, "dBFS", Better::Lower);
            out.Add("null.lagSamples", std::abs(static_cast<double>(r.lag)), "samples", Better::Lower);
            out.residual = { ToFloats(r.residual) };
        }

        /**
         * @brief Two renders of one stimulus that should be identical; subclasses choose how they differ.
         */
        class EquivalenceScenario : public Scenario
        {
        public:
            [[nodiscard]] std::uint32_t Dimensions() const override
            {
                return 0;
            }

            [[nodiscard]] std::vector<std::string> Variants() const override
            {
                return { "sine_997_44100", "sweep_44100", "pink_44100" };
            }

            void Measure(const RunContext& context, const GridPoint& point, const std::string& variant, Measurement& out) const override
            {
                const StimulusSpec* spec = FindStimulus(variant);
                if (spec == nullptr)
                {
                    out.error = "unknown stimulus " + variant;
                    return;
                }

                const std::uint64_t outFrames = OutputFrames(*spec, point.outputRate);
                const std::uint64_t duration = kLeadIn + outFrames + Seconds(0.1, point.outputRate);

                Capture a;
                Capture b;
                if (!RenderPair(context, point, *spec, duration, out, a, b))
                    return;

                if (!RequireSignal(out, a, kLeadIn, kLeadIn + outFrames) || !RequireSignal(out, b, kLeadIn, kLeadIn + outFrames))
                {
                    out.capture = std::move(a);
                    return;
                }

                NullCompare(a, b, out);

                if (spec->kind == StimulusKind::Sine && ClicksOnSecond())
                    AddClickMetrics(out, b, 4000.0, kLeadIn, duration);

                out.capture = std::move(a);
            }

        protected:
            virtual bool RenderPair(
                const RunContext& context,
                const GridPoint& point,
                const StimulusSpec& spec,
                std::uint64_t duration,
                Measurement& out,
                Capture& a,
                Capture& b) const = 0;

            [[nodiscard]] virtual bool ClicksOnSecond() const
            {
                return false;
            }
        };

        class StreamingEquivalenceScenario final : public EquivalenceScenario
        {
        public:
            [[nodiscard]] std::string Id() const override
            {
                return "P5";
            }

            [[nodiscard]] std::string Title() const override
            {
                return "Streamed vs in-memory";
            }

        protected:
            bool RenderPair(
                const RunContext& context,
                const GridPoint& point,
                const StimulusSpec& spec,
                std::uint64_t duration,
                Measurement& out,
                Capture& a,
                Capture& b) const override
            {
                const RenderSettings settings = IsolatedSettings(point, duration);
                return RenderDeterministic(context, settings, PlayOnly(SoundName(spec, false)), "memory", out, a) &&
                    RenderDeterministic(context, settings, PlayOnly(SoundName(spec, true)), "stream", out, b);
            }
        };

        class BlockSizeIndependenceScenario final : public EquivalenceScenario
        {
        public:
            [[nodiscard]] std::string Id() const override
            {
                return "P6";
            }

            [[nodiscard]] std::string Title() const override
            {
                return "Block-size independence";
            }

        protected:
            bool RenderPair(
                const RunContext& context,
                const GridPoint& point,
                const StimulusSpec& spec,
                std::uint64_t duration,
                Measurement& out,
                Capture& a,
                Capture& b) const override
            {
                GridPoint small = point;
                small.blockSize = 256;
                GridPoint large = point;
                large.blockSize = 4096;

                const ActionFactory actions = PlayOnly(SoundName(spec, false));
                return RenderDeterministic(context, IsolatedSettings(small, duration), actions, "b256", out, a) &&
                    RenderDeterministic(context, IsolatedSettings(large, duration), actions, "b4096", out, b);
            }
        };

        class VariableCallbackScenario final : public EquivalenceScenario
        {
        public:
            [[nodiscard]] std::string Id() const override
            {
                return "P8";
            }

            [[nodiscard]] std::string Title() const override
            {
                return "Variable callback size";
            }

            [[nodiscard]] std::vector<std::string> Variants() const override
            {
                return { "sine_997_44100", "pink_44100" };
            }

        protected:
            bool RenderPair(
                const RunContext& context,
                const GridPoint& point,
                const StimulusSpec& spec,
                std::uint64_t duration,
                Measurement& out,
                Capture& a,
                Capture& b) const override
            {
                const RenderSettings fixed = IsolatedSettings(point, duration);

                // Real devices sometimes ask for fewer frames than the nominal period.
                RenderSettings variable = fixed;
                Random random(7);
                for (int i = 0; i < 64; ++i)
                    variable.blockSequence.push_back(
                        256 + static_cast<std::uint32_t>(std::floor(random.Uniform() * static_cast<double>(point.blockSize - 255))));

                const ActionFactory actions = PlayOnly(SoundName(spec, false));
                return RenderDeterministic(context, fixed, actions, "fixed", out, a) &&
                    RenderDeterministic(context, variable, actions, "variable", out, b);
            }

            [[nodiscard]] bool ClicksOnSecond() const override
            {
                return true;
            }
        };
    } // namespace

    std::unique_ptr<Scenario> MakeStreamingEquivalenceScenario()
    {
        return std::make_unique<StreamingEquivalenceScenario>();
    }

    std::unique_ptr<Scenario> MakeBlockSizeIndependenceScenario()
    {
        return std::make_unique<BlockSizeIndependenceScenario>();
    }

    std::unique_ptr<Scenario> MakeVariableCallbackScenario()
    {
        return std::make_unique<VariableCallbackScenario>();
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
