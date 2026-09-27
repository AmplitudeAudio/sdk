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

#include <Core/Engine.h>
#include <Core/RoomInternalState.h>
#include <Mixer/Nodes/ReflectionsNode.h>

#include <SparkyStudios/Audio/Amplitude/Math/CartesianCoordinateSystem.h>

namespace SparkyStudios::Audio::Amplitude
{
    bool ReflectionsUpdateTracker::Changed(const RoomInternalState* roomState, const AmVector3& listenerLocation, AmReal32 speedOfSound)
    {
        if (roomState == nullptr)
        {
            _hasState = false;
            return false;
        }

        const AmReal32* coefficients = roomState->GetCoefficients();

        bool changed = !_hasState || roomState != _roomState || !(listenerLocation == _listenerLocation) ||
            !(roomState->GetLocation() == _roomLocation) || !(roomState->GetOrientation().GetQuaternion() == _roomOrientation) ||
            !(roomState->GetDimensions() == _roomDimensions) || roomState->GetCutOffFrequency() != _cutOffFrequency ||
            speedOfSound != _speedOfSound;

        for (AmSize i = 0; !changed && i < kAmRoomSurfaceCount; ++i)
            changed = coefficients[i] != _coefficients[i];

        if (!changed)
            return false;

        _hasState = true;
        _roomState = roomState;
        _listenerLocation = listenerLocation;
        _roomLocation = roomState->GetLocation();
        _roomOrientation = roomState->GetOrientation().GetQuaternion();
        _roomDimensions = roomState->GetDimensions();
        _cutOffFrequency = roomState->GetCutOffFrequency();
        _speedOfSound = speedOfSound;
        for (AmSize i = 0; i < kAmRoomSurfaceCount; ++i)
            _coefficients[i] = coefficients[i];

        return true;
    }

    ReflectionsNodeInstance::ReflectionsNodeInstance()
        : ProcessorNodeInstance(true)
        , _reflectionsProcessor(nullptr)
        , _numFramesProcessedOnEmptyInput(0)
    {}

    ReflectionsNodeInstance::~ReflectionsNodeInstance()
    {
        ampooldelete(eMemoryPoolKind_Amplimix, ReflectionsProcessor, _reflectionsProcessor);
        _reflectionsProcessor = nullptr;
    }

    void ReflectionsNodeInstance::Initialize(AmObjectID id, const AmplimixLayer* layer, const PipelineInstance* pipeline, AmSize paramCount)
    {
        ProcessorNodeInstance::Initialize(id, layer, pipeline, paramCount);

        const auto& deviceConfig = amEngine->GetMixer()->GetDeviceDescription();

        _orientationProcessor.Configure(1, true);
        _reflectionsProcessor = ampoolnew(
            eMemoryPoolKind_Amplimix, ReflectionsProcessor, deviceConfig.mDeviceOutputSampleRate, amEngine->GetSamplesPerStream());

        _output.Configure(1, true, amEngine->GetSamplesPerStream());
        _silenceBuffer = AudioBuffer(amEngine->GetSamplesPerStream(), kAmMonoChannelCount);
    }

    void ReflectionsNodeInstance::Configure(AmUInt64 frameCount, AmUInt16 channelCount)
    {
        ProcessorNodeInstance::Configure(frameCount, channelCount);

        _tempBuffer = AudioBuffer(frameCount, kAmMonoChannelCount);
    }

    const AudioBuffer* ReflectionsNodeInstance::Process(const AudioBuffer* input)
    {
        if (input == nullptr)
        {
            if (_numFramesProcessedOnEmptyInput < _reflectionsProcessor->GetNumFramesToProcessOnEmptyInput())
            {
                _numFramesProcessedOnEmptyInput += _silenceBuffer.GetFrameCount();
                input = &_silenceBuffer;
            }
            else
                return nullptr;
        }
        else
        {
            _numFramesProcessedOnEmptyInput = 0;
            AMPLITUDE_ASSERT(input->GetChannelCount() == kAmMonoChannelCount);
        }

        const auto* layer = GetLayer();
        const auto& room = layer->GetRoom();
        const AmReal32 roomGain = layer->GetChannel().GetState()->GetRoomGain(room.GetId());

        if (roomGain < kEpsilon)
            return nullptr;

        const auto& listener = layer->GetListener();
        if (!listener.Valid())
            return nullptr;

        // Recompute reflections only when the room, the listener or the speed of sound changed.
        const AmVector3& listenerLocation = listener.GetLocation();
        const AmReal32 speedOfSound = amEngine->GetSoundSpeed();
        if (_updateTracker.Changed(room.GetState(), listenerLocation, speedOfSound))
            _reflectionsProcessor->Update(room.GetState(), listenerLocation, speedOfSound);

        _output.Reset();

        AMPLITUDE_ASSERT(input->GetFrameCount() <= _tempBuffer.GetFrameCount());

        {
            // Apply reflections gain
            Gain::ApplyReplaceConstantGain(roomGain, input->GetChannel(0), 0, _tempBuffer[0], 0, _output.GetSampleCount());

            // Process reflections
            _reflectionsProcessor->Process(_tempBuffer, &_output);
        }

        // Rotate the reflections from room space to world space. The listener rotation
        // is applied later by the AmbisonicRotator node, before binaural decode.
        const CartesianCoordinateSystem::Converter engineToAmbiX(
            CartesianCoordinateSystem::Default(), CartesianCoordinateSystem::AmbiX());
        _orientationProcessor.SetOrientation(Orientation(Inverse(engineToAmbiX.Forward(room.GetOrientation().GetQuaternion()))));
        _orientationProcessor.Process(&_output, _output.GetSampleCount());

        return _output.GetBuffer();
    }

    ReflectionsNode::ReflectionsNode()
        : Node("Reflections")
    {}
} // namespace SparkyStudios::Audio::Amplitude
