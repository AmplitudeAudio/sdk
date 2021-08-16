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

#ifndef SS_AMPLITUDE_AUDIO_SOUND_H
#define SS_AMPLITUDE_AUDIO_SOUND_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Core/Codec.h>
#include <SparkyStudios/Audio/Amplitude/IO/FileLoader.h>

namespace SparkyStudios::Audio::Amplitude
{
    class SoundCollection;

    class Sound : public Resource
    {
    public:
        Sound();
        ~Sound() override;

        /**
         * @brief Initializes the Sound given the SoundCollection that it is a part of.
         *
         * The sound collection may contain useful metadata about the sound, like whether
         * or not the sound should be streamed, which may impact how you load it.
         *
         * @param collection The collection that the sound is part of.
         */
        void Initialize(const SoundCollection* collection);

        /**
         * @brief Loads the audio file.
         */
        void Load(FileLoader* loader) override;

        /**
         * @brief Sets the format of this Sound.
         *
         * @param format The sound format.
         */
        void SetFormat(const SoundFormat& format)
        {
            m_format = format;
        }

        /**
         * @brief Gets the format of this Sound.
         * @return The format of this Sound.
         */
        [[nodiscard]] const SoundFormat& GetFormat() const
        {
            return m_format;
        }

        /**
         * @brief Returns the user data associated to this Sound.
         * @return The user data.
         */
        [[nodiscard]] AmVoidPtr GetUserData() const
        {
            return _userData;
        }

        /**
         * @brief Sets the user data associated to this Sound.
         * @param userData The user data.
         */
        void SetUserData(AmVoidPtr userData)
        {
            _userData = userData;
        }

        /**
         * @brief Returns the SoundCollection storing this Sound.
         * @return The Sound collection.
         */
        [[nodiscard]] const SoundCollection* GetSoundCollection() const
        {
            return m_collection;
        }

        /**
         * @brief Renders audio data.
         *
         * The audio data is read from the audio file loaded by this
         * Sound. This function is mostly for internal uses.
         *
         * @param offset The offset in the audio data to start reading from.
         * @param frames The number of audio frames to read.
         *
         * @return The number of frames read.
         */
        AmUInt64 GetAudio(AmUInt64 offset, AmUInt64 frames);

        /**
         * @brief Destroys the audio sample loaded by this Sound and
         * releases all resources.
         */
        void Destroy();

    protected:
        const SoundCollection* m_collection;
        SoundFormat m_format;

    private:
        AmVoidPtr _userData;

        AmAlignedFloat32Buffer* _streamBuffer;
        Codec::Decoder* _decoder;

        bool _stream;
        bool _loop;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_SOUND_H
