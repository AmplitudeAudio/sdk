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
#include <DSP/Filters/MonoPoleFilter.h>
#include <DSP/Gain.h>
#include <Mixer/Nodes/OcclusionNode.h>

namespace SparkyStudios::Audio::Amplitude
{
    // Low pass filter coefficient for smoothing the applied occlusion. This avoids
    // sudden unrealistic changes in the volume of a sound object. Range [0, 1].
    // The value below has been calculated empirically.
    constexpr AmReal32 kOcclusionSmoothingCoefficient = 0.75f;

    AM_INLINE AmReal32 CalculateDirectivity(AmReal32 alpha, AmReal32 order, const SphericalPosition& position)
    {
        // Clamp alpha weighting.
        const AmReal32 alphaClamped = AM_CLAMP(alpha, 0.0f, 1.0f);

        // Check for zero-valued alpha (omnidirectional).
        if (alphaClamped < kEpsilon)
            return 1.0f;

        const AmReal32 gain = (1.0f - alphaClamped) + alphaClamped * (std::cos(position.GetAzimuth()) * std::cos(position.GetElevation()));

        return std::pow(std::abs(gain), std::max(order, 1.0f));
    }

    AM_INLINE AmReal32 CalculateOcclusionFilterCoefficient(AmReal32 directivity, AmReal32 occlusion)
    {
        const AmReal32 factor = 1.0f / IntegerPow(occlusion + 1.0f, 4);
        return std::max(0.0f, 1.0f - directivity * factor);
    }

    OcclusionNodeInstance::OcclusionNodeInstance(AmObjectID id, const Pipeline* pipeline)
        : ProcessorNodeInstance(id, pipeline)
        , _filter()
        , _currentOcclusions()
        , _occlusionFilters()
    {}

    OcclusionNodeInstance::~OcclusionNodeInstance()
    {
        for (const auto& filter : _occlusionFilters | std::views::values)
            _filter.DestroyInstance(filter);

        _occlusionFilters.clear();
        _currentOcclusions.clear();
    }

    AudioBuffer OcclusionNodeInstance::Process(const AudioBuffer& input)
    {
        AudioBuffer output = input.Clone();

        const auto* layer = GetLayer();
        if (layer == nullptr)
            return output;

        const AmReal32 occlusion = layer->GetOcclusion();

        const auto frames = input.GetFrameCount();
        const auto channels = input.GetChannelCount();
        const auto sampleRate = layer->GetSoundFormat().GetSampleRate();

        const AmUInt64 soundId = layer->GetId();

        const Listener listener = layer->GetListener();
        const Entity entity = layer->GetEntity();

        // Compute the relative listener/source direction in spherical angles.
        AmVec3 direction = GetRelativeDirection(listener.GetLocation(), listener.GetOrientation().GetQuaternion(), layer->GetLocation());
        const SphericalPosition listenerDirection = SphericalPosition::FromWorldSpace(direction);
        const AmReal32 listenerDirectivity =
            CalculateDirectivity(listener.GetDirectivity(), listener.GetDirectivitySharpness(), listenerDirection);

        AmReal32 soundDirectivity = 0.0f;
        if (entity.Valid())
        {
            direction = GetRelativeDirection(entity.GetLocation(), entity.GetOrientation().GetQuaternion(), listener.GetLocation());
            const SphericalPosition entityDirection = SphericalPosition::FromWorldSpace(direction);
            soundDirectivity = CalculateDirectivity(entity.GetDirectivity(), entity.GetDirectivitySharpness(), entityDirection);
        }

        _currentOcclusions[soundId] = AM_Lerp(occlusion, kOcclusionSmoothingCoefficient, _currentOcclusions[soundId]);

        const AmReal32 coefficient =
            CalculateOcclusionFilterCoefficient(listenerDirectivity * soundDirectivity, _currentOcclusions[soundId]);

        const auto& lpfCurve = Engine::GetInstance()->GetOcclusionCoefficientCurve();
        const auto& gainCurve = Engine::GetInstance()->GetOcclusionGainCurve();

        if (const AmReal32 lpf = lpfCurve.Get(coefficient); lpf > 0)
        {
            if (!_occlusionFilters.contains(soundId))
            {
                _filter.Initialize(lpf);
                _occlusionFilters[soundId] = _filter.CreateInstance();
            }

            // Update the filter coefficients
            _occlusionFilters[soundId]->SetParameter(MonoPoleFilter::ATTRIBUTE_COEFFICIENT, AM_CLAMP(lpf, 0.0f, 1.0f));

            // Apply Low Pass Filter
            _occlusionFilters[soundId]->Process(input, output, frames, sampleRate);
        }

        const AmReal32 gain = gainCurve.Get(_currentOcclusions[soundId]);

        // Apply Gain
        for (AmSize c = 0; c < output.GetChannelCount(); ++c)
            Gain::ApplyReplaceConstantGain(gain, output[c], 0, output[c], 0, frames);

        return output;
    }

    OcclusionNode::OcclusionNode()
        : Node("Occlusion")
    {}

    NodeInstance* OcclusionNode::CreateInstance(AmObjectID id, const Pipeline* pipeline) const
    {
        return ampoolnew(MemoryPoolKind::Amplimix, OcclusionNodeInstance, id, pipeline);
    }

    void OcclusionNode::DestroyInstance(NodeInstance* instance) const
    {
        ampooldelete(MemoryPoolKind::Amplimix, OcclusionNodeInstance, (OcclusionNodeInstance*)instance);
    }
} // namespace SparkyStudios::Audio::Amplitude
