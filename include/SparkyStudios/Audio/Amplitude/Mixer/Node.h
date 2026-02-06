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
#include <SparkyStudios/Audio/Amplitude/DSP/Filter.h>

namespace SparkyStudios::Audio::Amplitude
{
    class AmplimixLayer;
    class PipelineInstance;

    /**
     * @brief An instance of an Amplimix pipeline node.
     *
     * This class represents the actual node executed within the
     * Amplimix pipeline. Each node instance has a unique ID assigned
     * to it, and that matches the one provided in the pipeline asset.
     *
     * @ingroup mixer
     */
    class AM_API_PUBLIC NodeInstance
    {
        friend class PipelineInstance;

    public:
        /**
         * @brief Initializes the node instance.
         *
         * @param[in] id Unique identifier for the node instance.
         * @param[in] layer The Amplimix layer this node instance is currently associated with.
         * @param[in] pipeline The pipeline this node instance belongs to.
         * @param[in] paramCount The number of parameters the node will need.
         */
        virtual void Initialize(AmObjectID id, const AmplimixLayer* layer, const PipelineInstance* pipeline, AmSize paramCount);

        /**
         * @brief Default destructor.
         */
        virtual ~NodeInstance() = default;

        /**
         * @brief Gets the unique identifier for the node instance.
         *
         * @return The unique identifier for the node instance.
         */
        [[nodiscard]] AmObjectID GetId() const;

        /**
         * @brief Gets the Amplimix layer this node instance is currently associated with.
         *
         * @return The Amplimix layer this node instance is currently associated with.
         *
         * @see AmplimixLayer
         */
        [[nodiscard]] const AmplimixLayer* GetLayer() const;

        /**
         * @brief Resets the node instance's internal state.
         *
         * @warning Amplimix should call this function automatically, each time the pipeline is
         * about to be executed. Call it manually only if you know what you're doing.
         */
        virtual void Reset() = 0;

        /**
         * @brief Gets the current value of the parameter at the given index.
         *
         * @param[in] index The index of the parameter to retrieve.
         *
         * @return The current value of the parameter.
         */
        virtual AmReal32 GetParameter(AmSize index);

        /**
         * @brief Sets the value of the parameter at the given index.
         *
         * @param[in] index The index of the parameter to retrieve.
         * @param[in] value The value to set to the parameter.
         */
        virtual void SetParameter(AmSize index, AmReal32 value);

        /**
         * @brief Configures the node with the expected input buffer dimensions.
         *
         * This method is called during the pipeline configuration phase to allow
         * nodes to pre-allocate their output buffers based on known input dimensions.
         * The configuration is cached and only re-runs when dimensions change.
         *
         * @param[in] frameCount The expected number of frames in the input buffer.
         * @param[in] channelCount The expected number of channels in the input buffer.
         */
        virtual void Configure(AmUInt64 frameCount, AmUInt16 channelCount);

        /**
         * @brief Gets the output frame count for this node.
         *
         * By default, returns the same frame count as the input. Override this method
         * if the node changes the frame count (e.g., resampling nodes).
         *
         * @return The number of frames this node will output.
         */
        [[nodiscard]] virtual AmUInt64 GetOutputFrameCount() const;

        /**
         * @brief Gets the output channel count for this node.
         *
         * By default, returns the same channel count as the input. Override this method
         * if the node changes the channel count (e.g., panning nodes that convert mono to stereo).
         *
         * @return The number of channels this node will output.
         */
        [[nodiscard]] virtual AmUInt16 GetOutputChannelCount() const;

        /**
         * @brief Determines if this node should be skipped during processing.
         *
         * Override this method to return @c true when the node cannot produce useful output
         * based on current runtime conditions (e.g., wrong spatialization mode, no listener).
         *
         * When @c true, the @c Provide() method will return @c nullptr without calling @c Consume(),
         * avoiding unnecessary upstream processing in dead paths.
         *
         * @return @c true if the node should be skipped, @c false otherwise.
         */
        [[nodiscard]] virtual bool ShouldSkip() const;

    protected:
        /**
         * @brief The expected input frame count for this node.
         */
        AmUInt64 m_inputFrameCount = 0;

        /**
         * @brief The expected input channel count for this node.
         */
        AmUInt16 m_inputChannelCount = 0;

        /**
         * @brief The unique identifier for the node instance in the pipeline.
         */
        AmObjectID m_id;

        /**
         * @brief The Amplimix layer this node instance is currently associated with.
         */
        const AmplimixLayer* m_layer;

        /**
         * @brief The pipeline this node instance belongs to.
         */
        const PipelineInstance* m_pipeline;

        /**
         * @brief The number of parameters that have changed since the last frame.
         */
        AmUInt32 m_numParamsChanged;

        /**
         * @brief The parameters buffer.
         */
        std::vector<AmReal32> m_parameters;
    };

    /**
     * @brief Interface for Amplimix pipeline nodes that can consume audio data from an input buffer.
     *
     * @ingroup mixer
     */
    class AM_API_PUBLIC ConsumerNodeInstance
    {
    public:
        /**
         * @brief Default destructor.
         */
        virtual ~ConsumerNodeInstance() = default;

        /**
         * @brief Consumes audio data from the provider node.
         *
         * @note The provider node should be specified with the call of @ref Connect "`Connect()`".
         */
        virtual void Consume() = 0;

        /**
         * @brief Sets the input provider node ID for this node.
         *
         * @param[in] provider The provider node for this node.
         */
        virtual void Connect(AmObjectID provider) = 0;
    };

    /**
     * @brief Interface for Amplimix pipeline nodes that can provide audio data to an output buffer.
     *
     * @ingroup mixer
     */
    class AM_API_PUBLIC ProviderNodeInstance
    {
    public:
        /**
         * @brief Default destructor.
         */
        virtual ~ProviderNodeInstance() = default;

        /**
         * @brief Produces audio data ready to be taken as input from a consumer node.
         *
         * @return The output audio data.
         *
         * @see AudioBuffer
         */
        virtual const AudioBuffer* Provide() = 0;
    };

    /**
     * @brief Base class for Amplimix pipeline nodes that can process audio data in-place.
     *
     * @see NodeInstance, ConsumerNodeInstance, ProviderNodeInstance
     *
     * @ingroup mixer
     */
    class AM_API_PUBLIC ProcessorNodeInstance
        : public NodeInstance
        , public ConsumerNodeInstance
        , public ProviderNodeInstance
    {
    public:
        /**
         * @brief @c ProcessorNodeInstance constructor.
         *
         * @param[in] processOnEmptyBuffer If @c true, the node will execute the @ref Process `Process()` method
         * even if the input buffer is @c nullptr.
         */
        explicit ProcessorNodeInstance(bool processOnEmptyBuffer = false);

        /**
         * @brief Default destructor.
         */
        ~ProcessorNodeInstance() override = default;

        /**
         * @brief Processes input audio data and returns the output audio data.
         *
         * @param[in] input The input audio data to process.
         *
         * @return The output audio data.
         */
        virtual const AudioBuffer* Process(const AudioBuffer* input) = 0;

        /**
         * @inherit
         */
        void Consume() final;

        /**
         * @inherit
         */
        void Connect(AmObjectID provider) final;

        /**
         * @inherit
         */
        const AudioBuffer* Provide() final;

        /**
         * @inherit
         */
        void Reset() override;

        /**
         * @brief Configures the processor node with expected input dimensions.
         *
         * This override pre-allocates the output buffer based on the configured
         * output frame count and channel count.
         *
         * @param[in] frameCount The expected number of frames in the input buffer.
         * @param[in] channelCount The expected number of channels in the input buffer.
         */
        void Configure(AmUInt64 frameCount, AmUInt16 channelCount) override;

        /**
         * @brief Returns the ID of the input provider node for this processor.
         *
         * @return The ID of the provider node.
         */
        [[nodiscard]] AM_INLINE AmObjectID GetProvider() const
        {
            return m_provider;
        }

    protected:
        /**
         * @brief The ID of the input provider node.
         */
        AmObjectID m_provider;

        /**
         * @brief The pre-allocated output buffer for this processor node.
         *
         * This buffer is allocated during the configuration phase based on the
         * output frame count and channel count returned by GetOutputFrameCount()
         * and GetOutputChannelCount().
         */
        AudioBuffer _output;

    private:
        const AudioBuffer* _processingBuffer;
        const AudioBuffer* _lastOutputBuffer;
        bool _processOnEmptyInputBuffer;
        ProviderNodeInstance* _cachedProvider;
    };

    /**
     * @brief Base class for Amplimix pipeline nodes that can mix audio data from multiple input buffers.
     *
     * @see NodeInstance, ConsumerNodeInstance, ProviderNodeInstance
     *
     * @ingroup mixer
     */
    class AM_API_PUBLIC MixerNodeInstance
        : public NodeInstance
        , public ConsumerNodeInstance
        , public ProviderNodeInstance
    {
    public:
        /**
         * @brief @c MixerNodeInstance constructor.
         */
        MixerNodeInstance();

        /**
         * @brief Default destructor.
         */
        ~MixerNodeInstance() override = default;

        /**
         * @inherit
         */
        void Consume() final;

        /**
         * @inherit
         *
         * @note This method appends the given provider to the list of input
         * provider nodes. If the provider node already exists in the list, it
         * will not be added again.
         */
        void Connect(AmObjectID provider) final;

        /**
         * @brief Sets the input provider nodes for this mixer node.
         *
         * @param[in] providers The provider nodes for this mixer node.
         *
         * @note This method clears the existing input provider nodes and replaces them with the provided ones.
         */
        void Connect(const std::vector<AmObjectID>& providers);

        /**
         * @inherit
         */
        const AudioBuffer* Provide() final;

        /**
         * @inherit
         */
        void Reset() override;

        /**
         * @brief Configures the mixer node with expected input dimensions.
         *
         * This override pre-allocates the mix buffer based on the configured
         * frame count and channel count.
         *
         * @param[in] frameCount The expected number of frames in the input buffer.
         * @param[in] channelCount The expected number of channels in the input buffer.
         */
        void Configure(AmUInt64 frameCount, AmUInt16 channelCount) override;

        /**
         * @brief Returns the IDs of the input provider nodes for this mixer.
         *
         * @return A const reference to the vector of provider node IDs.
         */
        [[nodiscard]] AM_INLINE const std::vector<AmObjectID>& GetProviders() const
        {
            return m_providers;
        }

    protected:
        /**
         * @brief The IDs of the input provider nodes.
         */
        std::vector<AmObjectID> m_providers;

        /**
         * @brief The pre-allocated mix buffer for combining inputs.
         *
         * This buffer is allocated during the configuration phase and can be
         * overridden by derived classes that need custom channel counts.
         */
        AudioBuffer _mixBuffer;

    private:
        std::vector<const AudioBuffer*> _processingBuffers;
        bool _processed;
        std::vector<ProviderNodeInstance*> _cachedProviders;
    };

    /**
     * @brief Class used to mark the input of the pipeline.
     *
     * @warning This node is automatically added to the pipeline when created. And thus
     * should not be manually added to the pipeline asset.
     *
     * @see ProviderNodeInstance
     *
     * @ingroup mixer
     */
    class AM_API_PUBLIC InputNodeInstance
        : public NodeInstance
        , public ProviderNodeInstance
    {
        friend class PipelineInstance;

    public:
        /**
         * @brief @c InputNodeInstance constructor.
         */
        InputNodeInstance();

        /**
         * @brief Default destructor.
         */
        ~InputNodeInstance() override = default;

        /**
         * @brief Sets the input of the pipeline.
         *
         * @param[in] buffer The buffer to set as the input.
         */
        void SetInput(AudioBuffer* buffer);

        /**
         * @inherit
         */
        const AudioBuffer* Provide() override;

        /**
         * @inherit
         */
        void Reset() override;

    private:
        AudioBuffer* _buffer;
        std::shared_ptr<FilterInstance> _filter;
    };

    /**
     * @brief Class used to mark the output of the pipeline.
     *
     * @warning This node is automatically added to the pipeline when created. And thus
     * should not be manually added to the pipeline asset.
     *
     * @see ConsumerNodeInstance
     *
     * @ingroup mixer
     */
    class AM_API_PUBLIC OutputNodeInstance
        : public NodeInstance
        , public ConsumerNodeInstance
    {
        friend class PipelineInstance;

    public:
        /**
         * @brief @c OutputNodeInstance constructor.
         */
        OutputNodeInstance();

        /**
         * @brief Default destructor.
         */
        ~OutputNodeInstance() override = default;

        /**
         * @brief Sets the output of the pipeline.
         *
         * @param[in] buffer The buffer to set as the output.
         */
        void SetOutput(AudioBuffer* buffer);

        /**
         * @inherit
         */
        void Consume() override;

        /**
         * @inherit
         */
        void Connect(AmObjectID provider) override;

        /**
         * @inherit
         */
        void Reset() override;

    private:
        AmObjectID _provider;
        AudioBuffer* _buffer;
        ProviderNodeInstance* _cachedProvider;
    };

    /**
     * @brief Base class for Amplimix pipeline nodes.
     *
     * This class presents the basic structure to create Amplimix pipeline nodes. Each
     * @c Node of your pipelines must be derived from this class and implement the @ref CreateInstance "`CreateInstance()`" method.
     *
     * @see NodeInstance
     *
     * @ingroup mixer
     */
    class AM_API_PUBLIC Node
    {
    public:
        /**
         * @brief Node constructor.
         *
         * @param[in] name Name of the node. Should be unique within the pipeline.
         */
        explicit Node(AmString name);

        /**
         * @brief Node destructor.
         */
        virtual ~Node();

        /**
         * @brief Creates a new instance of the node.
         *
         * @return A new instance of the node.
         */
        virtual std::shared_ptr<NodeInstance> CreateInstance() const = 0;

        /**
         * @brief Returns the name of the node.
         */
        [[nodiscard]] const AmString& GetName() const;

        /**
         * @brief Returns @c true if the node can consume audio data.
         *
         * @return @c true if the node can consume audio data, @c false otherwise.
         */
        [[nodiscard]] virtual bool CanConsume() const = 0;

        /**
         * @brief Returns @c true if the node can produce audio data.
         *
         * @return @c true if the node can produce audio data, @c false otherwise.
         */
        [[nodiscard]] virtual bool CanProduce() const = 0;

        /**
         * @brief Returns the maximum number of input connections the node can have.
         *
         * @return The maximum number of input connections the node can have.
         */
        [[nodiscard]] virtual AmSize GetMaxInputCount() const = 0;

        /**
         * @brief Returns the minimum number of input connections the node can have.
         *
         * @return The minimum number of input connections the node can have.
         */
        [[nodiscard]] virtual AmSize GetMinInputCount() const = 0;

        /**
         * @brief Returns the number of parameters accepted by this node.
         *
         * @return The number of parameters accepted by the node.
         */
        [[nodiscard]] virtual AmSize GetParameterCount() const;

        /**
         * @brief Gets the name of the parameter at the given index.
         *
         * @param[in] index The parameter index.
         *
         * @return The name of the parameter at the given index.
         */
        [[nodiscard]] virtual AmString GetParameterName(AmSize index) const;

        /**
         * @brief Gets the type of the parameter at the given index.
         *
         * @param[in] index The parameter index.
         *
         * @return The type of the parameter at the given index.
         */
        [[nodiscard]] virtual eParameterType GetParameterType(AmSize index) const;

        /**
         * @brief Gets the maximum allowed value of the parameter at the given index.
         *
         * @param[in] index The parameter index.
         *
         * @return The maximum allowed value of the parameter at the given index.
         */
        [[nodiscard]] virtual AmReal32 GetParameterMax(AmSize index) const;

        /**
         * @brief Gets the minimum allowed value of the parameter at the given index.
         *
         * @param[in] index The parameter index.
         *
         * @return The minimum allowed value of the parameter at the given index.
         */
        [[nodiscard]] virtual AmReal32 GetParameterMin(AmSize index) const;

        /**
         * @brief Registers a new node.
         *
         * @param[in] node The node to add in the registry.
         */
        static void Register(std::shared_ptr<Node> node);

        /**
         * @brief Unregisters a node.
         *
         * @param[in] node The node to remove from the registry.
         */
        static void Unregister(std::shared_ptr<const Node> node);

        /**
         * @brief Look up a node by name.
         *
         * @return The node with the given name, or @c nullptr if not found.
         */
        static std::shared_ptr<Node> Find(const AmString& name);

        /**
         * @brief Creates a new instance of the node with the given name and returns its pointer.
         *
         * @param[in] name The name of the node.
         *
         * @return The node with the given name, or @c nullptr if not found.
         */
        static std::shared_ptr<NodeInstance> Construct(const AmString& name);

        /**
         * @brief Locks the nodes' registry.
         *
         * @warning This function is mainly used for internal purposes. It's
         * called before the @c Engine initialization, to discard the registration
         * of new nodes after the engine is fully loaded.
         */
        static void LockRegistry();

        /**
         * @brief Unlocks the nodes' registry.
         *
         * @warning This function is mainly used for internal purposes. It's
         * called after the @c Engine deinitialization, to allow the registration
         * of new nodes after the engine is fully unloaded.
         */
        static void UnlockRegistry();

        /**
         * @brief Gets the list of registered nodes.
         *
         * @return The registry of nodes.
         */
        static const std::map<AmString, std::shared_ptr<Node>>& GetRegistry();

    protected:
        /**
         * @brief The name of this node.
         */
        AmString m_name;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_MIXER_NODE_H
