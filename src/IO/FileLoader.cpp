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

#include <SparkyStudios/Audio/Amplitude/IO/FileLoader.h>

namespace SparkyStudios::Audio::Amplitude
{
    FileLoader::FileLoader()
        : _basePath(std::filesystem::current_path())
    {}

    void FileLoader::SetBasePath(const std::filesystem::path& basePath)
    {
        if (basePath.is_relative())
            _basePath = (std::filesystem::current_path() / basePath).make_preferred();
        else
            _basePath = basePath;
    }

    std::filesystem::path FileLoader::ResolvePath(const std::filesystem::path& path) const
    {
        if (path.is_relative())
            return (_basePath / path).make_preferred();
        else
            return path;
    }

    void FileLoader::StartLoading()
    {}

    bool FileLoader::TryFinalize()
    {
        return true;
    }

    void Resource::LoadFile(const std::filesystem::path& filename, const FileLoader* loader)
    {
        SetFilename(loader->ResolvePath(filename));
        Load(loader);
    }

    void Resource::SetFilename(const std::filesystem::path& filename)
    {
        _filename = filename;
    }

    const std::filesystem::path& Resource::GetFilename() const
    {
        return _filename;
    }
} // namespace SparkyStudios::Audio::Amplitude
