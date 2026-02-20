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

#pragma once

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Node.h>

#include <Core/ListenerInternalState.h>

#include "SimpleTestCase.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    class MinimalMockLayer : public AmplimixLayer
    {
    public:
        MinimalMockLayer()
            : _id(0)
            , _sampleRate(48000)
        {}

        void SetId(AmUInt32 id) { _id = id; }
        void SetSampleRate(AmUInt32 sampleRate) { _sampleRate = sampleRate; }
        void SetSoundFormat(const SoundFormat& format) { _soundFormat = format; }

        AmUInt32 GetId() const override { return _id; }
        AmUInt32 GetSampleRate() const override { return _sampleRate; }
        SoundFormat GetSoundFormat() const override { return _soundFormat; }

        AmUInt64 GetStartPosition() const override { return 0; }
        AmUInt64 GetEndPosition() const override { return 0; }
        AmUInt64 GetCurrentPosition() const override { return 0; }
        AmReal32 GetGain() const override { return 1.0f; }
        AmReal32 GetPitch() const override { return 1.0f; }
        AmReal32 GetPlaySpeed() const override { return 1.0f; }
        AmReal32 GetObstruction() const override { return 0.0f; }
        AmReal32 GetOcclusion() const override { return 0.0f; }
        AmVector3 GetLocation() const override { return {}; }
        Entity GetEntity() const override { return {}; }
        Listener GetListener() const override { return {}; }
        Room GetRoom() const override { return {}; }
        Channel GetChannel() const override { return {}; }
        Bus GetBus() const override { return {}; }
        eSpatialization GetSpatialization() const override { return eSpatialization_None; }
        bool IsLoopEnabled() const override { return false; }
        bool IsStreamEnabled() const override { return false; }
        const Sound* GetSound() const override { return nullptr; }
        const std::shared_ptr<EffectInstance> GetEffect() const override { return nullptr; }
        const Attenuation* GetAttenuation() const override { return nullptr; }
        bool IsMultiPosition() const override { return false; }
        eChannelInstanceMode GetInstancingMode() const override { return eChannelInstanceMode_Separate; }
        AmSize GetInstanceCount() const override { return 0; }
        AmVector3 GetInstanceLocation(AmSize) const override { return {}; }
        Room GetInstanceRoom(AmSize) const override { return {}; }
        AmReal32 GetInstanceWeight(AmSize) const override { return 0.0f; }
        AmReal32 GetInstanceGain(AmSize) const override { return 0.0f; }

    private:
        AmUInt32 _id;
        AmUInt32 _sampleRate;
        SoundFormat _soundFormat;
    };

    class SpatialMockLayer : public MinimalMockLayer
    {
    public:
        SpatialMockLayer()
            : _spatialization(eSpatialization_None)
            , _location{}
            , _obstruction(0.0f)
            , _occlusion(0.0f)
            , _multiPosition(false)
        {}

        void SetSpatialization(eSpatialization spat) { _spatialization = spat; }
        void SetLocation(const AmVector3& loc) { _location = loc; }
        void SetListener(const Listener& listener) { _listener = listener; }
        void SetEntity(const Entity& entity) { _entity = entity; }
        void SetRoom(const Room& room) { _room = room; }
        void SetChannel(const Channel& channel) { _channel = channel; }
        void SetSound(const Sound* sound) { _sound = sound; }
        void SetAttenuation(const Attenuation* attenuation) { _attenuation = attenuation; }
        void SetObstruction(AmReal32 obstruction) { _obstruction = obstruction; }
        void SetOcclusion(AmReal32 occlusion) { _occlusion = occlusion; }

        void SetMultiPosition(bool enabled) { _multiPosition = enabled; }
        void AddInstance(const AmVector3& location, AmReal32 weight, AmReal32 gain)
        {
            _instanceLocations.push_back(location);
            _instanceWeights.push_back(weight);
            _instanceGains.push_back(gain);
        }
        void ClearInstances()
        {
            _instanceLocations.clear();
            _instanceWeights.clear();
            _instanceGains.clear();
        }

        eSpatialization GetSpatialization() const override { return _spatialization; }
        AmVector3 GetLocation() const override { return _location; }
        Listener GetListener() const override { return _listener; }
        Entity GetEntity() const override { return _entity; }
        Room GetRoom() const override { return _room; }
        Channel GetChannel() const override { return _channel; }
        const Sound* GetSound() const override { return _sound; }
        const Attenuation* GetAttenuation() const override { return _attenuation; }
        AmReal32 GetObstruction() const override { return _obstruction; }
        AmReal32 GetOcclusion() const override { return _occlusion; }
        bool IsMultiPosition() const override { return _multiPosition; }
        AmSize GetInstanceCount() const override { return _instanceLocations.size(); }
        AmVector3 GetInstanceLocation(AmSize i) const override { return _instanceLocations[i]; }
        AmReal32 GetInstanceWeight(AmSize i) const override { return _instanceWeights[i]; }
        AmReal32 GetInstanceGain(AmSize i) const override { return _instanceGains[i]; }

    private:
        eSpatialization _spatialization;
        AmVector3 _location;
        Listener _listener;
        Entity _entity;
        Room _room;
        Channel _channel;
        const Sound* _sound = nullptr;
        const Attenuation* _attenuation = nullptr;
        AmReal32 _obstruction;
        AmReal32 _occlusion;
        bool _multiPosition;
        std::vector<AmVector3> _instanceLocations;
        std::vector<AmReal32> _instanceWeights;
        std::vector<AmReal32> _instanceGains;
    };

    class NodeTestCase : public SimpleTestCase
    {
    public:
        void SetUp() override
        {
            SimpleTestCase::SetUp();
            _mockLayer = std::make_unique<SpatialMockLayer>();
        }

        void TearDown() override
        {
            _mockLayer.reset();
            SimpleTestCase::TearDown();
        }

    protected:
        SpatialMockLayer& GetMockLayer() { return *_mockLayer; }

        template<typename NodeType>
        std::shared_ptr<NodeInstance> CreateAndConfigureNode(AmUInt64 frameCount, AmUInt16 channelCount)
        {
            NodeType node;
            auto instance = node.CreateInstance();
            instance->Initialize(1, _mockLayer.get(), nullptr, node.GetParameterCount());
            instance->Configure(frameCount, channelCount);
            return instance;
        }

        ProcessorNodeInstance* AsProcessor(const std::shared_ptr<NodeInstance>& instance)
        {
            return dynamic_cast<ProcessorNodeInstance*>(instance.get());
        }

    private:
        std::unique_ptr<SpatialMockLayer> _mockLayer;
    };
} // namespace SparkyStudios::Audio::Amplitude::Tests
