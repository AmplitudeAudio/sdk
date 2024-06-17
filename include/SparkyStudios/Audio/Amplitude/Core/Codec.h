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

#ifndef SS_AMPLITUDE_AUDIO_CODEC_H
#define SS_AMPLITUDE_AUDIO_CODEC_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/IO/FileSystem.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Audio file reader and writer.
     *
     * The Codec class is used to implement an audio file reader and writer.
     * This is the base class for all audio codecs, each implementation should
     * allow to build decoders and encoders.
     */
    class AM_API_PUBLIC Codec
    {
    public:
        /**
         * @brief Audio file reader.
         *
         * The Decoder is built by a Codec instance. It's used to read
         * an audio file and process its data. Each implementation should
         * allow to load the entire file into memory or stream it from the file system.
         *
         * The Stream() method of a decoder implementation should be thread-safe.
         */
        class Decoder
        {
        public:
            explicit Decoder(const Codec* codec)
                : m_format()
                , m_codec(codec)
            {}

            virtual ~Decoder() = default;

            /**
             * @brief Opens the given file to start decoding.
             *
             * @param file The file to read.
             */
            virtual bool Open(std::shared_ptr<File> file) = 0;

            /**
             * @brief Closes the file previously opened.
             *
             * @return Whether is operation is successful.
             */
            virtual bool Close() = 0;

            /**
             * @brief Gets the audio sample format.
             *
             * @return The audio sample format.
             */
            [[nodiscard]] const SoundFormat& GetFormat() const
            {
                return m_format;
            }

            /**
             * @breif Loads the entire audio file into the output buffer.
             *
             * The output buffer must allocate enough size for this operation
             * to be successful.
             *
             * @param out The buffer to load audio data data into.
             *
             * @return The number of audio frames loaded into the buffer.
             */
            virtual AmUInt64 Load(AmVoidPtr out) = 0;

            /**
             * @brief Streams a part of the file from disk into the output buffer.
             *
             * @param out The buffer to stream the file data into.
             * @param offset The offset in frames from which start to read the file.
             * @param length The length in frames to read from the file.
             *
             * @return The number of frames read.
             */
            virtual AmUInt64 Stream(AmVoidPtr out, AmUInt64 offset, AmUInt64 length) = 0;

            /**
             * @brief Moves the cursor to the given frame.
             * @param offset The offset in frames to move the cursor to.
             *
             * @return Whether the operation is successful.
             */
            virtual bool Seek(AmUInt64 offset) = 0;

        protected:
            /**
             * @brief The audio sample format of the file
             * currently loaded by this decoder.
             *
             * The sound format must be filled during the initialization
             * of this decoder.
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
         * The Encoder is built by a Codec instance. It's used to write
         * data to an audio file.
         *
         * The Write() methods of an encoder implementation should be thread safe.
         */
        class Encoder
        {
        public:
            explicit Encoder(const Codec* codec)
                : m_format()
                , m_codec(codec)
            {}

            virtual ~Encoder() = default;

            /**
             * @brief Opens or create a file at the given path to start encoding.
             *
             * @param file The file to write.
             */
            virtual bool Open(std::shared_ptr<File> file) = 0;

            /**
             * @brief Closes the opened file.
             *
             * @return Whether the operation is successful.
             */
            virtual bool Close() = 0;

            /**
             * @brief Sets the audio sample format.
             *
             * @param format The audio sample format.
             */
            virtual void SetFormat(const SoundFormat& format)
            {
                m_format = format;
            }

            /**
             * @brief Writes a the given buffer into the file.
             *
             * @param in The buffer to write into the the file.
             * @param offset The offset in frames from which write the input buffer.
             * @param length The length in frames to write from the input buffer.
             *
             * @return The number of frames written.
             */
            virtual AmUInt64 Write(AmVoidPtr in, AmUInt64 offset, AmUInt64 length) = 0;

        protected:
            /**
             * @brief The audio sample format of the file
             * to write using this encoder.
             *
             * The sound format must be set before the initialization
             * of this encoder. Otherwise the encoder initialization will
             * mostly fail.
             */
            SoundFormat m_format;

            /**
             * @brief The codec instance which built this decoder.
             */
            const Codec* m_codec;
        };

        /**
         * @brief Create a new Codec instance.
         *
         * @param name The codec name. Recommended names are "FILE_EXTENSION".
         * eg. "WAV" or "OGG".
         */
        explicit Codec(AmString name);

        virtual ~Codec();

        /**
         * @brief Creates a new instance of the decoder associated
         * to this codec.
         *
         * @return A Decoder instance.
         */
        [[nodiscard]] virtual Decoder* CreateDecoder() = 0;

        /**
         * @brief Destroys the decoder associated to this codec.
         *
         * @param decoder The decoder instance to destroy.
         */
        virtual void DestroyDecoder(Decoder* decoder) = 0;

        /**
         * @brief Creates a new instance of the encoder associated
         * to this codec.
         *
         * @return An Encoder instance.
         */
        [[nodiscard]] virtual Encoder* CreateEncoder() = 0;

        /**
         * @brief Destroys the encoder associated to this codec.
         *
         * @param encoder The encoder instance to destroy.
         */
        virtual void DestroyEncoder(Encoder* encoder) = 0;

        /**
         * @brief Checks whether this Codec can handle the file at
         * the given path.
         *
         * @param file The file to check.
         *
         * @return Whether this Codec can handle a file.
         */
        [[nodiscard]] virtual bool CanHandleFile(std::shared_ptr<File> file) const = 0;

        /**
         * @brief Gets the name of this codec.
         *
         * @return The name of this codec.
         */
        [[nodiscard]] const AmString& GetName() const
        {
            return m_name;
        }

        /**
         * @brief Registers a new audio codec.
         *
         * @param codec The audio codec to add in the registry.
         */
        static void Register(Codec* codec);

        /**
         * @brief Unregisters a audio codec.
         *
         * @param codec The audio codec to remove from the registry.
         */
        static void Unregister(const Codec* codec);

        /**
         * @brief Look up a codec by name.
         *
         * @return The codec with the given name, or NULL if none.
         */
        static Codec* Find(const AmString& name);

        /**
         * @brief Finds the codec which can handle the given file.
         *
         * @param file The file to find the codec for.
         * @return The codec which can handle the given file.
         */
        static Codec* FindCodecForFile(std::shared_ptr<File> file);

        /**
         * @brief Locks the codecs registry.
         *
         * This function is mainly used for internal purposes. Its
         * called before the Engine initialization, to discard the
         * registration of new codecs after the engine is fully loaded.
         */
        static void LockRegistry();

        /**
         * @brief Unlocks the codecs registry.
         *
         * This function is mainly used for internal purposes. Its
         * called after the Engine deinitialization, to allow the
         * registration of new divers after the engine is fully unloaded.
         */
        static void UnlockRegistry();

    protected:
        /**
         * @brief The name of this codec.
         */
        AmString m_name;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_CODEC_H
