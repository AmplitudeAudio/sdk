// Copyright (c) 2021-present Sparky Studios. All rights reserved.
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

#ifndef SS_AMPLITUDE_AUDIO_OCCLUSION_PROCESSOR_H
#define SS_AMPLITUDE_AUDIO_OCCLUSION_PROCESSOR_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/EngineInternalState.h>
#include <Sound/Filters/MonoPoleFilter.h>

namespace SparkyStudios::Audio::Amplitude
{
    // Low pass filter coefficient for smoothing the applied occlusion. This avoids
    // sudden unrealistic changes in the volume of a sound object. Range [0, 1].
    // The value below has been calculated empirically.
    constexpr AmReal32 kOcclusionSmoothingCoefficient = 0.75f;

    AM_INLINE AmReal32 CalculateDirectivity(AmReal32 alpha, AmReal32 order, const SphericalPosition& position)
    {
        // Clamp alpha weighting.
        const AmReal32 alphaClamped = std::min(std::max(alpha, 0.0f), 1.0f);

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

    class OcclusionProcessorInstance : public SoundProcessorInstance
    {
    public:
        OcclusionProcessorInstance()
            : _filter()
            , _currentOcclusions()
            , _occlusionFilters()
        {}

        ~OcclusionProcessorInstance() override
        {
            for (auto& [soundId, filter] : _occlusionFilters)
                _filter.DestroyInstance(filter);
        }

        void Process(
            AmAudioSampleBuffer out,
            AmConstAudioSampleBuffer in,
            AmUInt64 frames,
            AmSize bufferSize,
            AmUInt16 channels,
            AmUInt32 sampleRate,
            const AmplimixLayer* layer) override
        {
            const AmReal32 occlusion = layer->GetOcclusion();

            if (out != in)
                std::memcpy(out, in, bufferSize);

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

            const auto& lpfCurve = amEngine->GetOcclusionCoefficientCurve();
            const auto& gainCurve = amEngine->GetOcclusionGainCurve();

            if (const AmReal32 lpf = lpfCurve.Get(coefficient); lpf > 0)
            {
                if (!_occlusionFilters.contains(soundId))
                {
                    _filter.Init(lpf);
                    _occlusionFilters[soundId] = _filter.CreateInstance();
                }

                // Update the filter coefficients
                _occlusionFilters[soundId]->SetFilterParameter(MonoPoleFilter::ATTRIBUTE_COEFFICIENT, AM_CLAMP(coefficient, 0.0f, 1.0f));

                // Apply Low Pass Filter
                _occlusionFilters[soundId]->Process(out, frames, bufferSize, channels, sampleRate);
            }

            const AmReal32 gain = gainCurve.Get(_currentOcclusions[soundId]);

            // Apply Gain
            const AmSize length = frames * channels;

#if defined(AM_SIMD_INTRINSICS)
            const AmSize end = AmAudioFrame::size * (length / AmAudioFrame::size);
            const auto factor = xsimd::batch(gain);

            for (AmSize i = 0; i < end; i += AmAudioFrame::size)
            {
                const auto bin = xsimd::load_aligned(&out[i]);
                xsimd::store_aligned(&out[i], xsimd::mul(bin, factor));
            }

            for (AmSize i = end; i < length; i++)
            {
                out[i] = out[i] * gain;
            }
#else
            for (AmSize i = 0; i < length; i++)
            {
                out[i] = out[i] * gain;
            }
#endif // AM_SIMD_INTRINSICS
        }

        void Cleanup(const AmplimixLayer* layer) override
        {
            if (!_occlusionFilters.contains(layer->GetId()))
                return;

            _filter.DestroyInstance(_occlusionFilters[layer->GetId()]);
            _occlusionFilters.erase(layer->GetId());
        }

    private:
        MonoPoleFilter _filter;

        // Cached state per sound instance
        std::map<AmObjectID, AmReal32> _currentOcclusions;
        std::map<AmObjectID, FilterInstance*> _occlusionFilters;
    };

    class OcclusionProcessor final : public SoundProcessor
    {
    public:
        OcclusionProcessor()
            : SoundProcessor("OcclusionProcessor")
        {}

        SoundProcessorInstance* CreateInstance() override
        {
            return ampoolnew(MemoryPoolKind::Amplimix, OcclusionProcessorInstance);
        }

        void DestroyInstance(SoundProcessorInstance* instance) override
        {
            ampooldelete(MemoryPoolKind::Amplimix, OcclusionProcessorInstance, (OcclusionProcessorInstance*)instance);
        }
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_OCCLUSION_PROCESSOR_H
