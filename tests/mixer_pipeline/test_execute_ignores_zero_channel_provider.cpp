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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Node.h>

#include <Mixer/Pipeline.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    /**
     * @brief Processor node whose output buffer has frames but no channels.
     */
    class ZeroChannelProviderNodeInstance final : public ProcessorNodeInstance
    {
    public:
        ZeroChannelProviderNodeInstance()
            : ProcessorNodeInstance(false)
        {}

        [[nodiscard]] AmUInt16 GetOutputChannelCount() const override
        {
            return 0;
        }

        const AudioBuffer* Process(const AudioBuffer*) override
        {
            return &_output;
        }
    };

    class ZeroChannelProviderNode final : public Node
    {
    public:
        ZeroChannelProviderNode()
            : Node("ZeroChannelProvider")
        {}

        [[nodiscard]] AM_INLINE std::shared_ptr<NodeInstance> CreateInstance() const override
        {
            return ampoolshared(eMemoryPoolKind_Amplimix, ZeroChannelProviderNodeInstance);
        }

        [[nodiscard]] AM_INLINE bool CanConsume() const override
        {
            return true;
        }

        [[nodiscard]] AM_INLINE bool CanProduce() const override
        {
            return true;
        }

        [[nodiscard]] AM_INLINE AmSize GetMaxInputCount() const override
        {
            return 1;
        }

        [[nodiscard]] AM_INLINE AmSize GetMinInputCount() const override
        {
            return 1;
        }
    };

    /**
     * @brief A provider that yields a buffer without channels must leave the caller's
     * output buffer untouched instead of resizing it or reading past its channels.
     *
     * The node registry is locked while the engine is initialized, so the test node is
     * registered before the base fixture starts the engine and unregistered after it
     * stops it.  The node lives outside the engine memory pools because the memory
     * manager is not available at either of those points.
     */
    AM_TEST_CASE(EngineTestCase, mixer_pipeline, execute_ignores_zero_channel_provider)
    {
    public:
        void SetUp() override
        {
            _zeroChannelProviderNode = std::make_shared<ZeroChannelProviderNode>();
            Node::Register(_zeroChannelProviderNode);
            EngineTestCase::SetUp();
        }

        void TearDown() override
        {
            EngineTestCase::TearDown();
            Node::Unregister(_zeroChannelProviderNode);
            _zeroChannelProviderNode.reset();
        }

        void Run() override
        {
            constexpr AmUInt64 frameCount = 256;
            constexpr AmUInt16 monoChannels = 1;
            constexpr AmUInt16 stereoChannels = 2;
            constexpr AmReal32 sentinel = 0.25f;

            AmplimixLayerImpl layer;

            PipelineImpl pipeline;
            pipeline.LoadDefinitionFromFile(
                _fileSystem->OpenFile(
                    _fileSystem->Join({ AM_OS_STRING("pipelines"), AM_OS_STRING("tests.zero_channel_provider.ampipeline") }),
                    eFileOpenMode_Read),
                nullptr);

            auto instance = pipeline.CreateInstance(&layer);
            AM_EXPECT_NOT(instance == nullptr);
            if (instance == nullptr)
                return;

            AudioBuffer input(frameCount, monoChannels);
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = 1.0f;

            AudioBuffer output(frameCount, stereoChannels);
            for (AmUInt64 i = 0; i < frameCount; ++i)
            {
                output[0][i] = sentinel;
                output[1][i] = sentinel;
            }

            instance->Execute(input, output);

            AM_EXPECT_EQ(output.GetChannelCount(), stereoChannels);
            AM_EXPECT_EQ(output.GetFrameCount(), frameCount);
            if (output.GetChannelCount() != stereoChannels || output.GetFrameCount() != frameCount)
                return;

            for (AmUInt64 i = 0; i < frameCount; ++i)
            {
                AM_EXPECT_EQ(output[0][i], sentinel);
                AM_EXPECT_EQ(output[1][i], sentinel);
            }
        }

    private:
        std::shared_ptr<ZeroChannelProviderNode> _zeroChannelProviderNode = nullptr;
    };

    AM_REGISTER_TEST(mixer_pipeline, execute_ignores_zero_channel_provider);
} // namespace SparkyStudios::Audio::Amplitude::Tests
