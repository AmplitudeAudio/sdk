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

#ifndef SPARK_AUDIO_FILE_LOADER_H
#define SPARK_AUDIO_FILE_LOADER_H

#include <string>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    class FileLoader
    {
    public:
        void StartLoading()
        {}

        bool TryFinalize()
        {
            return true;
        }
    };

    class Resource
    {
    public:
        virtual ~Resource() = default;

        void LoadFile(AmString filename, FileLoader* loader);

        void SetFilename(const std::string& filename)
        {
            _filename = filename;
        }

        [[nodiscard]] const std::string& GetFilename() const
        {
            return _filename;
        }

    private:
        virtual void Load(FileLoader* loader) = 0;

        std::string _filename;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_FILE_LOADER_H
