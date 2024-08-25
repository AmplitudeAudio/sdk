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

#ifndef _AM_MIXER_NODE_H
#define _AM_MIXER_NODE_H

#include <SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h>
#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    class AmplimixLayer;
    class Pipeline;

    /**
     * @brief An instance of an Amplimix pipeline node.
     *
     * This class represents the actual node executed within the
     * Amplimix pipeline. Each node instance has a unique ID assigned
     * to it, that matches the one provided in the pipeline asset.
     */
    class AM_API_PUBLIC NodeInstance
    {
    public:
        /**
         * @brief NodeInstance constructor.
         *
         * @param id Unique identifier for the node instance.
         */
        NodeInstance(AmObjectID id);

        /**
         * @brief NodeInstance destructor.
         */
        virtual ~NodeInstance() = default;

        /**
         * @brief Returns the unique identifier for the node instance.
         */
        [[nodiscard]] AmObjectID GetId() const;

    protected:
        AmObjectID m_id; /// The unique identifier for the node instance in the pipeline.
    };

    /**
     * @brief Interface for Amplimix pipeline nodes that can consume
     * audio data from an input buffer.
     */
    class AM_API_PUBLIC ConsumerNodeInstance
    {
    public:
        /**
         * @brief Consumes audio data from the provider node
         * specified with the call of @c ConsumerNodeInstance::Connect().
         */
        virtual void Consume() = 0;

        /**
         * @brief Set the input provider node ID for this node.
         *
         * @param provider The provider node for this node.
         */
        virtual void Connect(AmObjectID provider) = 0;
    };

    /**
     * @brief Interface for Amplimix pipeline nodes that can provide
     * audio data to an output buffer.
     */
    class AM_API_PUBLIC ProviderNodeInstance
    {
    public:
        /**
         * @brief Produces audio data ready to be taken as input of
         * a consumer node.
         *
         * @return The output audio data.
         */
        virtual AudioBuffer Provide() = 0;
    };

    /**
     * @brief Base class for Amplimix pipeline nodes that can process
     * audio data in-place.
     */
    class AM_API_PUBLIC ProcessorNodeInstance
        : public NodeInstance
        , public ConsumerNodeInstance
        , public ProviderNodeInstance
    {
    public:
        /**
         * @brief ProcessorNodeInstance constructor.
         */
        ProcessorNodeInstance(AmObjectID id, Pipeline* pipeline);

        /**
         * @brief ProcessorNodeInstance destructor.
         */
        ~ProcessorNodeInstance() override = default;

        /**
         * @brief Process input audio data and returns the output audio data.
         *
         * @param input The input audio data to process.
         *
         * @return The output audio data.
         */
        virtual AudioBuffer Process(const AudioBuffer& input) = 0;

        /**
         * @copydoc ConsumerNodeInstance::Consume()
         */
        void Consume() final;

        /**
         * @copydoc ConsumerNodeInstance::Connect()
         */
        void Connect(AmObjectID provider) final;

        /**
         * @copydoc ProviderNodeInstance::Provide()
         */
        AudioBuffer Provide() final;

    protected:
        AmObjectID m_provider;
        Pipeline* m_pipeline; ///< The pipeline this node instance belongs to.

    private:
        AudioBuffer _processingBuffer;
    };

    /**
     * @brief Base class for Amplimix pipeline nodes that can mix
     * audio data from multiple input buffers, and outputs the result
     * of the mix.
     */
    class AM_API_PUBLIC MixerNodeInstance
        : public NodeInstance
        , public ConsumerNodeInstance
        , public ProviderNodeInstance
    {
    public:
        /**
         * @brief MixerNodeInstance constructor.
         */
        MixerNodeInstance(AmObjectID id, Pipeline* pipeline);

        /**
         * @brief MixerNodeInstance destructor.
         */
        ~MixerNodeInstance() override = default;

        /**
         * @brief Mixes audio data from multiple inputs, and outputs the result
         * of the mix.
         *
         * @param inputs The list of input audio data to mix.
         *
         * @return The output audio data.
         */
        virtual AudioBuffer Mix(const std::vector<AudioBuffer>& inputs) = 0;

        /**
         * @copydoc ConsumerNodeInstance::Consume()
         */
        void Consume() final;

        /**
         * @copydoc ConsumerNodeInstance::Connect()
         *
         * @note This method appends the given provider to the list of input
         * provider nodes. If the provider node already exists in the list, it
         * will not be added again.
         */
        void Connect(AmObjectID provider) final;

        /**
         * @brief Set the input provider nodes for this mixer node.
         *
         * @param providers The provider nodes for this mixer node.
         *
         * @note This method clears the existing input provider nodes,
         * and replaces them with the provided ones.
         */
        void In(const std::vector<AmObjectID>& providers);

        /**
         * @copydoc ProviderNodeInstance::Provide()
         */
        AudioBuffer Provide() final;

    protected:
        std::vector<AmObjectID> m_providers;
        Pipeline* m_pipeline; ///< The pipeline this node instance belongs to.

    private:
        std::vector<AudioBuffer> _processingBuffers;
    };

    class AM_API_PUBLIC InputNodeInstance final
        : public NodeInstance
        , public ProviderNodeInstance
    {
        friend class Pipeline;

        /**
         * @brief Class used to marks the input of the pipeline.
         *
         * This node is automatically added to the pipeline when created. And thus
         * should not be manually added to the pipeline asset.
         */
    public:
        /**
         * @brief InputNodeInstance constructor.
         */
        InputNodeInstance(AmObjectID id, Pipeline* pipeline);

        /**
         * @brief InputNodeInstance destructor.
         */
        ~InputNodeInstance() override = default;

        /**
         * @brief Set the input of the pipeline.
         *
         * @param layer The layer to which this input belongs.
         * @param buffer The buffer to set as the input.
         */
        void SetInput(const AmplimixLayer* layer, const AudioBuffer* buffer);

        /**
         * @copydoc ProviderNodeInstance::Provide()
         */
        AudioBuffer Provide() final;

    private:
        const AmplimixLayer* _layer;
        const AudioBuffer* _buffer;
    };

    /**
     * @brief Class used to marks the output of the pipeline.
     *
     * This node is automatically added to the pipeline when created. And thus
     * should not be manually added to the pipeline asset.
     */
    class AM_API_PUBLIC OutputNodeInstance final
        : public NodeInstance
        , public ConsumerNodeInstance
    {
        friend class Pipeline;

    public:
        /**
         * @brief OutputNodeInstance constructor.
         */
        OutputNodeInstance(AmObjectID id, Pipeline* pipeline);

        /**
         * @brief OutputNodeInstance destructor.
         */
        ~OutputNodeInstance() override = default;

        /**
         * @brief Set the output of the pipeline.
         *
         * @param buffer The buffer to set as the output.
         */
        void SetOutput(AudioBuffer* buffer);

        /**
         * @copydoc ConsumerNodeInstance::Consume()
         */
        void Consume() final;

        /**
         * @copydoc ConsumerNodeInstance::Connect()
         */
        void Connect(AmObjectID provider) final;

    private:
        AmObjectID _provider;
        Pipeline* _pipeline; ///< The pipeline this node instance belongs to.
        AudioBuffer* _buffer;
    };

    /**
     * @brief Base class for Amplimix pipeline nodes.
     *
     * This class presents the basic structure to create Amplimix
     * pipeline nodes. Each @c Node must be derived from this class
     * and implement the @c Node::CreateInstance() and
     * @c Node::DestroyInstance() methods.
     */
    class AM_API_PUBLIC Node
    {
    public:
        /**
         * @brief Node constructor.
         *
         * @param name Name of the node. Should be unique within the pipeline.
         */
        Node(AmString name);

        /**
         * @brief Node destructor.
         */
        virtual ~Node() = default;

        /**
         * @brief Returns the name of the node.
         */
        [[nodiscard]] const AmString& GetName() const;

        /**
         * @brief Creates a new instance of the node.
         *
         * @param id Unique identifier for the new instance.
         *
         * @return A new instance of the node.
         */
        virtual NodeInstance* CreateInstance(AmObjectID id) const = 0;

        /**
         * @brief Destroys the specified instance of the node.
         *
         * @param instance Pointer to the instance to be destroyed.
         */
        virtual void DestroyInstance(NodeInstance* instance) const = 0;

    private:
        AmString _name;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_MIXER_NODE_H