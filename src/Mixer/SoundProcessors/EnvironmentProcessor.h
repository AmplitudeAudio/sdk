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

#include <Core/ChannelInternalState.h>
#include <Mixer/RealChannel.h>

#include "sound_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    static std::unordered_map<AmEnvironmentID, std::unordered_map<AmObjectID, EffectInstance*>> gEnvironmentFilters = {};

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
            SoundInstance* sound) override
        {
            const auto& settings = sound->GetSettings();
            if (settings.m_spatialization != Spatialization_None)
            {
                const Entity& entity = sound->GetChannel()->GetParentChannelState()->GetEntity();
                if (entity.Valid())
                {
                    std::map<AmEnvironmentID, AmReal32> environments = entity.GetEnvironments();
                    std::vector<std::pair<AmEnvironmentID, AmReal32>> items(environments.begin(), environments.end());
                    std::sort(
                        items.begin(), items.end(),
                        [](const std::pair<AmEnvironmentID, AmReal32>& a, const std::pair<AmEnvironmentID, AmReal32>& b) -> bool
                        {
                            return a.second > b.second;
                        });

                    for (auto&& environment : items)
                    {
                        if (environment.second == 0.0f)
                            continue;

                        const Environment& handle = amEngine->GetEnvironment(environment.first);
                        if (!handle.Valid())
                            continue;

                        const Effect* effect = handle.GetEffect();
                        if (gEnvironmentFilters.find(environment.first) == gEnvironmentFilters.end())
                        {
                            std::unordered_map<AmSoundID, EffectInstance*> map{};
                            gEnvironmentFilters[environment.first] = map;
                        }

                        if (gEnvironmentFilters[environment.first].find(sound->GetId()) == gEnvironmentFilters[environment.first].end())
                        {
                            gEnvironmentFilters[environment.first][sound->GetId()] = effect->CreateInstance();
                        }

                        SoundChunk* scratch = SoundChunk::CreateChunk(frames, channels, MemoryPoolKind::Amplimix);
                        std::memcpy(scratch->buffer, in, bufferSize);

                        FilterInstance* filterInstance = gEnvironmentFilters[environment.first][sound->GetId()]->GetFilter();
                        filterInstance->SetFilterParameter(0, environment.second);
                        filterInstance->Process(
                            reinterpret_cast<AmAudioSampleBuffer>(scratch->buffer), frames, bufferSize, channels, sampleRate);

                        std::memcpy(out, reinterpret_cast<AmAudioSampleBuffer>(scratch->buffer), bufferSize);
                        SoundChunk::DestroyChunk(scratch);

                        return;
                    }
                }
            }

            if (out != in)
                std::memcpy(out, in, bufferSize);
        }

        void ProcessInterleaved(
            AmAudioSampleBuffer out,
            AmConstAudioSampleBuffer in,
            AmUInt64 frames,
            AmSize bufferSize,
            AmUInt16 channels,
            AmUInt32 sampleRate,
            SoundInstance* sound) override
        {
            const auto& settings = sound->GetSettings();
            if (settings.m_spatialization != Spatialization_None)
            {
                const Entity& entity = sound->GetChannel()->GetParentChannelState()->GetEntity();
                if (entity.Valid())
                {
                    auto environments = entity.GetEnvironments();
                    std::vector<std::pair<AmEnvironmentID, AmReal32>> items(environments.begin(), environments.end());
                    std::sort(
                        items.begin(), items.end(),
                        [](const std::pair<AmEnvironmentID, AmReal32>& a, const std::pair<AmEnvironmentID, AmReal32>& b) -> bool
                        {
                            return a.second > b.second;
                        });

                    for (auto&& environment : items)
                    {
                        if (environment.second == 0.0f)
                            continue;

                        const Environment& handle = amEngine->GetEnvironment(environment.first);
                        if (!handle.Valid())
                            continue;

                        const Effect* effect = handle.GetEffect();
                        if (gEnvironmentFilters.find(environment.first) == gEnvironmentFilters.end())
                        {
                            std::unordered_map<AmSoundID, EffectInstance*> map{};
                            gEnvironmentFilters[environment.first] = map;
                        }

                        if (gEnvironmentFilters[environment.first].find(sound->GetId()) == gEnvironmentFilters[environment.first].end())
                        {
                            gEnvironmentFilters[environment.first][sound->GetId()] = effect->CreateInstance();
                        }

                        SoundChunk* scratch = SoundChunk::CreateChunk(frames, channels, MemoryPoolKind::Amplimix);
                        std::memcpy(reinterpret_cast<AmAudioSampleBuffer>(scratch->buffer), in, bufferSize);

                        FilterInstance* filterInstance = gEnvironmentFilters[environment.first][sound->GetId()]->GetFilter();
                        filterInstance->SetFilterParameter(0, environment.second);
                        filterInstance->ProcessInterleaved(
                            reinterpret_cast<AmAudioSampleBuffer>(scratch->buffer), frames, bufferSize, channels, sampleRate);

                        std::memcpy(out, reinterpret_cast<AmAudioSampleBuffer>(scratch->buffer), bufferSize);
                        SoundChunk::DestroyChunk(scratch);

                        return;
                    }
                }
            }

            if (out != in)
                std::memcpy(out, in, bufferSize);
        }

        void Cleanup(SoundInstance* sound) override
        {
            const Entity& entity = sound->GetChannel()->GetParentChannelState()->GetEntity();
            if (entity.Valid())
            {
                auto environments = entity.GetEnvironments();
                for (auto&& environment : environments)
                    if (gEnvironmentFilters.find(environment.first) != gEnvironmentFilters.end())
                        gEnvironmentFilters[environment.first].erase(sound->GetId());
            }
        }
    };

    [[maybe_unused]] static class EnvironmentProcessor final : public SoundProcessor
    {
    public:
        EnvironmentProcessor()
            : SoundProcessor("EnvironmentProcessor")
        {}

        SoundProcessorInstance* CreateInstance() override
        {
            return ampoolnew(MemoryPoolKind::Filtering, EnvironmentProcessorInstance);
        }

        void DestroyInstance(SoundProcessorInstance* instance) override
        {
            ampooldelete(MemoryPoolKind::Filtering, EnvironmentProcessorInstance, (EnvironmentProcessorInstance*)instance);
        }
    } gEnvironmentProcessor; // NOLINT(cert-err58-cpp)
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_ENVIRONMENT_PROCESSOR_H
