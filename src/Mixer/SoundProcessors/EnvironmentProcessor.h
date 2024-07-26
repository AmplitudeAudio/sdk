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

#ifndef SS_AMPLITUDE_AUDIO_ENVIRONMENT_PROCESSOR_H
#define SS_AMPLITUDE_AUDIO_ENVIRONMENT_PROCESSOR_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/Playback/ChannelInternalState.h>
#include <Mixer/RealChannel.h>

#include "sound_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    class EnvironmentProcessorInstance final : public SoundProcessorInstance
    {
    public:
        void Process(
            AmAudioSampleBuffer out,
            AmConstAudioSampleBuffer in,
            AmUInt64 frames,
            AmSize bufferSize,
            AmUInt16 channels,
            AmUInt32 sampleRate,
            const AmplimixLayer* layer) override
        {
            if (const Entity& entity = layer->GetEntity(); entity.Valid())
            {
                auto environments = entity.GetEnvironments();
                std::vector<std::pair<AmEnvironmentID, AmReal32>> items(environments.begin(), environments.end());
                std::ranges::sort(
                    items,
                    [](const std::pair<AmEnvironmentID, AmReal32>& a, const std::pair<AmEnvironmentID, AmReal32>& b) -> bool
                    {
                        return a.second > b.second;
                    });

                for (const auto& [environment, amount] : items)
                {
                    if (amount == 0.0f)
                        continue;

                    const Environment& handle = amEngine->GetEnvironment(environment);
                    if (!handle.Valid())
                        continue;

                    const auto* effect = static_cast<const EffectImpl*>(handle.GetEffect());
                    if (!_environmentFilters.contains(environment))
                    {
                        const std::map<AmSoundID, EffectInstance*> map{};
                        _environmentFilters[environment] = map;
                    }

                    if (!_environmentFilters[environment].contains(layer->GetId()))
                    {
                        _environmentFilters[environment][layer->GetId()] = effect->CreateInstance();
                    }

                    SoundChunk* scratch = SoundChunk::CreateChunk(frames, channels, MemoryPoolKind::Amplimix);
                    std::memcpy(reinterpret_cast<AmAudioSampleBuffer>(scratch->buffer), in, bufferSize);

                    FilterInstance* filterInstance = _environmentFilters[environment][layer->GetId()]->GetFilter();
                    filterInstance->SetFilterParameter(0, amount);
                    filterInstance->Process(
                        reinterpret_cast<AmAudioSampleBuffer>(scratch->buffer), frames, bufferSize, channels, sampleRate);

                    std::memcpy(out, reinterpret_cast<AmAudioSampleBuffer>(scratch->buffer), bufferSize);
                    SoundChunk::DestroyChunk(scratch);

                    return;
                }
            }

            if (out != in)
                std::memcpy(out, in, bufferSize);
        }

        void Cleanup(const AmplimixLayer* layer) override
        {
            const Entity& entity = layer->GetEntity();
            if (!entity.Valid())
                return;

            for (const auto environments = entity.GetEnvironments(); const auto& environment : environments | std::views::keys)
            {
                if (!_environmentFilters.contains(environment))
                    continue;

                if (!_environmentFilters[environment].contains(layer->GetId()))
                    continue;

                const Environment& handle = amEngine->GetEnvironment(environment);
                if (!handle.Valid())
                    continue;

                const auto* effect = static_cast<const EffectImpl*>(handle.GetEffect());
                effect->DestroyInstance(_environmentFilters[environment][layer->GetId()]);

                _environmentFilters[environment].erase(layer->GetId());
            }
        }

    private:
        std::map<AmEnvironmentID, std::map<AmObjectID, EffectInstance*>> _environmentFilters = {};
    };

    class EnvironmentProcessor final : public SoundProcessor
    {
    public:
        EnvironmentProcessor()
            : SoundProcessor("EnvironmentProcessor")
        {}

        SoundProcessorInstance* CreateInstance() override
        {
            return ampoolnew(MemoryPoolKind::Amplimix, EnvironmentProcessorInstance);
        }

        void DestroyInstance(SoundProcessorInstance* instance) override
        {
            ampooldelete(MemoryPoolKind::Amplimix, EnvironmentProcessorInstance, (EnvironmentProcessorInstance*)instance);
        }
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_ENVIRONMENT_PROCESSOR_H
