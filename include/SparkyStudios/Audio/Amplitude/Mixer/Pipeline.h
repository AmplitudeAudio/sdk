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

#ifndef _AM_MIXER_PIPELINE_H
#define _AM_MIXER_PIPELINE_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Amplimix.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Node.h>

namespace SparkyStudios::Audio::Amplitude
{
    class Pipeline;

    /**
     * @brief Represents an instance of a pipeline for a specific layer.
     *
     * A pipeline instance is created for each single layer in the mixer. Each pipeline instance
     * manages its own state and creates a set of node instances following the provided configuration.
     *
     * @ingroup mixer
     */
    class AM_API_PUBLIC PipelineInstance
    {
        friend class Pipeline;

    public:
        /**
         * @brief Default destructor.
         */
        virtual ~PipelineInstance() = default;

        /**
         * @brief Executes the pipeline for the given layer.
         *
         * @param[in,out] in The input buffer to process. This buffer may be modified in-place
         * by the input node's filter. It is passed to the input node of the pipeline.
         * @param[out] out The output buffer where the output node will fill processed data.
         */
        virtual void Execute(AudioBuffer& in, AudioBuffer& out) = 0;

        /**
         * @brief Resets the internal state for all nodes in the pipeline.
         *
         * @warning This method is called automatically when Amplimix has finished processing a frame
         * for a specific layer. __You should not manually call this method unless you know what
         * you're doing__.
         */
        virtual void Reset() = 0;

        /**
         * @brief Gets the node with the specified ID.
         *
         * @param[in] id The ID of the node to retrieve.
         *
         * @return The node with the specified ID, or @c nullptr if not found.
         */
        [[nodiscard]] virtual std::shared_ptr<NodeInstance> GetNode(AmObjectID id) const = 0;

        /**
         * @brief Configures the pipeline with the expected input and output buffer dimensions.
         *
         * This method is called automatically by Execute() when buffer dimensions change.
         * It walks through the node graph and calls Configure() on each node, allowing
         * them to pre-allocate their output buffers.
         *
         * The configuration is cached and only re-runs when dimensions change, avoiding
         * unnecessary allocations on every frame.
         *
         * @param[in] inputFrameCount The expected number of frames in the input buffer.
         * @param[in] inputChannelCount The expected number of channels in the input buffer.
         * @param[in] outputFrameCount The expected number of frames in the output buffer.
         * @param[in] outputChannelCount The expected number of channels in the output buffer.
         */
        virtual void Configure(
            AmUInt64 inputFrameCount, AmUInt16 inputChannelCount, AmUInt64 outputFrameCount, AmUInt16 outputChannelCount) = 0;
    };

    /**
     * @brief Assembles a set of nodes to process audio data.
     *
     * For each layer in @c Amplimix, a @c PipelineInstance is created for that specific layer.
     *
     * @see Amplimix, PipelineInstance
     *
     * @ingroup mixer
     */
    class AM_API_PUBLIC Pipeline : public Asset<AmPipelineID>
    {
    public:
        /**
         * @brief Default destructor.
         */
        ~Pipeline() override = default;

        /**
         * @brief Creates a new pipeline instance for the specified layer.
         *
         * @param[in] layer The layer for which to create the pipeline instance.
         *
         * @return A new pipeline instance for the specified layer.
         */
        virtual std::shared_ptr<PipelineInstance> CreateInstance(const AmplimixLayer* layer) const = 0;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_MIXER_PIPELINE_H
