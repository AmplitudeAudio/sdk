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
     * manages its own state, and create a set of node instances following the provided configuration.
     *
     * @ingroup mixer
     */
    class AM_API_PUBLIC PipelineInstance
    {
        friend class Pipeline;

    public:
        /**
         * @brief Default constructor.
         */
        virtual ~PipelineInstance() = default;

        /**
         * @brief Executes the pipeline for the given layer.
         *
         * @param[in] in The input buffer to process. This buffer is passed to the input
         * node of the pipeline.
         * @param[out] out The output buffer where the output node will fill processed data.
         */
        virtual void Execute(const AudioBuffer& in, AudioBuffer& out) = 0;

        /**
         * @brief Resets the internal state for all nodes in the pipeline.
         *
         * @warning This method is called automatically when Amplimix has finished processing a frame
         * for a specific layer. You should not manually call this method, unless you know what
         * you're doing.
         */
        virtual void Reset() = 0;

        /**
         * @brief Gets the node with the specified ID.
         *
         * @param[in] id The ID of the node to retrieve.
         *
         * @return The node with the specified ID, or `nullptr` if not found.
         */
        [[nodiscard]] virtual NodeInstance* GetNode(AmObjectID id) const = 0;
    };

    /**
     * @brief A pipeline assembles a set of nodes to process audio data.
     *
     * For each layer in `Amplimix`, a `PipelineInstance` is created for that specific layer.
     *
     * @see [Amplimix](../Amplimix/index.md), [PipelineInstance](../PipelineInstance/index.md)
     *
     * @ingroup mixer
     */
    class AM_API_PUBLIC Pipeline : public Asset<AmPipelineID>
    {
    public:
        /**
         * @brief Default destructor.
         */
        virtual ~Pipeline() = default;

        /**
         * @brief Creates a new pipeline instance for the specified layer.
         *
         * @param[in] layer The layer for which to create the pipeline instance.
         *
         * @return A new pipeline instance for the specified layer.
         */
        virtual PipelineInstance* CreateInstance(const AmplimixLayer* layer) const = 0;

        /**
         * @brief Destroys the specified pipeline instance.
         *
         * @param[in] instance The pipeline instance to destroy.
         */
        virtual void DestroyInstance(PipelineInstance* instance) const = 0;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_MIXER_PIPELINE_H
