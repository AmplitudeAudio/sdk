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

#include <optional>

#include <Core/ListenerInternalState.h>

#include "ComponentTestCase.h"

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
        const Orientation& GetOrientation() const override { return _zeroOrientation; }
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
        AmReal32 GetInstanceAttenuationGain(AmSize) const override { return 1.0f; }

    protected:
        const Orientation _zeroOrientation = Orientation::Zero();

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

        /// Sets the orientation of the processed instance. Without one, the layer follows its entity's orientation.
        void SetOrientation(const Orientation& orientation) { _orientation = orientation; }
        void SetInstanceOrientation(AmSize index, const Orientation& orientation) { _instanceOrientations[index] = orientation; }

        void SetMultiPosition(bool enabled) { _multiPosition = enabled; }
        void SetSharingPipelineAcrossInstances(bool v) { _sharingPipelineAcrossInstances = v; }
        void AddInstance(const AmVector3& location, AmReal32 weight, AmReal32 gain)
        {
            _instanceLocations.push_back(location);
            _instanceWeights.push_back(weight);
            _instanceGains.push_back(gain);
            _instanceOrientations.emplace_back(std::nullopt);
        }
        void ClearInstances()
        {
            _instanceLocations.clear();
            _instanceWeights.clear();
            _instanceGains.clear();
            _instanceOrientations.clear();
        }

        eSpatialization GetSpatialization() const override { return _spatialization; }
        AmVector3 GetLocation() const override { return _location; }
        const Orientation& GetOrientation() const override { return Followed(_orientation); }
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
        bool HasInstanceOrientation(AmSize i) const override { return _instanceOrientations[i].has_value(); }
        const Orientation& GetInstanceOrientation(AmSize i) const override { return Followed(_instanceOrientations[i]); }
        AmReal32 GetInstanceWeight(AmSize i) const override { return _instanceWeights[i]; }
        AmReal32 GetInstanceGain(AmSize i) const override { return _instanceGains[i]; }
        AmReal32 GetInstanceAttenuationGain(AmSize i) const override
        {
            if (_attenuation == nullptr || !_listener.Valid())
                return 1.0f;

            return _instanceOrientations[i].has_value() ? _attenuation->GetGain(_instanceLocations[i], *_instanceOrientations[i], _listener)
                                                        : _attenuation->GetGain(_instanceLocations[i], _listener);
        }
        bool IsSharingPipelineAcrossInstances() const override { return _sharingPipelineAcrossInstances; }

    private:
        // The explicit orientation when there is one, otherwise the entity's, otherwise zero.
        const Orientation& Followed(const std::optional<Orientation>& explicitOrientation) const
        {
            if (explicitOrientation.has_value())
                return *explicitOrientation;

            return _entity.Valid() ? _entity.GetOrientation() : _zeroOrientation;
        }

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
        bool _sharingPipelineAcrossInstances = false;
        std::vector<AmVector3> _instanceLocations;
        std::vector<AmReal32> _instanceWeights;
        std::vector<AmReal32> _instanceGains;
        std::optional<Orientation> _orientation;
        std::vector<std::optional<Orientation>> _instanceOrientations;
    };

    /**
     * @brief Simple mock attenuation for testing AttenuationNode without engine.
     *
     * Implements a linear distance-based gain falloff:
     *   gain = max(0, 1 - distance / maxDistance)
     */
    class MockAttenuation : public Attenuation
    {
    public:
        explicit MockAttenuation(AmReal64 maxDistance = 100.0)
            : _maxDistance(maxDistance)
        {}

        [[nodiscard]] AmAttenuationID GetId() const override { return 1; }
        [[nodiscard]] const AmString& GetName() const override { static AmString name = "MockAttenuation"; return name; }

        [[nodiscard]] AmReal32 GetGain(const AmVector3& soundLocation, const Listener& listener) const override
        {
            const AmReal32 distance = Length(Sub(soundLocation, listener.GetLocation()));
            return std::max(0.0f, 1.0f - distance / static_cast<AmReal32>(_maxDistance));
        }

        [[nodiscard]] AmReal32 GetGain(const Entity& entity, const Listener& listener) const override
        {
            return GetGain(entity.GetLocation(), listener);
        }

        [[nodiscard]] AmReal32 GetGain(
            const AmVector3& soundLocation, const Orientation& soundOrientation, const Listener& listener) const override
        {
            _orientedGainCount++;
            _lastOrientation = soundOrientation;

            return GetGain(soundLocation, listener);
        }

        /// How many times the orientation overload was evaluated.
        [[nodiscard]] AmUInt32 GetOrientedGainCount() const { return _orientedGainCount; }

        /// The orientation the overload was last evaluated with.
        [[nodiscard]] const std::optional<Orientation>& GetLastOrientation() const { return _lastOrientation; }

        [[nodiscard]] AttenuationZone* GetShape() const override { return nullptr; }
        [[nodiscard]] const Curve& GetGainCurve() const override { static Curve c; return c; }

        [[nodiscard]] AmReal32 GetSpreadFactor(AmReal64) const override
        {
            return _spreadFactor;
        }

        void SetSpreadFactor(AmReal32 factor)
        {
            _spreadFactor = factor;
        }

        [[nodiscard]] AmReal64 GetMaxDistance() const override { return _maxDistance; }
        [[nodiscard]] bool IsAirAbsorptionEnabled() const override { return false; }
        [[nodiscard]] AmReal32 EvaluateAirAbsorption(const AmVector3&, const AmVector3&, AmUInt32) const override { return 1.0f; }

    private:
        AmReal64 _maxDistance;
        AmReal32 _spreadFactor = 1.0f;
        mutable AmUInt32 _orientedGainCount = 0;
        mutable std::optional<Orientation> _lastOrientation;
    };

    /**
     * @brief Minimal mock Sound for testing nodes that need GetNearFieldGain().
     */
    class MockSound : public Sound
    {
    public:
        explicit MockSound(AmReal32 nearFieldGain = 1.0f)
        {
            _nearFieldGain.Init(nearFieldGain);
            _spread.Init(kAmSpreadNative);
            _gain.Init(1.0f);
            _pitch.Init(1.0f);
            _priority.Init(1.0f);
        }

        [[nodiscard]] bool IsStream() const override { return false; }
        [[nodiscard]] bool IsLoop() const override { return false; }
        [[nodiscard]] const RtpcValue& GetNearFieldGain() const override { return _nearFieldGain; }

        [[nodiscard]] const RtpcValue& GetSpread() const override
        {
            return _spread;
        }

        void SetSpread(AmReal32 spread)
        {
            _spread.Init(spread);
        }

        [[nodiscard]] const RtpcValue& GetGain() const override { return _gain; }
        [[nodiscard]] const RtpcValue& GetPitch() const override { return _pitch; }
        [[nodiscard]] const RtpcValue& GetPriority() const override { return _priority; }
        [[nodiscard]] const Effect* GetEffect() const override { return nullptr; }
        [[nodiscard]] const Attenuation* GetAttenuation() const override { return nullptr; }
        [[nodiscard]] Bus GetBus() const override { return {}; }
        [[nodiscard]] eSpatialization GetSpatialization() const override { return eSpatialization_Position; }
        [[nodiscard]] eScope GetScope() const override { return eScope_World; }
        [[nodiscard]] AmSoundID GetId() const override { return 1; }
        [[nodiscard]] const AmString& GetName() const override { static AmString name = "MockSound"; return name; }
        [[nodiscard]] const AmOsString& GetPath() const override { static AmOsString path; return path; }
        void Load(std::shared_ptr<const FileSystem>) override {}

    private:
        RtpcValue _nearFieldGain;
        RtpcValue _spread;
        RtpcValue _gain;
        RtpcValue _pitch;
        RtpcValue _priority;
    };

    class NodeTestCase : public ComponentTestCase
    {
    public:
        void SetUp() override
        {
            ComponentTestCase::SetUp();
            Engine::RegisterDefaultExtensions();
            _mockLayer = std::make_unique<SpatialMockLayer>();
        }

        void TearDown() override
        {
            _mockLayer.reset();
            Engine::UnregisterDefaultExtensions();
            ComponentTestCase::TearDown();
        }

    protected:
        SpatialMockLayer& GetMockLayer() { return *_mockLayer; }
        const AmplimixLayer* GetLayer() const { return _mockLayer.get(); }
        AmplimixLayer* GetLayer() { return _mockLayer.get(); }

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
