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
#include <SparkyStudios/Audio/Amplitude/DSP/Reverb.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    class MockReverbInstance final : public ReverbInstance
    {
    public:
        explicit MockReverbInstance(Reverb* parent)
            : ReverbInstance(parent)
        {}

        void Initialize(AmUInt32 sampleRate) override
        {}

        void Reset() override
        {}

        void SetRoomParameters(AmReal32 roomSize, AmReal32 absorption) override
        {}

        void Process(const AudioBuffer& in, AudioBuffer& out, AmUInt64 frames, AmUInt32 sampleRate) override
        {}
    };

    class MockReverb final : public Reverb
    {
    public:
        MockReverb()
            : Reverb("MockReverb")
        {}

        std::shared_ptr<ReverbInstance> CreateInstance() override
        {
            return ampoolshared(eMemoryPoolKind_Filtering, MockReverbInstance, this);
        }
    };

    AM_TEST_CASE(DSPTestCase, dsp_reverbs, reverb_registry)
    {
    public:
        void Run() override
        {
            auto mock = amshared(MockReverb);
            Reverb::Register(mock);

            auto found = Reverb::Find("MockReverb");
            AM_EXPECT_NOT(found == nullptr);
            AM_EXPECT_EQ(found->GetName(), "MockReverb");

            const size_t count = Reverb::GetRegistry().size();
            Reverb::Register(mock);
            AM_EXPECT_EQ(Reverb::GetRegistry().size(), count);

            auto instance = found->CreateInstance();
            AM_EXPECT_NOT(instance == nullptr);
            AM_EXPECT_EQ(instance->GetParent(), found.get());

            Reverb::LockRegistry();
            Reverb::Unregister(mock);
            AM_EXPECT_NOT(Reverb::Find("MockReverb") == nullptr);
            Reverb::UnlockRegistry();

            Reverb::Unregister(mock);
            AM_EXPECT(Reverb::Find("MockReverb") == nullptr);
        }
    };

    AM_REGISTER_TEST(dsp_reverbs, reverb_registry);
} // namespace SparkyStudios::Audio::Amplitude::Tests
