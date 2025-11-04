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

#ifndef _AM_CORE_CODEC_H
#define _AM_CORE_CODEC_H

#include <SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h>
#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/IO/FileSystem.h>

#include <map>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Audio file reader and writer.
     *
     * The @c Codec class is used to implement an audio file reader and writer.
     * This is the base class for all audio codecs, each implementation should
     * allow building decoders and encoders.
     *
     * The @c Codec class follows the [plugin architecture](/plugins/anatomy), and thus, you are able to create
     * your own codecs and register them to the Engine by inheriting from this class and by implementing the necessary dependencies.
     *
     * @ingroup engine
     */
    class AM_API_PUBLIC Codec
    {
    public:
        /**
         * @brief Audio file reader.
         *
         * The @c Decoder is built by a @c Codec instance. It's used to read an audio file and process its data. Each implementation
         * should allow to @ref Load "load" the entire file into memory or @ref Stream "stream" it from the file system.
         *
         * The @ref Stream "`Stream()`" method of a decoder implementation must be thread-safe.
         */
        class AM_API_PUBLIC Decoder
        {
        public:
            /**
             * @brief Creates a new instance of the decoder for the given codec.
             *
             * @param[in] codec The parent codec for the decoder.
             */
            explicit Decoder(const Codec* codec);

            /**
             * @brief Default destructor.
             */
            virtual ~Decoder() = default;

            /**
             * @brief Opens the given file to start decoding.
             *
             * @param[in] file The file to read.
             *
             * @return @c true if the file was opened successfully, @c false otherwise.
             */
            virtual bool Open(std::shared_ptr<File> file) = 0;

            /**
             * @brief Closes the previously opened file.
             *
             * @return @c true if the file was closed successfully, @c false otherwise.
             */
            virtual bool Close() = 0;

            /**
             * @brief Gets the audio sample format.
             *
             * @return The audio sample format.
             *
             * @see SoundFormat
             */
            [[nodiscard]] const SoundFormat& GetFormat() const;

            /**
             * @brief Loads the entire audio file into the output buffer.
             *
             * @warning The output buffer must allocate enough sizes for this operation to be successful.
             *
             * @param[out] out The buffer to load audio data data into.
             *
             * @return The number of audio frames loaded into the buffer.
             */
            virtual AmUInt64 Load(AudioBuffer* out) = 0;

            /**
             * @brief Streams a part of the file from the disk into the output buffer.
             *
             * @param[out] out The buffer to stream the file data into.
             * @param[in] bufferOffset The offset in frames from which start to write in the @c out buffer.
             * @param[in] seekOffset The offset in frames from which start to read the file.
             * @param[in] length The length in frames to read from the file.
             *
             * @return The number of frames read.
             */
            virtual AmUInt64 Stream(AudioBuffer* out, AmUInt64 bufferOffset, AmUInt64 seekOffset, AmUInt64 length) = 0;

            /**
             * @brief Moves the cursor to the given frame.
             *
             * @param[in] offset The offset in frames to move the cursor to.
             *
             * @return @c true if the cursor was moved successfully, @c false otherwise.
             */
            virtual bool Seek(AmUInt64 offset) = 0;

        protected:
            /**
             * @brief The audio sample format of the file currently loaded by this decoder.
             *
             * @note The sound format must be filled during the initialization of this decoder.
             */
            SoundFormat m_format;

            /**
             * @brief The codec instance which built this decoder.
             */
            const Codec* m_codec;
        };

        /**
         * @brief Audio file writer.
         *
         * The @c Encoder is built by a @c Codec instance. It's used to write
         * data to an audio file.
         *
         * The @ref Write "`Write()`" method of an encoder implementation must be thread safe.
         */
        class AM_API_PUBLIC Encoder
        {
        public:
            /**
             * @brief Creates a new instance of the encoder for the given codec.
             *
             * @param[in] codec The parent codec for the encoder.
             */
            explicit Encoder(const Codec* codec);

            /**
             * @brief Default destructor.
             */
            virtual ~Encoder() = default;

            /**
             * @brief Opens or create a file at the given path to start encoding.
             *
             * @param[in] file The file to write.
             *
             * @return @c true if the file was opened successfully, @c false otherwise.
             */
            virtual bool Open(std::shared_ptr<File> file) = 0;

            /**
             * @brief Closes a previously opened file.
             *
             * @return @c true if the file was closed successfully, @c false otherwise.
             */
            virtual bool Close() = 0;

            /**
             * @brief Sets the audio sample format.
             *
             * @param[in] format The audio sample format.
             */
            virtual void SetFormat(const SoundFormat& format);

            /**
             * @brief Writes the given buffer into the file.
             *
             * @param[in] in The buffer to write into the file.
             * @param[in] offset The offset in frames from which write the input buffer.
             * @param[in] length The length in frames to write from the input buffer.
             *
             * @return The number of frames written.
             */
            virtual AmUInt64 Write(AudioBuffer* in, AmUInt64 offset, AmUInt64 length) = 0;

        protected:
            /**
             * @brief The audio sample format of the file to write using this encoder.
             *
             * The sound format must be set before the initialization of this encoder. Otherwise,
             * the encoder initialization should fail.
             *
             * @note The final behavior depends on the specific codec implementation.
             */
            SoundFormat m_format;

            /**
             * @brief The codec instance which built this decoder.
             */
            const Codec* m_codec;
        };

        /**
         * @brief Create a new codec instance.
         *
         * @param name The codec name. Recommended names are "FILE_EXTENSION", e.g., "WAV" or "OGG".
         */
        explicit Codec(AmString name);

        /**
         * @brief Default destructor.
         */
        virtual ~Codec();

        /**
         * @brief Creates a new instance of the decoder associated with this codec.
         *
         * @return A @c Decoder instance.
         */
        [[nodiscard]] virtual std::shared_ptr<Decoder> CreateDecoder() = 0;

        /**
         * @brief Creates a new instance of the encoder associated with this codec.
         *
         * @return An @c Encoder instance.
         */
        [[nodiscard]] virtual std::shared_ptr<Encoder> CreateEncoder() = 0;

        /**
         * @brief Checks whether this codec can handle the file at the given path.
         *
         * @param[in] file The file to check.
         *
         * @return @c true if the codec can handle the file, @c false otherwise.
         */
        [[nodiscard]] virtual bool CanHandleFile(std::shared_ptr<File> file) const = 0;

        /**
         * @brief Gets the name of this codec.
         *
         * @return The name of this codec.
         */
        [[nodiscard]] const AmString& GetName() const;

        /**
         * @brief Registers a new audio codec.
         *
         * @note This method does nothing if the registry is locked.
         *
         * @param[in] codec The audio codec to add in the registry.
         *
         * @see LockRegistry, UnlockRegistry
         */
        static void Register(std::shared_ptr<Codec> codec);

        /**
         * @brief Unregisters a audio codec.
         *
         * @note This method does nothing if the registry is locked.
         *
         * @param[in] codec The audio codec to remove from the registry.
         *
         * @see LockRegistry, UnlockRegistry
         */
        static void Unregister(std::shared_ptr<const Codec> codec);

        /**
         * @brief Look up a codec by name.
         *
         * @param[in] name The name of the codec to find.
         *
         * @return The codec with the given name, or @c nullptr if not found.
         */
        static std::shared_ptr<Codec> Find(const AmString& name);

        /**
         * @brief Finds the codec which can handle the given file.
         *
         * @param[in] file The file to find the codec for.
         *
         * @return The codec which can handle the given file, or @c nullptr if not found.
         */
        static std::shared_ptr<Codec> FindForFile(std::shared_ptr<File> file);

        /**
         * @brief Locks the codecs' registry.
         *
         * @warning This function is mainly used for internal purposes. It's
         * called before the @c Engine initialization, to discard the registration
         * of new codecs after the engine is fully loaded.
         */
        static void LockRegistry();

        /**
         * @brief Unlocks the codecs' registry.
         *
         * @warning This function is mainly used for internal purposes. It's
         * called after the @c Engine deinitialization, to allow the registration
         * of new codecs after the engine is fully unloaded.
         */
        static void UnlockRegistry();

        /**
         * @brief Gets the list of registered codecs.
         *
         * @return The registry of codecs.
         */
        static const std::map<AmString, std::shared_ptr<Codec>>& GetRegistry();

    protected:
        /**
         * @brief The name of this codec.
         */
        AmString m_name;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_CODEC_H
