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

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Audio file reader and parser.
     *
     * The Codec class is used to implement an audio file reader and parser.
     * This is the base class for all audio codecs, each implementation should
     * allow to load the entire file into memory or stream it from disk.
     *
     * The Stream() method of a codec implementation should be thread safe.
     */
    class Codec
    {
    public:
        /**
         * @brief Create a new Codec instance.
         *
         * @param name The codec name. Recommended names are "FILE_EXTENSION".
         * eg. "WAV" or "OGG".
         */
        explicit Codec(AmString name);

        virtual ~Codec() = default;

        /**
         * @brief Initialize the codec instance with the given file.
         * @param filePath The path to the file.
         */
        virtual bool Initialize(AmString filePath) = 0;

        /**
         * @breif Load the entire audio file into the output buffer.
         *
         * @param out The buffer to load audio file data into.
         *
         * @return The number of audio frames loaded into the buffer.
         */
        virtual AmUInt64 Load(AmFloat32Buffer out) = 0;

        /**
         * @brief Stream a part of the file from disk into the output buffer.
         *
         * @param out The buffer to stream the file data into.
         * @param offset The offset in frames from which start to read the file.
         * @param length The length in frames to read from the file.
         *
         * @return The number of frames read.
         */
        virtual AmUInt64 Stream(AmFloat32Buffer out, AmUInt64 offset, AmUInt64 length) = 0;

        /**
         * @brief Checks whether this Codec can handle the file at
         * the given path.
         *
         * @param filePath The path to the file to check.
         *
         * @return Whether this Codec can handle a file.
         */
        virtual bool CanHandleFile(AmString filePath) = 0;

        /**
         * @brief Gets the name of this codec.
         *
         * @return The name of this codec.
         */
        [[nodiscard]] AmString GetName() const
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
         * @brief Look up a codec by name.
         *
         * @return The codec with the given name, or NULL if none.
         */
        static Codec* Find(AmString name);

        /**
         * @brief Finds the codec which can handle the given file.
         *
         * @param filePath The path to the file.
         * @return The codec which can handle the given file.
         */
        static Codec* FindCodecForFile(AmString filePath);

        /**
         * @brief Locks the codecs registry.
         *
         * This function is mainly used for internal purposes. Its
         * called before the Engine initialization, to discard the
         * registration of new codecs after the engine is fully loaded.
         */
        static void LockRegistry();

    protected:
        AmString m_name;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_CODEC_H
