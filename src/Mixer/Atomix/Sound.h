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

#ifndef SPARK_AUDIO_SOUND_H
#define SPARK_AUDIO_SOUND_H

#include <SparkyStudios/Audio/Amplitude/IO/FileLoader.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>
#include <SparkyStudios/Audio/Amplitude/Sound/SoundCollection.h>

#include <Core/Drivers/MiniAudio/miniaudio.h>

#include "atomix.h"

namespace SparkyStudios::Audio::Amplitude
{
    class Sound : public Resource
    {
    public:
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

        AmInt32 GetAudio(AmUInt32 offset, AmUInt32 frames);
        void Destroy();

    protected:
        const SoundCollection* m_collection;
        SoundFormat m_format;

    private:
        AmVoidPtr _userData;

        AmFloat32Buffer _streamBuffer;
        ma_decoder _streamDecoder;

        bool _stream;
        bool _loop;

        AmUInt32 _size;
        AmUInt8 _channels;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_SOUND_H
