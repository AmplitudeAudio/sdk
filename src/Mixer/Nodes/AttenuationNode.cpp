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
        const AmReal32 oldGainLow = _lowShelfFilter[_currentSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN);
        const AmReal32 oldGainMid = _peakingFilter[_currentSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN);
        const AmReal32 oldGainHigh = _highShelfFilter[_currentSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN);

        if (std::abs(gainLow - oldGainLow) > kEpsilon)
        {
            _lowShelfFilter[_currentSet]->SetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN, gainLow);
            _needUpdateGains = true;
        }

        if (std::abs(gainMid - oldGainMid) > kEpsilon)
        {
            _peakingFilter[_currentSet]->SetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN, gainMid);
            _needUpdateGains = true;
        }

        if (std::abs(gainHigh - oldGainHigh) > kEpsilon)
        {
            _highShelfFilter[_currentSet]->SetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN, gainHigh);
            _needUpdateGains = true;
        }
    }

    void AirAbsorptionEQFilter::Process(const AudioBuffer& input, AudioBuffer& output, AmReal32 sampleRate)
    {
        if (_needUpdateGains)
        {
            AMPLITUDE_ASSERT(_crossFader != nullptr);
            AMPLITUDE_ASSERT(_tempBuffer.GetFrameCount() >= input.GetFrameCount());
            AMPLITUDE_ASSERT(_crossfadeBuffer.GetFrameCount() >= input.GetFrameCount());

            const AmUInt32 previousSet = _currentSet;
            _currentSet = 1 - _currentSet;

            _lowShelfFilter[_currentSet]->SetParameter(
                BiquadResonantFilter::ATTRIBUTE_FREQUENCY,
                _lowShelfFilter[previousSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_FREQUENCY));
            _lowShelfFilter[_currentSet]->SetParameter(
                BiquadResonantFilter::ATTRIBUTE_RESONANCE,
                _lowShelfFilter[previousSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_RESONANCE));
            _lowShelfFilter[_currentSet]->SetParameter(
                BiquadResonantFilter::ATTRIBUTE_GAIN, _lowShelfFilter[previousSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN));

            _peakingFilter[_currentSet]->SetParameter(
                BiquadResonantFilter::ATTRIBUTE_FREQUENCY,
                _peakingFilter[previousSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_FREQUENCY));
            _peakingFilter[_currentSet]->SetParameter(
                BiquadResonantFilter::ATTRIBUTE_RESONANCE,
                _peakingFilter[previousSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_RESONANCE));
            _peakingFilter[_currentSet]->SetParameter(
                BiquadResonantFilter::ATTRIBUTE_GAIN, _peakingFilter[previousSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN));

            _highShelfFilter[_currentSet]->SetParameter(
                BiquadResonantFilter::ATTRIBUTE_FREQUENCY,
                _highShelfFilter[previousSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_FREQUENCY));
            _highShelfFilter[_currentSet]->SetParameter(
                BiquadResonantFilter::ATTRIBUTE_RESONANCE,
                _highShelfFilter[previousSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_RESONANCE));
            _highShelfFilter[_currentSet]->SetParameter(
                BiquadResonantFilter::ATTRIBUTE_GAIN, _highShelfFilter[previousSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN));

            // Render the previous and current gain sets into distinct member buffers:
            // CrossFade asserts that its destination does not alias either input buffer.
            // The current set is the fade-to target (arg1), so the block ends on it,
            // matching the behavior of the old (aliasing) call.
            ApplyFilters(previousSet, input, _tempBuffer, sampleRate);
            ApplyFilters(_currentSet, input, _crossfadeBuffer, sampleRate);

            _crossFader->CrossFade(_crossfadeBuffer, _tempBuffer, output);

            _needUpdateGains = false;
        }
        else
        {
            ApplyFilters(_currentSet, input, output, sampleRate);
        }
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

        if (Gain::IsZero(targetGain))
            return nullptr;

        // Set and normalize gains for air absorption
        if (attenuation->IsAirAbsorptionEnabled() && listener.Valid())
        {
            const AmVector3& listenerLocation = listener.GetLocation();

            for (AmUInt32 i = 0; i < kAmAirAbsorptionBandCount; ++i)
                _gains[i] = attenuation->EvaluateAirAbsorption(effectiveLocation, listenerLocation, i);

            AirAbsorptionEQFilter::Normalize(_gains, targetGain);
            _eqFilter.SetGains(_gains[0], _gains[1], _gains[2]);
        }

        AudioBuffer::Copy(*input, 0, _output, 0, input->GetFrameCount());

        // Apply gain attenuation
        if (!Gain::IsOne(targetGain))
            for (AmSize c = 0; c < _output.GetChannelCount(); ++c)
                Gain::ApplyReplaceConstantGain(targetGain, input->GetChannel(c), 0, _output[c], 0, input->GetFrameCount());

        // Apply air absorption EQ filter
        if (attenuation->IsAirAbsorptionEnabled() && listener.Valid())
            _eqFilter.Process(_output, _output, layer->GetSampleRate());

        return &_output;
    }

    AttenuationNode::AttenuationNode()
        : Node("Attenuation")
    {}
} // namespace SparkyStudios::Audio::Amplitude
