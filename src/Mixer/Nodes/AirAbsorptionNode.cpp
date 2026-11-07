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

#include <Core/EngineInternalState.h>
#include <DSP/Gain.h>
#include <Mixer/InstanceBlending.h>
#include <Mixer/Nodes/AirAbsorptionNode.h>

namespace SparkyStudios::Audio::Amplitude
{
    constexpr AmReal32 kQ = 0.707107f; // sqrt(0.5)
    constexpr AmReal32 kMaxEQGain = 0.0625f;

    AM_INLINE AmReal32 ToEQGainDb(AmReal32 linearGain)
    {
        return 20.0f * std::log10(std::max(linearGain, kMaxEQGain));
    }

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

    namespace
    {
        // The location air absorption is evaluated at: the layer location, or the weighted center of its instances when
        // it is multi-position.
        AmVector3 GetEffectiveLocation(const AmplimixLayer& layer)
        {
            if (!layer.IsMultiPosition())
                return layer.GetLocation();

            constexpr AmReal32 kMinTotalWeight = 1e-3f;

            AmReal32 totalWeight = 0.0f;
            AmVector3 weightedLocation = kVector3Zero;

            for (AmSize i = 0, count = layer.GetInstanceCount(); i < count; ++i)
            {
                const AmReal32 weight = layer.GetInstanceWeight(i);

                weightedLocation = Add(weightedLocation, Scale(layer.GetInstanceLocation(i), weight));
                totalWeight += weight;
            }

            return totalWeight > kMinTotalWeight ? Scale(weightedLocation, 1.0f / totalWeight) : kVector3Zero;
        }
    } // namespace

    AirAbsorptionNodeInstance::AirAbsorptionNodeInstance()
        : ProcessorNodeInstance(false)
        , _gains{ 1.0f, 1.0f, 1.0f }
        , _eqFilter()
    {}

    void AirAbsorptionNodeInstance::Configure(AmUInt64 frameCount, AmUInt16 channelCount)
    {
        ProcessorNodeInstance::Configure(frameCount, channelCount);

        _eqFilter.Configure(frameCount, channelCount);
    }

    const AudioBuffer* AirAbsorptionNodeInstance::Process(const AudioBuffer* input)
    {
        const auto* layer = GetLayer();

        const Attenuation* attenuation = layer->GetAttenuation();
        if (attenuation == nullptr || !attenuation->IsAirAbsorptionEnabled())
            return input;

        const Listener& listener = layer->GetListener();
        if (!listener.Valid())
            return input;

        const AmVector3 location = GetEffectiveLocation(*layer);
        const AmVector3& listenerLocation = listener.GetLocation();

        for (AmUInt32 i = 0; i < kAmAirAbsorptionBandCount; ++i)
            _gains[i] = attenuation->EvaluateAirAbsorption(location, listenerLocation, i);

        // The loudest band becomes a broadband gain, the EQ only keeps the shape of the spectrum.
        AmReal32 targetGain = 1.0f;
        AirAbsorptionEQFilter::Normalize(_gains, targetGain);
        _eqFilter.SetGains(ToEQGainDb(_gains[0]), ToEQGainDb(_gains[1]), ToEQGainDb(_gains[2]));

        const AmUInt64 frames = input->GetFrameCount();
        const AmUInt16 channels = _output.GetChannelCount();

        // Instances sharing one pipeline within a block must not ramp from each other's gain.
        if (layer->IsSharingPipelineAcrossInstances())
            for (AmUInt16 c = 0; c < channels; ++c)
                _gain[c].Invalidate();

        if (const AmUInt32 sampleRate = layer->GetSampleRate(); sampleRate > 0)
            for (AmUInt16 c = 0; c < channels; ++c)
                _gain[c].SetMinRampFrames(GainRampMinFrames(sampleRate));

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

        _eqFilter.Process(_output, _output, layer->GetSampleRate());

        return &_output;
    }

    AirAbsorptionNode::AirAbsorptionNode()
        : Node("AirAbsorption")
    {}
} // namespace SparkyStudios::Audio::Amplitude
