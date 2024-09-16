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

#pragma once

#ifndef _AM_IMPLEMENTATION_DSP_REFLECTIONS_PROCESSOR_H
#define _AM_IMPLEMENTATION_DSP_REFLECTIONS_PROCESSOR_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <Ambisonics/BFormat.h>
#include <Core/AudioBufferCrossFader.h>
#include <Core/RoomInternalState.h>
#include <DSP/Delay.h>
#include <DSP/Filters/MonoPoleFilter.h>
#include <DSP/Gain.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct Reflection
    {
        // Time of arrival of the reflection in seconds.
        AmReal32 m_delaySeconds = 0.0f;

        // Magnitude of the reflection.
        AmReal32 m_magnitude = 0.0f;
    };

    class ReflectionsProcessor
    {
    public:
        ReflectionsProcessor(AmUInt32 sampleRate, AmSize frameCount);
        ~ReflectionsProcessor();

        void Update(const RoomInternalState* roomState, const AmVec3& listenerPosition, AmReal32 speedOfSound);

        void Process(const AudioBuffer& input, BFormat* output);

        AM_INLINE AmSize GetNumFramesToProcessOnEmptyInput() const
        {
            return _frameCountOnEmptyInput;
        }

    private:
        void ComputeReflections(
            const AmVec3& relativeListenerPosition,
            const AmVec3& dimensions,
            AmReal32 speedOfSound,
            const AmReal32* reflectionCoefficients);

        void UpdateGainAndDelay();

        void ProcessReflections(AudioBuffer& output);

        const AmUInt32 _sampleRate;

        const AmSize _frameCount;

        const AmSize _maxDelaySamples;

        FilterInstance* _lowPassFilter;

        AudioBuffer _tempMonoBuffer;

        AudioBuffer _currentReflectionBuffer;
        AudioBuffer _targetReflectionBuffer;

        std::vector<Reflection> _reflections;

        bool _crossFade;
        AudioBufferCrossFader _crossFader;

        // Number of frames needed to keep processing on empty input signal.
        AmSize _frameCountOnEmptyInput;

        // Number of samples of delay to be applied for each reflection.
        std::vector<AmSize> _delays;

        // Delay filter to delay the incoming buffer.
        Delay _delayFilter;

        // Delay buffer used to store delayed reflections before scaling and encoding.
        AudioBuffer _delayBuffer;

        // Gains to be applied for each reflection.
        std::vector<AmReal32> _gains;

        // |GainProcessor|s to apply |gains_|.
        std::vector<GainProcessor> _gainProcessors;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_DSP_REFLECTIONS_PROCESSOR_H
