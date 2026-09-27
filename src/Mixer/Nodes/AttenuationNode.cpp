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

#include <SparkyStudios/Audio/Amplitude/Mixer/Amplimix.h>

#include <Core/EngineInternalState.h>
#include <DSP/Gain.h>
#include <Mixer/Nodes/AttenuationNode.h>

namespace SparkyStudios::Audio::Amplitude
{
    constexpr AmReal32 kQ = 0.707107f; // sqrt(0.5)
    constexpr AmReal32 kMaxEQGain = 0.0625f;

    namespace
    {
        BiquadResonantFilterInstance* AsBiquad(const std::shared_ptr<FilterInstance>& filter)
        {
            // EnsureFilters() only creates instances from a BiquadResonantFilter factory.
            return static_cast<BiquadResonantFilterInstance*>(filter.get());
        }
    } // namespace

    void AirAbsorptionEQFilter::Normalize(std::array<AmReal32, kAmAirAbsorptionBandCount>& gains, AmReal32& overallGain)
    {
        if (const auto maxGain = std::max({ gains[0], gains[1], gains[2] }); maxGain < kEpsilon)
        {
            overallGain = 0.0f;
            for (auto i = 0; i < kAmAirAbsorptionBandCount; ++i)
                gains[i] = 1.0f;
        }
        else
        {
            for (auto i = 0; i < kAmAirAbsorptionBandCount; ++i)
            {
                gains[i] /= maxGain;
                gains[i] = std::max(gains[i], kMaxEQGain);
            }

            overallGain *= maxGain;
        }
    }

    AirAbsorptionEQFilter::AirAbsorptionEQFilter()
        : _eqFilterFactory()
        , _lowShelfFilter{ nullptr, nullptr }
        , _peakingFilter{ nullptr, nullptr }
        , _highShelfFilter{ nullptr, nullptr }
        , _tempBuffer()
        , _crossfadeBuffer()
        , _crossFader(nullptr)
        , _currentSet(0)
        , _needUpdateGains(false)
    {
        EnsureFilters();
    }

    AirAbsorptionEQFilter::~AirAbsorptionEQFilter()
    {
        for (AmUInt32 i = 0; i < 2; ++i)
        {
            _lowShelfFilter[i] = nullptr;
            _peakingFilter[i] = nullptr;
            _highShelfFilter[i] = nullptr;
        }

        if (_crossFader != nullptr)
        {
            ampooldelete(eMemoryPoolKind_Amplimix, AudioBufferCrossFader, _crossFader);
            _crossFader = nullptr;
        }
    }

    void AirAbsorptionEQFilter::Configure(AmUInt64 frameCount, AmUInt16 channelCount)
    {
        _tempBuffer = AudioBuffer(frameCount, channelCount);
        _crossfadeBuffer = AudioBuffer(frameCount, channelCount);

        if (_crossFader != nullptr)
            ampooldelete(eMemoryPoolKind_Amplimix, AudioBufferCrossFader, _crossFader);

        _crossFader = ampoolnew(eMemoryPoolKind_Amplimix, AudioBufferCrossFader, frameCount);
    }

    void AirAbsorptionEQFilter::SetGains(AmReal32 gainLow, AmReal32 gainMid, AmReal32 gainHigh)
    {
        const AmReal32 activeLow = _lowShelfFilter[_currentSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN);
        const AmReal32 activeMid = _peakingFilter[_currentSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN);
        const AmReal32 activeHigh = _highShelfFilter[_currentSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN);

        const bool changed = std::abs(gainLow - activeLow) > kEpsilon || std::abs(gainMid - activeMid) > kEpsilon ||
            std::abs(gainHigh - activeHigh) > kEpsilon;

        // The latest call wins: matching the active set cancels any pending change.
        _needUpdateGains = changed;

        if (!changed)
            return;

        // Stage all three gains into the inactive set; Process() crossfades to it.
        const AmUInt32 nextSet = 1 - _currentSet;
        _lowShelfFilter[nextSet]->SetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN, gainLow);
        _peakingFilter[nextSet]->SetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN, gainMid);
        _highShelfFilter[nextSet]->SetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN, gainHigh);
    }

    void AirAbsorptionEQFilter::Process(const AudioBuffer& input, AudioBuffer& output, AmReal32 sampleRate)
    {
        if (!_needUpdateGains)
        {
            ApplyFilters(_currentSet, input, output, sampleRate);
            return;
        }

        AMPLITUDE_ASSERT(_crossFader != nullptr);
        AMPLITUDE_ASSERT(_tempBuffer.GetFrameCount() >= input.GetFrameCount());
        AMPLITUDE_ASSERT(_crossfadeBuffer.GetFrameCount() >= input.GetFrameCount());

        const AmUInt32 nextSet = 1 - _currentSet;

        // Start the new set from the active set's history so it does not ring from stale state.
        CopyFilterState(_currentSet, nextSet);

        // Render old and new responses into distinct buffers: CrossFade asserts its output aliases neither input.
        ApplyFilters(_currentSet, input, _tempBuffer, sampleRate);
        ApplyFilters(nextSet, input, _crossfadeBuffer, sampleRate);

        // Fade the new response in and the old one out; the block ends on the new response.
        _crossFader->CrossFade(_crossfadeBuffer, _tempBuffer, output);

        _currentSet = nextSet;
        _needUpdateGains = false;
    }

    void AirAbsorptionEQFilter::EnsureFilters()
    {
        for (AmUInt32 i = 0; i < 2; ++i)
        {
            if (_lowShelfFilter[i] == nullptr)
            {
                _eqFilterFactory.InitializeLowShelf(kHighCutoffFrequencies[0], kQ, 0.0f);
                _lowShelfFilter[i] = _eqFilterFactory.CreateInstance();
            }

            if (_peakingFilter[i] == nullptr)
            {
                const AmReal32 cutoffFrequency = std::sqrt(kLowCutoffFrequencies[1] * kHighCutoffFrequencies[1]);
                _eqFilterFactory.InitializePeaking(
                    cutoffFrequency, cutoffFrequency / (kHighCutoffFrequencies[1] - kLowCutoffFrequencies[1]), 0.0f);
                _peakingFilter[i] = _eqFilterFactory.CreateInstance();
            }

            if (_highShelfFilter[i] == nullptr)
            {
                _eqFilterFactory.InitializeHighShelf(kLowCutoffFrequencies[2], kQ, 0.0f);
                _highShelfFilter[i] = _eqFilterFactory.CreateInstance();
            }
        }
    }

    void AirAbsorptionEQFilter::ApplyFilters(AmUInt32 set, const AudioBuffer& input, AudioBuffer& output, AmReal32 sampleRate)
    {
        _lowShelfFilter[set]->Process(input, output, input.GetFrameCount(), sampleRate);
        _peakingFilter[set]->Process(output, output, input.GetFrameCount(), sampleRate);
        _highShelfFilter[set]->Process(output, output, input.GetFrameCount(), sampleRate);
    }

    void AirAbsorptionEQFilter::CopyFilterState(AmUInt32 fromSet, AmUInt32 toSet)
    {
        AsBiquad(_lowShelfFilter[toSet])->CopyStateFrom(*AsBiquad(_lowShelfFilter[fromSet]));
        AsBiquad(_peakingFilter[toSet])->CopyStateFrom(*AsBiquad(_peakingFilter[fromSet]));
        AsBiquad(_highShelfFilter[toSet])->CopyStateFrom(*AsBiquad(_highShelfFilter[fromSet]));
    }

    AttenuationNodeInstance::AttenuationNodeInstance()
        : ProcessorNodeInstance(false)
        , _gains{ 1.0f, 1.0f, 1.0f }
        , _eqFilter()
    {}

    void AttenuationNodeInstance::Configure(AmUInt64 frameCount, AmUInt16 channelCount)
    {
        ProcessorNodeInstance::Configure(frameCount, channelCount);

        _eqFilter.Configure(frameCount, channelCount);
    }

    const AudioBuffer* AttenuationNodeInstance::Process(const AudioBuffer* input)
    {
        const auto* layer = GetLayer();

        const Attenuation* attenuation = layer->GetAttenuation();
        if (attenuation == nullptr)
            return input;

        const Listener& listener = layer->GetListener();

        AmReal32 targetGain = 1.0f;
        AmVector3 effectiveLocation = layer->GetLocation();

        // Compute attenuated gain based on spatialization
        {
            const eSpatialization spatialization = layer->GetSpatialization();

            if (listener.Valid())
            {
                if (layer->IsMultiPosition())
                {
                    const AmSize instanceCount = layer->GetInstanceCount();
                    AmReal32 totalWeight = 0.0f;
                    AmReal32 blendedGain = 0.0f;
                    AmVector3 weightedLocation = kVector3Zero;

                    for (AmSize i = 0; i < instanceCount; ++i)
                    {
                        const AmVector3 location = layer->GetInstanceLocation(i);
                        const AmReal32 weight = layer->GetInstanceWeight(i);
                        const AmReal32 instanceGain = attenuation->GetGain(location, listener);

                        blendedGain += instanceGain * weight;
                        weightedLocation = Add(weightedLocation, Scale(location, weight));
                        totalWeight += weight;
                    }

                    // Normalize by total weight
                    constexpr AmReal32 kMinTotalWeight = 1e-3f;

                    if (totalWeight > kMinTotalWeight)
                    {
                        const AmReal32 invTotalWeight = 1.0f / totalWeight;
                        targetGain = blendedGain * invTotalWeight;
                        effectiveLocation = Scale(weightedLocation, invTotalWeight);
                    }
                    else
                    {
                        targetGain = 0.0f;
                        effectiveLocation = kVector3Zero;
                    }
                }
                else
                {
                    const Entity& entity = layer->GetEntity();

                    if (spatialization == eSpatialization_PositionOrientation)
                    {
                        AMPLITUDE_ASSERT(entity.Valid());
                        targetGain *= attenuation->GetGain(entity, listener);
                    }
                    else if (spatialization == eSpatialization_HRTF && entity.Valid())
                    {
                        targetGain *= attenuation->GetGain(entity, listener);
                    }
                    else if (spatialization == eSpatialization_Position)
                    {
                        // Position-based spatialization, or HRTF-based spatialization without entity
                        targetGain *= attenuation->GetGain(effectiveLocation, listener);
                    }
                }
            }
            else
            {
                // No sound without listener on an attenuated source
                targetGain = 0.0f;
            }
        }

        const AmUInt64 frames = input->GetFrameCount();
        const AmUInt16 channels = _output.GetChannelCount();

        // Separate-mode instances share this node within a block: each instance snaps to its own gain.
        if (layer->GetInstancingMode() == eChannelInstanceMode_Separate && layer->GetInstanceCount() > 0)
            for (AmUInt16 c = 0; c < channels; ++c)
                _gain[c].Invalidate();

        if (const AmUInt32 sampleRate = layer->GetSampleRate(); sampleRate > 0)
            for (AmUInt16 c = 0; c < channels; ++c)
                _gain[c].SetMinRampFrames(GainRampMinFrames(sampleRate));

        const bool applyAirAbsorption = attenuation->IsAirAbsorptionEnabled() && listener.Valid();

        // Normalize() may zero targetGain, so air absorption is evaluated before the cull check.
        // While fading out, band gains are not re-evaluated: the EQ keeps its last setting.
        if (applyAirAbsorption && !Gain::IsZero(targetGain))
        {
            const AmVector3& listenerLocation = listener.GetLocation();

            for (AmUInt32 i = 0; i < kAmAirAbsorptionBandCount; ++i)
                _gains[i] = attenuation->EvaluateAirAbsorption(effectiveLocation, listenerLocation, i);

            AirAbsorptionEQFilter::Normalize(_gains, targetGain);
            _eqFilter.SetGains(_gains[0], _gains[1], _gains[2]);
        }

        // Cull only once the gain has fully ramped down (or was never audible). Leaving the processors
        // initialized at 0 makes a later re-entry fade in instead of jumping.
        const bool silent = !_gain[0].IsInitialized() || (Gain::IsZero(_gain[0].GetGain()) && !_gain[0].IsRamping());
        if (Gain::IsZero(targetGain) && silent)
        {
            for (AmUInt16 c = 0; c < channels; ++c)
                _gain[c].Reset(0.0f);

            return nullptr;
        }

        for (AmUInt16 c = 0; c < channels; ++c)
            _gain[c].ApplyGain(targetGain, input->GetChannel(c), 0, _output[c], 0, frames, false);

        if (applyAirAbsorption)
            _eqFilter.Process(_output, _output, layer->GetSampleRate());

        return &_output;
    }

    AttenuationNode::AttenuationNode()
        : Node("Attenuation")
    {}
} // namespace SparkyStudios::Audio::Amplitude
