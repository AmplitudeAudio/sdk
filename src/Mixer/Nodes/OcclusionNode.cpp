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

    /**
     * @brief Computes the directivity of a source, as seen from a given direction.
     *
     * @param[in] alpha The weight of the directional part, in [0, 1]: 0 is omnidirectional, 0.5 a cardioid and 1 a
     * figure-of-eight.
     * @param[in] order The sharpness of the lobe.
     * @param[in] localDirection The direction to the other party, in the frame of the source. In that frame, forward
     * is +Y, and the lobe is centered on it.
     */
    AM_INLINE AmReal32 CalculateDirectivity(AmReal32 alpha, AmReal32 order, const AmVector3& localDirection)
    {
        // Clamp alpha weighting.
        const AmReal32 alphaClamped = AM_CLAMP(alpha, 0.0f, 1.0f);

        // Check for zero-valued alpha (omnidirectional).
        if (alphaClamped < kEpsilon)
            return 1.0f;

        // Coincident positions have no direction: they are on-axis.
        const AmReal32 distance = Length(localDirection);
        if (distance < kEpsilon)
            return 1.0f;

        const AmReal32 cosToForward = AM_CLAMP(localDirection.y / distance, -1.0f, 1.0f);
        const AmReal32 gain = (1.0f - alphaClamped) + alphaClamped * cosToForward;

        return std::pow(std::abs(gain), std::max(order, 1.0f));
    }

    AM_INLINE AmReal32 CalculateOcclusionFilterCoefficient(AmReal32 directivity, AmReal32 occlusion)
    {
        const AmReal32 factor = 1.0f / IntegerPow(occlusion + 1.0f, 4);
        return std::max(0.0f, 1.0f - directivity * factor);
    }

    OcclusionNodeInstance::OcclusionNodeInstance()
        : _filter()
        , _currentOcclusion(0)
        , _occlusionFilter(nullptr)
    {
        _occlusionFilter = _filter.CreateInstance();
    }

    OcclusionNodeInstance::~OcclusionNodeInstance()
    {
        _occlusionFilter = nullptr;
    }

    const AudioBuffer* OcclusionNodeInstance::Process(const AudioBuffer* input)
    {
        const auto* layer = GetLayer();

        const AmReal32 occlusion = layer->GetOcclusion();

        const auto frames = input->GetFrameCount();
        const auto channels = input->GetChannelCount();
        const auto sampleRate = layer->GetSoundFormat().GetSampleRate();

        const Listener listener = layer->GetListener();
        const Entity entity = layer->GetEntity();

        if (!listener.Valid())
            return nullptr;

        // The layer location and orientation are the instance's when a separate-mode instance is being processed.
        const AmVector3 sourceLocation = layer->GetLocation();

        // The direction of the source in the listener's frame, where the listener's lobe points forward.
        AmVector3 direction = GetRelativeDirection(listener.GetLocation(), listener.GetOrientation().GetQuaternion(), sourceLocation);
        const AmReal32 listenerDirectivity = CalculateDirectivity(listener.GetDirectivity(), listener.GetDirectivitySharpness(), direction);

        AmReal32 soundDirectivity = 1.0f;
        if (entity.Valid())
        {
            // The direction of the listener in the source's frame, where the source's lobe points forward.
            direction = GetRelativeDirection(sourceLocation, layer->GetOrientation().GetQuaternion(), listener.GetLocation());
            soundDirectivity = CalculateDirectivity(entity.GetDirectivity(), entity.GetDirectivitySharpness(), direction);
        }

        const auto& lpfCurve = Engine::GetInstance()->GetOcclusionCoefficientCurve();
        const auto& gainCurve = Engine::GetInstance()->GetOcclusionGainCurve();

        _currentOcclusion = Lerp(kOcclusionSmoothingCoefficient, occlusion, _currentOcclusion);

        const AmReal32 lpf = lpfCurve.Get(_currentOcclusion);
        const AmReal32 coefficient = CalculateOcclusionFilterCoefficient(listenerDirectivity * soundDirectivity, lpf);

        _output.Clear();

        if (coefficient > kEpsilon)
        {
            // Update the filter coefficients
            _occlusionFilter->SetParameter(MonoPoleFilter::ATTRIBUTE_COEFFICIENT, AM_CLAMP(coefficient, 0.0f, 1.0f));

            // Apply Low Pass Filter
            _occlusionFilter->Process(*input, _output, frames, sampleRate);
        }
        else
        {
            AudioBuffer::Copy(*input, 0, _output, 0, frames);
        }

        const AmReal32 gain = gainCurve.Get(_currentOcclusion);

        // Apply Gain
        for (AmSize c = 0; c < channels; ++c)
            Gain::ApplyReplaceConstantGain(gain, _output[c], 0, _output[c], 0, frames);

        return &_output;
    }

    OcclusionNode::OcclusionNode()
        : Node("Occlusion")
    {}
} // namespace SparkyStudios::Audio::Amplitude
