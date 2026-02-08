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

#pragma once

#ifndef _AM_IMPLEMENTATION_MIXER_PIPELINE_H
#define _AM_IMPLEMENTATION_MIXER_PIPELINE_H

#include <SparkyStudios/Audio/Amplitude/Mixer/Pipeline.h>

#include <Core/Asset.h>
#include <Mixer/Amplimix.h>

#include "pipeline_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    class PipelineInstanceImpl final : public PipelineInstance
    {
        friend class PipelineImpl;

    public:
        PipelineInstanceImpl(const Pipeline* parent, const AmplimixLayerImpl* layer);

        ~PipelineInstanceImpl() override;

        void Execute(const AudioBuffer& inputBuffer, AudioBuffer& outputBuffer) override;

        std::shared_ptr<NodeInstance> GetNode(AmObjectID id) const override;

        void Reset() override;

        void Configure(
            AmUInt64 inputFrameCount, AmUInt16 inputChannelCount, AmUInt64 outputFrameCount, AmUInt16 outputChannelCount) override;

        void AddNode(AmObjectID id, AmString nodeName, std::shared_ptr<NodeInstance> nodeInstance);

    private:
        /**
         * @brief Checks if the pipeline needs reconfiguration based on new dimensions.
         */
        [[nodiscard]] bool NeedsReconfiguration(AmUInt64 inFrames, AmUInt16 inChannels, AmUInt64 outFrames, AmUInt16 outChannels) const;

        /**
         * @brief Walks the node graph and configures each node with appropriate dimensions.
         */
        void ConfigureNodeGraph();

        std::unordered_map<AmObjectID, std::pair<AmString, std::shared_ptr<NodeInstance>>> _nodeInstances;

        std::shared_ptr<InputNodeInstance> _inputNode;
        std::shared_ptr<OutputNodeInstance> _outputNode;

        const AmplimixLayerImpl* _layer;

        // Configuration caching state
        AmUInt64 _configuredInputFrameCount = 0;
        AmUInt16 _configuredInputChannelCount = 0;
        AmUInt64 _configuredOutputFrameCount = 0;
        AmUInt16 _configuredOutputChannelCount = 0;
        bool _isConfigured = false;
    };

    class PipelineImpl final
        : public Pipeline
        , public AssetImpl<AmPipelineID, PipelineDefinition>
    {
    public:
        ~PipelineImpl() override;

        std::shared_ptr<PipelineInstance> CreateInstance(const AmplimixLayer* layer) const override;

        /**
         * @copydoc Asset::GetId
         */
        [[nodiscard]] AM_INLINE AmEventID GetId() const override
        {
            return AssetImpl::GetId();
        }

        /**
         * @copydoc Asset::GetName
         */
        [[nodiscard]] AM_INLINE const AmString& GetName() const override
        {
            return AssetImpl::GetName();
        }

        /**
         * @copydoc AssetImpl::LoadDefinition
         */
        bool LoadDefinition(const PipelineDefinition* definition, std::shared_ptr<EngineInternalState> state) override;

        /**
         * @copydoc AssetImpl::GetDefinition
         */
        [[nodiscard]] const PipelineDefinition* GetDefinition() const override;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_PIPELINE_H
