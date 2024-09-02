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

#ifndef _AM_CORE_AUDIO_BUFFER_H
#define _AM_CORE_AUDIO_BUFFER_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Represents a vire to a single channel in an @c AudioBuffer.
     */
    class AM_API_PUBLIC AudioBufferChannel
    {
    public:
        /**
         * @brief Gets the size of the buffer.
         *
         * @return The size of the buffer.
         */
        [[nodiscard]] AmSize size() const;

        /**
         * @brief Returns a float pointer to the begin of the channel data.
         *
         * @return A float pointer to the begin of the channel data.
         */
        AmReal32* begin();

        /**
         * @brief Returns a const float pointer to the begin of the channel data.
         *
         * @return A const float pointer to the begin of the channel data.
         */
        [[nodiscard]] const AmReal32* begin() const;

        /**
         * @brief Returns a float pointer to the end of the channel data.
         *
         * @return A float pointer to the end of the channel data.
         */
        AmReal32* end();

        /**
         * @brief Returns a const float pointer to the end of the channel data.
         *
         * @return A const float pointer to the end of the channel data.
         */
        [[nodiscard]] const AmReal32* end() const;

        /**
         * @brief Clears the channel data to zero.
         */
        void clear();

        /**
         * @brief Checks if the channel is enabled.
         *
         * @return @c true if the channel is enabled, @c false otherwise.
         */
        [[nodiscard]] bool enabled() const;

        /**
         * @brief Gets the frame at the specified index.
         *
         * @param index The frame index.
         *
         * @return The frame at the specified index.
         */
        AmReal32& operator[](AmSize index);

        /**
         * @brief Gets the frame at the specified index.
         *
         * @param index The frame index.
         *
         * @return The frame at the specified index.
         */
        [[nodiscard]] const AmReal32& operator[](AmSize index) const;

        /**
         * @brief Sets the entire channel data from the provided vector.
         *
         * @param data The vector containing the new channel data.
         *
         * @return A reference to the modified channel.
         */
        AudioBufferChannel& operator=(const std::vector<AmReal32>& data);

        /**
         * @brief Sets the entire channel data from the provided @c AudioBufferChannel.
         *
         * @param channel The @c AudioBufferChannel to copy the data from.
         *
         * @return A reference to the modified channel.
         */
        AudioBufferChannel& operator=(const AudioBufferChannel& channel);

        /**
         * @brief Adds the provided @c AudioBufferChannel to this channel.
         *
         * @param channel The @c AudioBufferChannel to add.
         *
         * @return A reference to the modified channel.
         */
        AudioBufferChannel& operator+=(const AudioBufferChannel& channel);

        /**
         * @brief Subtracts the provided @c AudioBufferChannel from this channel.
         *
         * @param channel The @c AudioBufferChannel to subtract.
         *
         * @return A reference to the modified channel.
         */
        AudioBufferChannel& operator-=(const AudioBufferChannel& channel);

        /**
         * @brief Pointwise multiplies this channel with the provided @c AudioBufferChannel.
         *
         * @param channel The @c AudioBufferChannel to multiply with.
         *
         * @return A reference to the modified channel.
         */
        AudioBufferChannel& operator*=(const AudioBufferChannel& channel);

        /**
         * @brief Pointwise multiplies this channel with the provided scalar.
         *
         * @param scalar The scalar to multiply with.
         *
         * @return A reference to the modified channel.
         */
        AudioBufferChannel& operator*=(AmReal32 scalar);

    private:
        friend class AudioBuffer;

        AudioBufferChannel(AmReal32* begin, AmSize numFrames);

        AmReal32* _begin;
        AmSize _frameCount;
        bool _isEnabled;
    };

    class AM_API_PUBLIC AudioBuffer
    {
    public:
        /**
         * @brief Copies the given number of frames from the source buffer to the destination buffer.
         *
         * @param source The source buffer to copy.
         * @param sourceOffset The offset in the source buffer.
         * @param destination The destination buffer to copy to.
         * @param destinationOffset The offset in the destination buffer.
         * @param numFrames The number of frames to copy.
         */
        static void Copy(
            const AudioBuffer& source, AmSize sourceOffset, AudioBuffer& destination, AmSize destinationOffset, AmSize numFrames);

        /**
         * @brief Creates an empty audio buffer.
         */
        AudioBuffer();

        /**
         * @brief Creates an audio buffer with the specified number of frames and channels.
         *
         * @param numFrames The number of frames in the buffer.
         * @param numChannels The number of channels in the buffer.
         */
        AudioBuffer(AmSize numFrames, AmSize numChannels);

        /**
         * @brief Explicitly deletes the audio buffer copy to avoid unintended usage.
         *
         * @param buffer The other audio buffer to copy.
         *
         * @note Use the assignment operator to copy the audio buffer.
         */
        AudioBuffer(const AudioBuffer& buffer) = delete;

        /**
         * @brief Moves the given audio buffer data in this one.
         *
         * @param buffer The other audio buffer to move.
         */
        AudioBuffer(AudioBuffer&& buffer) noexcept;

        ~AudioBuffer();

        /**
         * @brief Check if the audio buffer is empty.
         *
         * @return @c true if the audio buffer is empty, @c false otherwise.
         */
        [[nodiscard]] bool IsEmpty() const;

        /**
         * @brief Gets the number of frames in the buffer.
         *
         * @return The number of frames in the buffer.
         */
        [[nodiscard]] AmSize GetFrameCount() const;

        /**
         * @brief Gets the number of channels in the buffer.
         *
         * @return The number of channels in the buffer.
         */
        [[nodiscard]] AmSize GetChannelCount() const;

        /**
         * @brief Sets the entire audio buffer data to zero.
         */
        void Clear();

        /**
         * @brief Gets the wrapped audio buffer data.
         *
         * @return The wrapped audio buffer data.
         */
        [[nodiscard]] const AmAlignedReal32Buffer& GetData() const;

        /**
         * @brief Gets the @c AudioBufferChannel at the specified index.
         *
         * @param index The channel index.
         *
         * @return The @c AudioBufferChannel at the specified index.
         */
        AudioBufferChannel& GetChannel(AmSize index);

        /**
         * @brief Gets the @c AudioBufferChannel at the specified index.
         *
         * @param index The channel index.
         *
         * @return The @c AudioBufferChannel at the specified index.
         */
        [[nodiscard]] const AudioBufferChannel& GetChannel(AmSize index) const;

        /**
         * @brief Clones the audio buffer and returns a new instance.
         *
         * @return A new instance with a cloned copy of the audio buffer data.
         */
        AudioBuffer Clone() const;

        /**
         * @brief Gets the @c AudioBufferChannel at the specified index.
         *
         * @param index The channel index.
         *
         * @return The @c AudioBufferChannel at the specified index.
         */
        AudioBufferChannel& operator[](AmSize index);

        /**
         * @brief Gets the @c AudioBufferChannel at the specified index.
         *
         * @param index The channel index.
         *
         * @return The @c AudioBufferChannel at the specified index.
         */
        [[nodiscard]] const AudioBufferChannel& operator[](AmSize index) const;

        /**
         * @brief Copies the audio buffer data from the provided @c AudioBuffer.
         *
         * @param buffer The other audio buffer to copy.
         *
         * @return This instance with the copied audio buffer data.
         */
        AudioBuffer& operator=(const AudioBuffer& buffer);

        /**
         * @brief Accumulates the audio buffer data from the provided @c AudioBuffer.
         *
         * @param buffer The buffer to add in this one.
         *
         * @return This instance with the added audio buffer data.
         */
        AudioBuffer& operator+=(const AudioBuffer& buffer);

        /**
         * @brief Subtracts the audio buffer data from the provided @c AudioBuffer.
         *
         * @param buffer The buffer to subtract from this one.
         *
         * @return This instance with the subtracted audio buffer data.
         */
        AudioBuffer& operator-=(const AudioBuffer& buffer);

        /**
         * @brief Pointwise multiplies the audio buffer data with the provided @c AudioBuffer.
         *
         * @param buffer The buffer to multiply with this one.
         *
         * @return This instance with the multiplied audio buffer data.
         */
        AudioBuffer& operator*=(const AudioBuffer& buffer);

    private:
        void Initialize(AmSize channelCount);

        AmSize _frameCount;
        std::vector<AudioBufferChannel> _channels;

        AmAlignedReal32Buffer _data;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_AUDIO_BUFFER_H
