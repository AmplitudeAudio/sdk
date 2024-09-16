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

#include <DSP/Delay.h>
#include <DSP/ReflectionsProcessor.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    // Maximum allowed delay time for a reflection. Above 2s, the effective output
    // level of a reflection will fall below -60dB and thus perceived dynamic
    // changes should be negligible.
    constexpr AmSize kMaxDelayTimeSeconds = 2;

    // Returns the maximum delay time in the given set of reflections.
    static AmReal32 FindMaxReflectionDelayTime(const std::vector<Reflection>& reflections)
    {
        AmReal32 maxDelayTime = 0.0f;
        for (const auto& reflection : reflections)
            maxDelayTime = AM_MAX(maxDelayTime, reflection.m_delaySeconds);

        return maxDelayTime;
    }

    ReflectionsProcessor::ReflectionsProcessor(AmUInt32 sampleRate, AmSize frameCount)
        : _sampleRate(sampleRate)
        , _frameCount(frameCount)
        , _maxDelaySamples(kMaxDelayTimeSeconds * sampleRate)
        , _lowPassFilter(nullptr)
        , _tempMonoBuffer(frameCount, kAmMonoChannelCount)
        , _currentReflectionBuffer(frameCount, kAmFirstOrderAmbisonicChannelCount)
        , _targetReflectionBuffer(frameCount, kAmFirstOrderAmbisonicChannelCount)
        , _reflections(kAmRoomSurfaceCount)
        , _crossFade(false)
        , _crossFader(frameCount)
        , _frameCountOnEmptyInput(0)
        , _delays(kAmRoomSurfaceCount)
        , _delayFilter(_maxDelaySamples, frameCount)
        , _delayBuffer(frameCount, kAmRoomSurfaceCount)
        , _gains(kAmRoomSurfaceCount)
        , _gainProcessors(kAmRoomSurfaceCount)
    {
        _lowPassFilter = Filter::Construct("MonoPole");
        _lowPassFilter->SetParameter(MonoPoleFilter::ATTRIBUTE_COEFFICIENT, 0.0f);
    }

    ReflectionsProcessor::~ReflectionsProcessor()
    {
        if (_lowPassFilter != nullptr)
        {
            Filter::Destruct("MonoPole", _lowPassFilter);
            _lowPassFilter = nullptr;
        }
    }

    void ReflectionsProcessor::Update(const RoomInternalState* roomState, const AmVec3& listenerPosition, AmReal32 speedOfSound)
    {
        const AmReal32 lowPassCoefficient = ComputeMonopoleFilterCoefficient(roomState->GetCutOffFrequency(), _sampleRate);
        _lowPassFilter->SetParameter(MonoPoleFilter::ATTRIBUTE_COEFFICIENT, lowPassCoefficient);

        const AmVec3& relativeListenerPosition =
            GetRelativeDirection(roomState->GetLocation(), AM_InvQ(roomState->GetOrientation().GetQuaternion()), listenerPosition);

        ComputeReflections(relativeListenerPosition, roomState->GetDimensions(), speedOfSound, roomState->GetCoefficients());

        _frameCountOnEmptyInput = _frameCount + static_cast<AmSize>(FindMaxReflectionDelayTime(_reflections) * _sampleRate);

        // Enable cross-fading between reflections
        _crossFade = true;
    }

    void ReflectionsProcessor::Process(const AudioBuffer& input, BFormat* output)
    {
        AMPLITUDE_ASSERT(input.GetChannelCount() == kAmMonoChannelCount);
        AMPLITUDE_ASSERT(input.GetFrameCount() == _frameCount);
        AMPLITUDE_ASSERT(output->GetChannelCount() >= kAmFirstOrderAmbisonicChannelCount);
        AMPLITUDE_ASSERT(output->GetSampleCount() == _frameCount);

        // Prefilter mono input
        _tempMonoBuffer.Clear();

        if (_lowPassFilter->GetParameter(MonoPoleFilter::ATTRIBUTE_COEFFICIENT) < kEpsilon)
            _tempMonoBuffer = input;
        else
            _lowPassFilter->Process(input, _tempMonoBuffer, _frameCount, _sampleRate);

        _delayFilter.Insert(_tempMonoBuffer[0]);

        // Process reflections
        if (_crossFade)
        {
            ProcessReflections(_currentReflectionBuffer);
            UpdateGainAndDelay();
            ProcessReflections(_targetReflectionBuffer);

            _crossFader.CrossFade(_targetReflectionBuffer, _currentReflectionBuffer, *output->GetBuffer());
            _crossFade = false;
        }
        else
        {
            ProcessReflections(*output->GetBuffer());
        }
    }

    void ReflectionsProcessor::ComputeReflections(
        const AmVec3& relativeListenerPosition, const AmVec3& dimensions, AmReal32 speedOfSound, const AmReal32* reflectionCoefficients)
    {
        const AmVec3 roomCenter = AM_V3(0.0f, 0.0f, 0.0f);

        BoxShape roomShape(roomCenter, dimensions);
        if (!roomShape.Contains(relativeListenerPosition))
        {
            // Nothing to do if the listener is outside the room.
            std::fill(_reflections.begin(), _reflections.end(), Reflection());
            return;
        }

        // Calculate the distance of the listener to each wall.
        // Since all the sources are 'attached' to the listener in the computation
        // of reflections, the distance traveled is arbitrary. So, we add 1.0f to
        // the computed distance in order to avoid delay time approaching 0 and the
        // magnitude approaching +inf.
        const AmVec3& offsets = 0.5f * dimensions;
        const AmReal32 distances[kAmRoomSurfaceCount] = {
            offsets[0] + relativeListenerPosition[0] + 1.0f, offsets[0] - relativeListenerPosition[0] + 1.0f,
            offsets[1] - relativeListenerPosition[1] + 1.0f, offsets[1] + relativeListenerPosition[1] + 1.0f,
            offsets[2] + relativeListenerPosition[2] + 1.0f, offsets[2] - relativeListenerPosition[2] + 1.0f
        };

        for (size_t i = 0; i < kAmRoomSurfaceCount; ++i)
        {
            // Convert distances to time delays in seconds.
            _reflections[i].m_delaySeconds = distances[i] / speedOfSound;
            // Division by distance is performed here as we don't want this applied more than once.
            _reflections[i].m_magnitude = reflectionCoefficients[i] / distances[i];
        }
    }

    void ReflectionsProcessor::UpdateGainAndDelay()
    {
        for (size_t i = 0; i < kAmRoomSurfaceCount; ++i)
        {
            _delays[i] =
                std::min(_maxDelaySamples, static_cast<AmSize>(_reflections[i].m_delaySeconds * static_cast<AmReal32>(_sampleRate)));
            _gains[i] = _reflections[i].m_magnitude;
        }
    }

    void ReflectionsProcessor::ProcessReflections(AudioBuffer& output)
    {
        AMPLITUDE_ASSERT(output.GetChannelCount() >= kAmFirstOrderAmbisonicChannelCount);
        output.Clear();

        for (AmSize i = 0; i < kAmRoomSurfaceCount; ++i)
        {
            auto& delayChannel = _delayBuffer[i];
            _delayFilter.Process(delayChannel, _delays[i]);

            const bool isZeroGain = Gain::IsZero(_gains[i]) && Gain::IsZero(_gainProcessors[i].GetGain());

            if (isZeroGain)
            {
                _gainProcessors[i].SetGain(0.0f);
            }
            else
            {
                // Apply reflections gain
                _gainProcessors[i].ApplyGain(_gains[i], delayChannel, 0, delayChannel, 0, delayChannel.size(), false);

                // Apply ambisonic reflection encoding
                output[eBFormatChannel_W] += delayChannel;
                switch (static_cast<RoomWall>(i))
                {
                case RoomWall::Right: // Right wall reflection
                    output[eBFormatChannel_Y] += delayChannel;
                    break;

                case RoomWall::Left: // Left wall reflection
                    output[eBFormatChannel_Y] -= delayChannel;
                    break;

                case RoomWall::Front: // Front wall reflection
                    output[eBFormatChannel_X] += delayChannel;
                    break;

                case RoomWall::Back: // Back wall reflection
                    output[eBFormatChannel_X] -= delayChannel;
                    break;

                case RoomWall::Floor: // Floor reflection
                    output[eBFormatChannel_Z] += delayChannel;
                    break;

                case RoomWall::Ceiling: // Ceiling reflection
                    output[eBFormatChannel_Z] -= delayChannel;
                    break;
                }
            }
        }
    }
} // namespace SparkyStudios::Audio::Amplitude
