// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#include <ranges>

#include <SparkyStudios/Audio/Amplitude/Core/Engine.h>
#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>
#include <SparkyStudios/Audio/Amplitude/Core/Environment.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Amplimix.h>

#include <Mixer/Nodes/EnvironmentEffectNode.h>
#include <Mixer/SoundData.h>
#include <Sound/Effect.h>

namespace SparkyStudios::Audio::Amplitude
{
    EnvironmentEffectNodeInstance::EnvironmentEffectNodeInstance()
        : _scratch()
        , _environmentFilters()
    {}

    EnvironmentEffectNodeInstance::~EnvironmentEffectNodeInstance()
    {
        const auto* layer = GetLayer();
        const auto layerId = layer->GetId();

        const Entity& entity = layer->GetEntity();
        if (!entity.Valid())
            return;

        for (const auto environments = entity.GetEnvironments(); const auto& environment : environments | std::views::keys)
        {
            if (!_environmentFilters.contains(environment))
                continue;

            if (!_environmentFilters[environment].contains(layerId))
                continue;

            const Environment& handle = Engine::GetInstance()->GetEnvironment(environment);
            if (!handle.Valid())
                continue;

            _environmentFilters[environment].erase(layerId);
        }
    }

    void EnvironmentEffectNodeInstance::Configure(AmUInt64 frameCount, AmUInt16 channelCount)
    {
        ProcessorNodeInstance::Configure(frameCount, channelCount);

        _scratch = AudioBuffer(frameCount, channelCount);
    }

    bool EnvironmentEffectNodeInstance::ShouldSkip() const
    {
        const auto* layer = GetLayer();
        const Entity& entity = layer->GetEntity();

        if (!entity.Valid())
            return true;

        const auto& environments = entity.GetEnvironments();
        if (environments.empty())
            return true;

        return false;
    }

    const AudioBuffer* EnvironmentEffectNodeInstance::Process(const AudioBuffer* input)
    {
        const auto* layer = GetLayer();
        const auto& entity = layer->GetEntity();
        const auto& environments = entity.GetEnvironments();
        const auto layerId = layer->GetId();

        std::vector<std::pair<AmEnvironmentID, AmReal32>> items(environments.begin(), environments.end());
        std::ranges::sort(
            items,
            [](const std::pair<AmEnvironmentID, AmReal32>& a, const std::pair<AmEnvironmentID, AmReal32>& b) -> bool
            {
                return a.second > b.second;
            });

        _output.Clear();

        for (const auto& [environment, amount] : items)
        {
            if (amount < kEpsilon)
                continue;

            const Environment& handle = Engine::GetInstance()->GetEnvironment(environment);
            if (!handle.Valid())
                continue;

            const auto* effect = static_cast<const EffectImpl*>(handle.GetEffect());
            if (!_environmentFilters.contains(environment))
            {
                const std::unordered_map<AmSoundID, std::shared_ptr<EffectInstance>> map{};
                _environmentFilters[environment] = std::move(map);
            }

            if (!_environmentFilters[environment].contains(layerId))
                _environmentFilters[environment][layerId] = effect->CreateInstance();

            // Clear scratch and process effect
            _scratch.Clear();

            {
                auto filterInstance = _environmentFilters[environment][layerId]->GetFilter();
                filterInstance->SetParameter(0, amount);
                filterInstance->Process(*input, _scratch, input->GetFrameCount(), layer->GetSampleRate());
            }

            _output += _scratch;
        }

        return &_output;
    }

    EnvironmentEffectNode::EnvironmentEffectNode()
        : Node("EnvironmentEffect")
    {}
} // namespace SparkyStudios::Audio::Amplitude
