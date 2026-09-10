// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#ifndef _AM_DSP_REVERB_H
#define _AM_DSP_REVERB_H

#include <SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h>
#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

#include <map>
#include <memory>

namespace SparkyStudios::Audio::Amplitude
{
    class Reverb;

    /**
     * @brief A Reverb instance.
     *
     * This class is where the DSP reverberation is performed. It processes audio frames
     * according to room parameters and the underlying reverb algorithm.
     *
     * An instance of this class will be created each time its parent Reverb is requested.
     *
     * @ingroup dsp
     */
    class AM_API_PUBLIC ReverbInstance
    {
    public:
        /**
         * @brief Constructs a new @c ReverbInstance object.
         *
         * @param[in] parent The parent @c Reverb object that created this instance.
         */
        explicit ReverbInstance(Reverb* parent);

        /**
         * @brief Destroys the @c ReverbInstance object.
         */
        virtual ~ReverbInstance() = default;

        /**
         * @brief Initializes the reverb instance for the specified sample rate.
         *
         * @param[in] sampleRate The sample rate in Hertz.
         */
        virtual void Initialize(AmUInt32 sampleRate) = 0;

        /**
         * @brief Resets the internal state and delay buffers of the reverb instance.
         */
        virtual void Reset() = 0;

        /**
         * @brief Sets room simulation parameters.
         *
         * @param[in] roomSize Normalized room size parameter.
         * @param[in] absorption Normalized absorption parameter.
         */
        virtual void SetRoomParameters(AmReal32 roomSize, AmReal32 absorption) = 0;

        /**
         * @brief Configures the reverb instance for the specified frame count.
         *
         * @param[in] frames The number of audio frames per process call.
         */
        virtual void Configure(AmUInt64 frames)
        {}

        /**
         * @brief Processes audio frames through the reverb instance.
         *
         * @param[in] in The input buffer.
         * @param[out] out The output buffer where reverberated audio is stored.
         * @param[in] frames The number of audio frames to process.
         * @param[in] sampleRate The current sample rate in Hertz.
         */
        virtual void Process(const AudioBuffer& in, AudioBuffer& out, AmUInt64 frames, AmUInt32 sampleRate) = 0;

        /**
         * @brief Gets the parent @c Reverb object that created this instance.
         *
         * @return A pointer to the parent @c Reverb object.
         */
        [[nodiscard]] Reverb* GetParent() const;

    protected:
        Reverb* _parent;
    };

    /**
     * @brief Base class used to create DSP reverbs.
     *
     * The @c Reverb class implements factory methods to create instances of @c ReverbInstance objects.
     *
     * The @c Reverb class follows the [plugin architecture](/plugins/anatomy), allowing creation
     * and registration of custom reverb algorithms to the Engine.
     *
     * @ingroup dsp
     */
    class AM_API_PUBLIC Reverb
    {
    public:
        /**
         * @brief Constructs a new @c Reverb object.
         *
         * @param[in] name The reverb name, e.g. "Freeverb".
         */
        explicit Reverb(AmString name);

        /**
         * @brief Default destructor.
         */
        virtual ~Reverb() = default;

        /**
         * @brief Gets the name of this reverb.
         *
         * @return The name of this reverb.
         */
        [[nodiscard]] const AmString& GetName() const;

        /**
         * @brief Creates a new instance of the reverb.
         *
         * @return A new instance of the reverb.
         */
        virtual std::shared_ptr<ReverbInstance> CreateInstance() = 0;

        /**
         * @brief Registers a new reverb.
         *
         * @note This method does nothing if the registry is locked.
         *
         * @param[in] reverb The reverb to add in the registry.
         *
         * @see LockRegistry, UnlockRegistry
         */
        static void Register(std::shared_ptr<Reverb> reverb);

        /**
         * @brief Unregisters a reverb.
         *
         * @note This method does nothing if the registry is locked.
         *
         * @param[in] reverb The reverb to remove from the registry.
         *
         * @see LockRegistry, UnlockRegistry
         */
        static void Unregister(std::shared_ptr<const Reverb> reverb);

        /**
         * @brief Look up a reverb by name.
         *
         * @param[in] name The name of the reverb to find.
         *
         * @return The reverb with the given name, or @c nullptr if not found.
         */
        static std::shared_ptr<Reverb> Find(const AmString& name);

        /**
         * @brief Gets the list of registered reverbs.
         *
         * @return The registry of reverbs.
         */
        static const std::map<AmString, std::shared_ptr<Reverb>>& GetRegistry();

        /**
         * @brief Locks the reverbs' registry.
         *
         * @warning This function is mainly used for internal purposes. It's
         * called before the @c Engine initialization, to discard the registration
         * of new reverbs after the engine is fully loaded.
         */
        static void LockRegistry();

        /**
         * @brief Unlocks the reverbs' registry.
         *
         * @warning This function is mainly used for internal purposes. It's
         * called after the @c Engine deinitialization, to allow the registration
         * of new reverbs after the engine is fully unloaded.
         */
        static void UnlockRegistry();

    protected:
        AmString m_name;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_DSP_REVERB_H
