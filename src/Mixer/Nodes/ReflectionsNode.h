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

#ifndef _AM_IMPLEMENTATION_MIXER_NODES_REFLECTIONS_NODE_H
#define _AM_IMPLEMENTATION_MIXER_NODES_REFLECTIONS_NODE_H

#include <array>

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Node.h>

#include <Ambisonics/AmbisonicOrientationProcessor.h>
#include <Ambisonics/BFormat.h>
#include <DSP/ReflectionsProcessor.h>

namespace SparkyStudios::Audio::Amplitude
{
    class RoomInternalState;

    /**
     * @brief Tells whether early reflections must be recomputed for a room and listener.
     *
     * Remembers the last room state, listener location and speed of sound it was asked about, and
     * reports a change when any value that affects the reflections differs.
     */
    class ReflectionsUpdateTracker
    {
    public:
        /**
         * @brief Returns @c true when the reflections must be recomputed, and remembers the given state.
         *
         * @param[in] roomState The room the listener is in. @c nullptr means no room: returns @c false.
         * @param[in] listenerLocation The listener's world location.
         * @param[in] speedOfSound The engine speed of sound.
         */
        bool Changed(const RoomInternalState* roomState, const AmVector3& listenerLocation, AmReal32 speedOfSound);

    private:
        bool _hasState = false;
        const RoomInternalState* _roomState = nullptr;
        AmVector3 _listenerLocation{};
        AmVector3 _roomLocation{};
        AmQuaternion _roomOrientation{};
        AmVector3 _roomDimensions{};
        std::array<AmReal32, kAmRoomSurfaceCount> _coefficients{};
        AmReal32 _cutOffFrequency = 0.0f;
        AmReal32 _speedOfSound = 0.0f;
    };

    class ReflectionsNodeInstance final : public ProcessorNodeInstance
    {
    public:
        ReflectionsNodeInstance();
        ~ReflectionsNodeInstance() override;

        void Initialize(AmObjectID id, const AmplimixLayer* layer, const PipelineInstance* pipeline, AmSize paramCount) override;

        void Configure(AmUInt64 frameCount, AmUInt16 channelCount) override;

        const AudioBuffer* Process(const AudioBuffer* input) override;

    private:
        AmbisonicOrientationProcessor _orientationProcessor;
        ReflectionsProcessor* _reflectionsProcessor;

        BFormat _output;
        AudioBuffer _silenceBuffer;
        AudioBuffer _tempBuffer;

        AmSize _numFramesProcessedOnEmptyInput;

        ReflectionsUpdateTracker _updateTracker;
    };

    class ReflectionsNode final : public Node
    {
    public:
        ReflectionsNode();

        [[nodiscard]] AM_INLINE std::shared_ptr<NodeInstance> CreateInstance() const override
        {
            return ampoolshared(eMemoryPoolKind_Amplimix, ReflectionsNodeInstance);
        }

        [[nodiscard]] AM_INLINE bool CanConsume() const override
        {
            return true;
        }

        [[nodiscard]] AM_INLINE bool CanProduce() const override
        {
            return true;
        }

        [[nodiscard]] AM_INLINE AmSize GetMaxInputCount() const override
        {
            return 1;
        }

        [[nodiscard]] AM_INLINE AmSize GetMinInputCount() const override
        {
            return 1;
        }
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_NODES_REFLECTIONS_NODE_H
