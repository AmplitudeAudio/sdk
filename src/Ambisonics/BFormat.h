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

#ifndef _AM_IMPLEMENTATION_AMBISONICS_BFORMAT_H
#define _AM_IMPLEMENTATION_AMBISONICS_BFORMAT_H

#include <SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h>
#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

#include <Ambisonics/AmbisonicComponent.h>

namespace SparkyStudios::Audio::Amplitude
{
    class BFormat final : public AmbisonicComponent
    {
        friend class AmbisonicSource;
        friend class AmbisonicOrientationProcessor;

    public:
        BFormat();
        ~BFormat() override;

        /**
         * @copydoc AmbisonicComponent::Reset
         */
        void Reset() override;

        /**
         * @copydoc AmbisonicComponent::Refresh
         */
        void Refresh() override;

        /**
         * @brief Gets the number of samples.
         *
         * @return The number of samples.
         */
        [[nodiscard]] AM_INLINE AmUInt32 GetSampleCount() const
        {
            return _buffer->GetFrameCount();
        }

        /**
         * @brief Creates internal buffers for the given values. Existing buffers will be destroyed.
         *
         * @param order The order of the ambisonic component.
         * @param is3D Whether the ambisonic component is 3D or not (has height).
         * @param sampleCount The number of samples.
         *
         * @return @c true if the configuration is successful, @c false otherwise.
         */
        bool Configure(AmUInt32 order, bool is3D, AmUInt32 sampleCount);

        /**
         * @brief Copies a buffer's samples into the internal buffers.
         *
         * @param buffer The buffer to copy samples from.
         * @param channel The channel.
         * @param sampleCount The number of samples to copy.
         */
        void CopyStream(const AudioBufferChannel& buffer, AmUInt32 channel, AmUInt32 sampleCount) const;

        /**
         * @brief Adds values from the given buffer into the internal buffers.
         *
         * @param buffer The buffer to add samples from.
         * @param channel The channel.
         * @param sampleCount The number of samples.
         * @param offset The offset into the internal buffers.
         */
        void AddStream(const AudioBufferChannel& buffer, AmUInt32 channel, AmUInt32 sampleCount, AmUInt32 offset = 0) const;

        /**
         * @brief Gets the values from the internal buffers into the given buffer.
         *
         * @param buffer The buffer to get samples into.
         * @param channel The channel.
         * @param sampleCount The number of samples.
         */
        void GetStream(AudioBufferChannel& buffer, AmUInt32 channel, AmUInt32 sampleCount) const;

        [[nodiscard]] const AudioBufferChannel& GetBufferChannel(AmUInt32 channel) const
        {
            return (*_buffer)[channel];
        }

        AudioBufferChannel& GetBufferChannel(AmUInt32 channel)
        {
            return (*_buffer)[channel];
        }

        [[nodiscard]] AM_INLINE AmReal32 GetSample(AmUInt32 channel, AmUInt32 sampleIndex) const
        {
            return (*_buffer)[channel][sampleIndex];
        }

        AM_INLINE void SetSample(AmUInt32 channel, AmUInt32 sampleIndex, AmReal32 sample)
        {
            (*_buffer)[channel][sampleIndex] = sample;
        }

        [[nodiscard]] AM_INLINE AudioBuffer* GetBuffer() const
        {
            return _buffer;
        }

        /**
         * @brief Copy operator.
         *
         * @param other The other BFormat to copy from.
         */
        BFormat& operator=(const BFormat& other);

        /**
         * @brief Equality operator.
         *
         * @param other The other BFormat to compare with.
         *
         * @return @c true if the BFormat objects are equal, @c false otherwise.
         */
        bool operator==(const BFormat& other) const;

        /**
         * @brief Inequality operator.
         *
         * @param other The other BFormat to compare with.
         *
         * @return @c true if the BFormat objects are not equal, @c false otherwise.
         */
        bool operator!=(const BFormat& other) const;

        BFormat& operator+=(const BFormat& other);
        BFormat& operator-=(const BFormat& other);
        BFormat& operator*=(const BFormat& other);
        BFormat& operator+=(const AmReal32& value);
        BFormat& operator-=(const AmReal32& value);
        BFormat& operator*=(const AmReal32& value);

    private:
        AudioBuffer* _buffer;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_AMBISONICS_BFORMAT_H
